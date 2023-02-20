/*
 * Copyright (c) 2020, the SerenityOS developers.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Engine.h"
#include <LibCore/File.h>
#include <fcntl.h>
#include <spawn.h>
#include <stdio.h>
#include <unistd.h>

Engine::~Engine()
{
    if (m_pid != -1)
        kill(m_pid, SIGINT);
}

Engine::Engine(StringView command)
{
    int wpipefds[2];
    int rpipefds[2];
    if (pipe2(wpipefds, O_CLOEXEC) < 0) {
        perror("pipe2");
        VERIFY_NOT_REACHED();
    }

    if (pipe2(rpipefds, O_CLOEXEC) < 0) {
        perror("pipe2");
        VERIFY_NOT_REACHED();
    }

    posix_spawn_file_actions_t file_actions;
    posix_spawn_file_actions_init(&file_actions);
    posix_spawn_file_actions_adddup2(&file_actions, wpipefds[0], STDIN_FILENO);
    posix_spawn_file_actions_adddup2(&file_actions, rpipefds[1], STDOUT_FILENO);

    DeprecatedString cstr(command);
    char const* argv[] = { cstr.characters(), nullptr };
    if (posix_spawnp(&m_pid, cstr.characters(), &file_actions, nullptr, const_cast<char**>(argv), environ) < 0) {
        perror("posix_spawnp");
        VERIFY_NOT_REACHED();
    }

    posix_spawn_file_actions_destroy(&file_actions);

    close(wpipefds[0]);
    close(rpipefds[1]);

    auto infile = Core::File::adopt_fd(rpipefds[0], Core::File::OpenMode::Read, Core::File::ShouldCloseFileDescriptor::Yes).release_value_but_fixme_should_propagate_errors();
    auto buffered_infile = Core::BufferedFile::create(move(infile)).release_value_but_fixme_should_propagate_errors();
    set_in(move(buffered_infile));

    auto outfile = Core::File::adopt_fd(wpipefds[1], Core::File::OpenMode::Write, Core::File::ShouldCloseFileDescriptor::Yes).release_value_but_fixme_should_propagate_errors();
    auto buffered_outfile = Core::BufferedFile::create(move(outfile)).release_value_but_fixme_should_propagate_errors();
    set_out(move(buffered_outfile));

    send_command(Chess::UCI::UCICommand());
}

void Engine::handle_bestmove(Chess::UCI::BestMoveCommand const& command)
{
    if (m_bestmove_callback)
        m_bestmove_callback(command.move());

    m_bestmove_callback = nullptr;
}
