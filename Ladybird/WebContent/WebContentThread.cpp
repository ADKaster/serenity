/*
 * Copyright (c) 2022, Andrew Kaster <akaster@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "WebContentThread.h"
#include <AK/Error.h>
#include <AK/Format.h>
#include <Ladybird/EventLoopImplementationQt.h>
#include <LibCore/Timer.h>
#include <QDebug>

extern ErrorOr<void> start_web_content(int);

WebContentThread::WebContentThread(int fd_passing_socket)
    : m_fd_passing_socket(fd_passing_socket)
{
}

void WebContentThread::run()
{
    Core::EventLoop m_event_loop;
    qDebug() << "Running web content thread";
    auto err = start_web_content(m_fd_passing_socket);
    if (err.is_error()) {
        warnln("WebContent failed with error: {}", err.error());
        exit(err.error().code());
    } else {
        m_event_loop.exec();
    }
}
