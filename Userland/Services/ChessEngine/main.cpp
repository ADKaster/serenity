/*
 * Copyright (c) 2020, the SerenityOS developers.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "ChessEngine.h"
#include <LibCore/File.h>
#include <LibCore/EventLoop.h>
#include <LibCore/System.h>
#include <LibMain/Main.h>

ErrorOr<int> serenity_main(Main::Arguments)
{
    TRY(Core::System::pledge("stdio recvfd sendfd unix"));
    Core::EventLoop loop;
    TRY(Core::System::unveil(nullptr, nullptr));

    auto buffered_stdin = TRY(Core::BufferedFile::create(TRY(Core::File::standard_input())));
    auto buffered_stdout = TRY(Core::BufferedFile::create(TRY(Core::File::standard_output())));

    auto engine = TRY(ChessEngine::try_create(move(buffered_stdin), move(buffered_stdout)));
    return loop.exec();
}
