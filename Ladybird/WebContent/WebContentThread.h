/*
 * Copyright (c) 2022, Andrew Kaster <akaster@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibCore/EventLoop.h>
#include <QThread>

class WebContentThread : public QThread {
    Q_OBJECT
public:
    WebContentThread(int fd_passing_socket);
    virtual ~WebContentThread() override = default;

private:
    virtual void run() override;

    int m_fd_passing_socket { -1 };
};
