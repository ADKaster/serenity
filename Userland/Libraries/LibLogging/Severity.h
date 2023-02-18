/*
 * Copyright (c) 2023, Andrew Kaster <akaster@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Types.h>
#include <syslog.h>

namespace Logging {

enum class Severity : i32 {
    None = -1,
    Emergency = LOG_EMERG,
    Alert = LOG_ALERT,
    Critical = LOG_CRIT,
    Error = LOG_ERR,
    Warning = LOG_WARNING,
    Notice = LOG_NOTICE,
    Info = LOG_INFO,
    Debug = LOG_DEBUG,
    Verbose = LOG_DEBUG + 1, // Will never be logged to IPC or Syslog
    Default = 100,           // Use system default
};

}
