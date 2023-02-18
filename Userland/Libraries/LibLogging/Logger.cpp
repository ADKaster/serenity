/*
 * Copyright (c) 2023, Andrew Kaster <akaster@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/BuiltinWrappers.h>
#include <AK/NonnullRefPtr.h>
#include <AK/Optional.h>
#include <AK/StdLibExtras.h>
#include <AK/String.h>
#include <LibConfig/Client.h>
#include <LibCore/System.h>
#include <LibLogging/LogMonitorClient.h>
#include <LibLogging/Logger.h>
#include <LibLogging/Severity.h>
#include <LibLogging/SyslogClient.h>
#include <unistd.h>

namespace Logging {

static Optional<Severity> string_to_severity(StringView str)
{
    if (str.is_empty())
        return Severity::None;
    if (str.equals_ignoring_case("debug"sv))
        return Severity::Debug;
    if (str.equals_ignoring_case("info"sv))
        return Severity::Info;
    if (str.equals_ignoring_case("notice"sv))
        return Severity::Notice;
    if (str.equals_ignoring_case("warning"sv))
        return Severity::Warning;
    if (str.equals_ignoring_case("error"sv))
        return Severity::Error;
    if (str.equals_ignoring_case("critical"sv))
        return Severity::Critical;
    if (str.equals_ignoring_case("alert"sv))
        return Severity::Alert;
    if (str.equals_ignoring_case("emergency"sv))
        return Severity::Emergency;
    if (str.equals_ignoring_case("verbose"sv))
        return Severity::Verbose;
    return {};
}

static Severity severity_or_default(Destination destination, Optional<Severity> severity)
{
    if (severity.has_value() && severity.value() != Severity::Default)
        return severity.value();

    switch (destination) {
    case Destination::DebugPort:
        return Severity::Info;
    case Destination::IPCSocket:
        return Severity::Notice;
    case Destination::StandardError:
    case Destination::SyslogSocket:
    case Destination::None:
        return Severity::None;
    }
}

static Array<Severity, 4> read_severities_from_config(StringView config_domain)
{
    auto stderr_severity_str = Config::read_string(config_domain, "Logger"sv, "stderr_severity"sv);
    auto debug_port_severity_str = Config::read_string(config_domain, "Logger"sv, "debug_port_severity"sv);
    auto log_monitor_severity_str = Config::read_string(config_domain, "Logger"sv, "log_monitor_severity"sv);
    auto syslog_severity_str = Config::read_string(config_domain, "Logger"sv, "syslog_severity"sv);

    Optional<Severity> sev = string_to_severity(stderr_severity_str);
    if (!sev.has_value())
        warnln("Invalid severity for stderr {}, using default", stderr_severity_str);
    auto stderr_severity = severity_or_default(Destination::StandardError, sev);

    sev = string_to_severity(debug_port_severity_str);
    if (!sev.has_value())
        warnln("Invalid severity for debug port {}, using default", debug_port_severity_str);
    auto debug_port_severity = severity_or_default(Destination::DebugPort, sev);

    sev = string_to_severity(log_monitor_severity_str);
    if (!sev.has_value())
        warnln("Invalid severity for log monitor {}, using default", log_monitor_severity_str);
    auto log_monitor_severity = severity_or_default(Destination::IPCSocket, sev);

    sev = string_to_severity(syslog_severity_str);
    if (!sev.has_value())
        warnln("Invalid severity for syslog {}, using default", syslog_severity_str);
    auto syslog_severity = severity_or_default(Destination::SyslogSocket, sev);

    return { stderr_severity, debug_port_severity, log_monitor_severity, syslog_severity };
}

ErrorOr<NonnullOwnPtr<SyslogClient>> create_syslog_client_from_config(StringView config_domain)
{
    auto syslog_address = Config::read_string(config_domain, "Logger"sv, "syslog_address"sv, "localhost"sv);
    auto syslog_port = Config::read_i32(config_domain, "Logger"sv, "syslog_port"sv, 514);
    auto syslog_protocol = Config::read_string(config_domain, "Logger"sv, "syslog_protocol"sv, "udp"sv);

    if (syslog_port > NumericLimits<u16>::max() || syslog_port < 0)
        return Error::from_string_literal("Invalid port in syslog_port configuration");

    auto socket = TRY([&]() -> ErrorOr<NonnullOwnPtr<Core::Socket>> {
        if (syslog_protocol.equals_ignoring_case("udp"sv))
            return Core::UDPSocket::connect(syslog_address, static_cast<u16>(syslog_port));
        if (syslog_protocol.equals_ignoring_case("tcp"sv))
            return Core::TCPSocket::connect(syslog_address, static_cast<u16>(syslog_port));
        return Error::from_string_literal("Invalid protocol in syslog_protocol configuration");
    }());

    // FIXME: Move this to Core::System, it's similar to what Core::Process does
#if defined(AK_OS_BSD_GENERIC)
    auto* progname = getprogname();
    String application_name = TRY(String::
                                      : from_utf8({ progname, strlen(progname) }));
#elif defined(AK_OS_SERENITY)
    char progname[1024];
    if (0 != get_process_name(progname, sizeof(progname) - 1))
        strncpy(progname, "Unknown", 9);
    progname[sizeof(progname) - 1] = '\0';
    String application_name = TRY(String::from_utf8({ progname, strlen(progname) }));
#else
    String application_name = String::from_utf8_short_string("???"sv);
#endif

    auto hostname = TRY(String::from_deprecated_string(TRY(Core::System::gethostname())));

    return try_make<SyslogClient>(move(socket), move(application_name), move(hostname));
}

static constexpr int destination_to_index(Destination destination)
{
    return count_trailing_zeroes_safe(to_underlying(destination));
}

Logger::~Logger() = default;

ErrorOr<NonnullRefPtr<Logger>> Logger::create_with_configuration(StringView config_domain)
{
    auto severities = read_severities_from_config(config_domain);

    OwnPtr<SyslogClient> syslog_client;
    if (severities[destination_to_index(Destination::SyslogSocket)] != Severity::None)
        syslog_client = TRY(create_syslog_client_from_config(config_domain));

    return AK::try_make_ref_counted<Logger>(severities, nullptr, move(syslog_client));
}

Logger::Logger(Array<Severity, 4> severities, OwnPtr<LogMonitorClient> log_monitor_client, OwnPtr<SyslogClient> syslog_client)
    : m_severities(severities)
    , m_log_monitor_client(move(log_monitor_client))
    , m_syslog_client(move(syslog_client))
{
    if (!m_log_monitor_client)
        m_severities[destination_to_index(Destination::IPCSocket)] = Severity::None;
    if (!m_syslog_client)
        m_severities[destination_to_index(Destination::SyslogSocket)] = Severity::None;

    for (auto i = 0u; i < m_severities.size(); ++i) {
        auto destination = Destination(1 << i);
        VERIFY(m_severities[i] != Severity::Default);
        m_max_severity = max(m_max_severity, m_severities[i]);
        if (m_severities[i] != Severity::None)
            m_destination |= destination;
    }
}

void Logger::set_severity(Destination destination, Severity severity)
{
    if (destination == Destination::None)
        return;

    if (has_flag(m_destination, destination)) {
        severity = severity_or_default(destination, severity);
        m_severities[destination_to_index(destination)] = severity;
        m_max_severity = max(m_max_severity, severity);
    }
}

Severity Logger::get_severity(Destination destination)
{
    if (destination == Destination::None)
        return Severity::None;

    if (has_flag(m_destination, destination)) {
        return m_severities[destination_to_index(destination)];
    }

    return Severity::None;
}

static bool should_log(Severity filter_level, Severity log_level)
{
    if (filter_level == Severity::None || log_level == Severity::None)
        return false;
    return filter_level >= log_level;
}

void Logger::vlog(Severity severity, StringView fmtstr, AK::TypeErasedFormatParams& params)
{
    if (!will_log(severity))
        return;

    bool const should_log_stderr = has_flag(m_destination, Destination::StandardError) && should_log(m_severities[destination_to_index(Destination::StandardError)], severity);
    bool const should_log_debug_port = has_flag(m_destination, Destination::DebugPort) && should_log(m_severities[destination_to_index(Destination::DebugPort)], severity);
    bool const should_log_ipc = has_flag(m_destination, Destination::IPCSocket) && should_log(m_severities[destination_to_index(Destination::IPCSocket)], severity);
    bool const should_log_syslog = has_flag(m_destination, Destination::SyslogSocket) && should_log(m_severities[destination_to_index(Destination::SyslogSocket)], severity);

    // FIXME: Just use one builder for all of these instead of deferring to AK::Format to build
    if (should_log_stderr)
        vout(stderr, fmtstr, params, true);
    if (should_log_debug_port)
        vdbgln(fmtstr, params);

    if (should_log_ipc || should_log_syslog) {
        auto now = Time::now_realtime();

        StringBuilder builder;
        if (auto err = vformat(builder, fmtstr, params); err.is_error())
            return;
        if (should_log_syslog)
            m_syslog_client->log(severity, builder.string_view(), now);
        // TODO: Destination::IPCSocket
    }
}

}
