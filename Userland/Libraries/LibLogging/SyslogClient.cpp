/*
 * Copyright (c) 2023, Andrew Kaster <akaster@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/StringBuilder.h>
#include <LibCore/DateTime.h>
#include <LibLogging/Severity.h>
#include <LibLogging/SyslogClient.h>
#include <LibTimeZone/TimeZone.h>
#include <syslog.h>
#include <unistd.h>

namespace Logging {

SyslogClient::SyslogClient(NonnullOwnPtr<Core::Socket> socket, String host_name, String application_name)
    : m_host_name(move(host_name))
    , m_application_name(move(application_name))
    , m_pid(getpid())
    , m_socket(move(socket))
{
}

static Optional<int> severity_to_syslog_severity(Severity severity)
{
    switch (severity) {
    case Severity::Debug:
        return LOG_DEBUG;
    case Severity::Info:
        return LOG_INFO;
    case Severity::Notice:
        return LOG_NOTICE;
    case Severity::Warning:
        return LOG_WARNING;
    case Severity::Error:
        return LOG_ERR;
    case Severity::Critical:
        return LOG_CRIT;
    case Severity::Alert:
        return LOG_ALERT;
    case Severity::Emergency:
        return LOG_EMERG;
    case Severity::Default:
    case Severity::None:
    case Severity::Verbose:
        return {};
    }
}

void SyslogClient::log(Severity severity, StringView message, Time current_time)
{
    if (auto err = try_log(severity, message, current_time); err.is_error())
        dbgln("Unable to log syslog message!");
}

// RFC 5424 6. Syslog Message Format, https://www.rfc-editor.org/rfc/rfc5424#section-6
ErrorOr<void> SyslogClient::try_log(Severity severity, StringView message, Time current_time)
{
    auto syslog_severity = severity_to_syslog_severity(severity);
    if (!syslog_severity.has_value())
        return {};

    StringBuilder builder;

    TRY(builder.try_appendff("<{}>1", m_facility_code * 8 + syslog_severity.value()));
    TRY(builder.try_appendff(" {} {} {} {}", m_host_name, m_application_name, m_pid, m_message_id));

    struct tm utc_time { };
    time_t const t = static_cast<time_t>(current_time.to_seconds());
    (void)gmtime_r(&t, &utc_time);
    TRY(builder.try_appendff(" {:04}-{:02}-{:02}T{:02}:{:02}:{:02}.{:06}Z",
        utc_time.tm_year + 1900,
        utc_time.tm_mon + 1,
        utc_time.tm_mday,
        utc_time.tm_hour,
        utc_time.tm_min,
        utc_time.tm_sec,
        current_time.to_microseconds()));

    // NOTE: Structured data goes here

    TRY(builder.try_appendff(" {}", message));

    TRY(m_socket->write_entire_buffer(builder.to_byte_buffer()));

    return {};
}

}
