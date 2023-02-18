/*
 * Copyright (c) 2023, Andrew Kaster <akaster@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Array.h>
#include <AK/Error.h>
#include <AK/Format.h>
#include <AK/NonnullRefPtr.h>
#include <AK/OwnPtr.h>
#include <AK/StringView.h>
#include <LibLogging/Forward.h>
#include <LibLogging/Severity.h>

namespace Logging {

enum class Destination : u8 {
    None = 0,
    StandardError = 1,
    DebugPort = 2,
    IPCSocket = 4,
    SyslogSocket = 8
};

class Logger : public RefCounted<Logger> {
public:
    static ErrorOr<NonnullRefPtr<Logger>> create_with_configuration(StringView config_domain);
    ~Logger();

    // TODO: Include optional parameters:
    //   Message ID: Which component is logging this message
    //   Structured Data: JSON or other key-value items the caller wants to show in the output

    template<typename... Parameters>
    void log(Severity severity, CheckedFormatString<Parameters...>&& fmtstr, Parameters const&... parameters)
    {
        AK::VariadicFormatParams<AK::AllowDebugOnlyFormatters::No, Parameters...> variadic_format_params { parameters... };
        vlog(severity, fmtstr.view(), variadic_format_params);
    }

#define LOG_AT_SEVERITY(name, severity)                                                                                     \
    template<typename... Parameters>                                                                                        \
    void name(CheckedFormatString<Parameters...>&& fmtstr, Parameters const&... parameters)                                 \
    {                                                                                                                       \
        AK::VariadicFormatParams<AK::AllowDebugOnlyFormatters::No, Parameters...> variadic_format_params { parameters... }; \
        vlog(severity, fmtstr.view(), variadic_format_params);                                                              \
    }

    LOG_AT_SEVERITY(emergency, Severity::Emergency)
    LOG_AT_SEVERITY(alert, Severity::Alert)
    LOG_AT_SEVERITY(critical, Severity::Critical)
    LOG_AT_SEVERITY(error, Severity::Error)
    LOG_AT_SEVERITY(warning, Severity::Warning)
    LOG_AT_SEVERITY(notice, Severity::Notice)
    LOG_AT_SEVERITY(info, Severity::Info)
    LOG_AT_SEVERITY(debug, Severity::Debug)
    LOG_AT_SEVERITY(verbose, Severity::Verbose)

#undef LOG_AT_SEVERITY

    void vlog(Severity, StringView fmtstr, AK::TypeErasedFormatParams&);

    void set_severity(Destination, Severity);
    Severity get_severity(Destination);

    bool will_log(Severity severity) { return m_max_severity >= severity; }

    Logger(Array<Severity, 4> severities, OwnPtr<LogMonitorClient>, OwnPtr<SyslogClient>);

private:
    Destination m_destination = Destination::None;
    Array<Severity, 4> m_severities;
    Severity m_max_severity = Severity::None;

    OwnPtr<LogMonitorClient> m_log_monitor_client;
    OwnPtr<SyslogClient> m_syslog_client;
};

AK_ENUM_BITWISE_OPERATORS(Destination);

}
