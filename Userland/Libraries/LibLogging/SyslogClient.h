/*
 * Copyright (c) 2023, Andrew Kaster <akaster@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Badge.h>
#include <AK/Error.h>
#include <AK/NonnullOwnPtr.h>
#include <AK/String.h>
#include <AK/Time.h>
#include <LibCore/Socket.h>
#include <LibLogging/Forward.h>
#include <LibLogging/Severity.h>

namespace Logging {

class SyslogClient {
public:
    explicit SyslogClient(NonnullOwnPtr<Core::Socket>, String host_name, String application_name);

    void log(Severity, StringView message, Time current_time);

private:
    ErrorOr<void> try_log(Severity severity, StringView message, Time current_time);

    // FIXME: Pull from config
    int m_facility_code = LOG_USER;

    // FIXME: Pass through from Logger
    StringView m_message_id = ""sv;

    String m_host_name;
    String m_application_name;

    pid_t m_pid;

    NonnullOwnPtr<Core::Socket> m_socket;
};

}
