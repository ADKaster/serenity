/*
 * Copyright (c) 2022, Andrew Kaster <akaster@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Function.h>
#include <AK/Vector.h>
#include <LibWebView/ViewImplementation.h>
#include <QObject>
#include <QtCore/private/qandroidextras_p.h>

#ifndef AK_OS_ANDROID
#    error This file is for Android only, check CMake config!
#endif

class WebContentServiceAndroid;

class WebContentServiceConnection : public QAndroidServiceConnection
    , public WebView::ViewImplementation::OSPrivateState {
public:
    WebContentServiceConnection(WebContentServiceAndroid& owner, int ipc_fd, int fd_passing_fd);
    virtual ~WebContentServiceConnection() override;
    Function<void(QAndroidBinder const&)> on_service_connected;

private:
    virtual void onServiceConnected(QString const& name, QAndroidBinder const& service_binder) override;
    virtual void onServiceDisconnected(QString const& name) override;
    QAndroidBinder m_binder;
    WebContentServiceAndroid& m_owner;
    int m_ipc_fd { -1 };
    int m_fd_passing_fd { -1 };
};

class WebContentServiceAndroid : public QObject {
    Q_OBJECT
public:
    explicit WebContentServiceAndroid(QObject* parent = nullptr);
    ~WebContentServiceAndroid();

    OwnPtr<WebContentServiceConnection> add_client(int fd, int fd2);

    static WebContentServiceAndroid& the();

private:
    friend class WebContentServiceConnection;
    Vector<WebContentServiceConnection*> m_connections;
};
