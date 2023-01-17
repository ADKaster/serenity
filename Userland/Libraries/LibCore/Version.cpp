/*
 * Copyright (c) 2021, Mahmoud Mandour <ma.mandourr@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/String.h>
#include <LibCore/System.h>
#include <LibCore/Version.h>

namespace Core::Version {

ErrorOr<String> read_long_version_string()
{
#if defined(AK_OS_WINDOWS)
    auto version = "1.0.0"sv;
    auto git_hash = "a"sv;
#else
    auto result = Core::System::uname();
    if (result.is_error())
        return {};

    auto const* version = result.value().release;
    auto const *git_hash = result.value().version;
#endif

    return String::formatted("Version {} revision {}", version, git_hash);
}

}
