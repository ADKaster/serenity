/*
 * Copyright (c) 2023, Liav A. <liavalb@hotmail.co.il>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifdef EXPORT_MODULE
module;
#include <stddef.h>
#include <AK/PublicMacros.h>
module AK;
#else
#include <AK/Error.h>

#ifdef KERNEL
#    include <AK/Format.h>
#endif
#endif

namespace AK {

Error Error::from_string_view_or_print_error_and_return_errno(StringView string_literal, [[maybe_unused]] int code)
{
#ifdef KERNEL
    dmesgln("{}", string_literal);
    return Error::from_errno(code);
#else
    return Error::from_string_view(string_literal);
#endif
}

}
