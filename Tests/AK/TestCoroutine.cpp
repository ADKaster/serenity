/*
 * Copyright (c) 2021, Andrew Kaster <akaster@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibTest/TestCase.h>

#include <AK/Coroutine.h>

static Task<int> get_int()
{
    co_return 1;
}

static Task<int> get_sum()
{
    int x = co_await get_int();
    int y = co_await get_int();
    co_return x + y;
}

TEST_CASE(simple_task)
{
    Task<int> t = get_int();
    EXPECT(t.done());
    EXPECT(t.promise().has_value());
    EXPECT_EQ(t.promise().value(), 1);

    EXPECT_EQ(get_sum().promise().value(), 2);
}
