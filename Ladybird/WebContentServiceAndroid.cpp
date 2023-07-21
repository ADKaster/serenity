/*
 * Copyright (c) 2022, Andrew Kaster <akaster@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "WebContentServiceAndroid.h"

#include <AK/Function.h>
#include <AK/OwnPtr.h>
#include <QJniObject>
#include <QtCore/private/qandroidextras_p.h>

#ifndef AK_OS_ANDROID
#    error This file is for Android only, check CMake config!
#endif

WebContentServiceConnection::WebContentServiceConnection(WebContentServiceAndroid& owner, int ipc_fd, int fd_passing_fd)
    : QAndroidServiceConnection()
    , m_owner(owner)
    , m_ipc_fd(ipc_fd)
    , m_fd_passing_fd(fd_passing_fd)
{
    m_owner.m_connections.append(this);
}

WebContentServiceConnection::~WebContentServiceConnection()
{
    m_owner.m_connections.remove_all_matching([this](auto& a) { return a == this; });
    if (m_ipc_fd != -1)
        ::close(m_ipc_fd);
    if (m_fd_passing_fd != -1)
        ::close(m_fd_passing_fd);
}

void WebContentServiceConnection::onServiceConnected(QString const& name, QAndroidBinder const& service_binder)
{
    qDebug() << "Service " << name << " connected";
    m_binder = service_binder;

    qDebug() << "Sending fds " << m_ipc_fd << " and " << m_fd_passing_fd << "to webcontent service";
    QJniObject bundle = QJniObject::callStaticMethod<jobject>(
        "org/serenityos/ladybird/WebContentService",
        "bundleFileDescriptors",
        "(II)Landroid/os/Bundle;",
        m_ipc_fd, m_fd_passing_fd);
    m_ipc_fd = -1;
    m_fd_passing_fd = -1;
    QAndroidParcel data;
    data.handle().callMethod<void>("writeBundle", "(Landroid/os/Bundle;)V", bundle.object());
    m_binder.transact(1, data);
}

void WebContentServiceConnection::onServiceDisconnected(QString const& name)
{
    qDebug() << "Service " << name << " disconnected";
}

static WebContentServiceAndroid* s_the;

WebContentServiceAndroid::WebContentServiceAndroid(QObject* parent)
    : QObject { parent }
{
    s_the = this;
}

WebContentServiceAndroid::~WebContentServiceAndroid()
{
    s_the = nullptr;
}

WebContentServiceAndroid& WebContentServiceAndroid::the()
{
    return *s_the;
}

OwnPtr<WebContentServiceConnection> WebContentServiceAndroid::add_client(int ipc_fd, int fd_passing_fd)
{
    qDebug() << "Binding WebContentService";
    QAndroidIntent intent(QNativeInterface::QAndroidApplication::context(),
        "org/serenityos/ladybird/WebContentService");

    auto connection = make<WebContentServiceConnection>(*this, ipc_fd, fd_passing_fd);

    QtAndroidPrivate::BindFlags bind_flags = { QtAndroidPrivate::BindFlag::AutoCreate };
    [[maybe_unused]] bool ret = QtAndroidPrivate::bindService(intent, *connection, bind_flags);
    VERIFY(ret);

    qDebug() << "WebContentService bound properly (hopefully)";
    return connection;
}
