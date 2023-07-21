/*
 * Copyright (c) 2022, Andrew Kaster <akaster@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "WebContentBinder.h"

WebContentBinder::~WebContentBinder()
{
    m_thread->quit();
    m_thread->wait();
}

bool WebContentBinder::onTransact(int code, QAndroidParcel const& data, QAndroidParcel const& reply, QAndroidBinder::CallType flags)
{
    qDebug() << "WebContentBinder: onTransact(), code " << code << ", flags " << int(flags);

    if (code != 1) {
        qDebug() << "What is code " << code << "?";
        return false;
    }

    auto parcel = data.handle();

    qDebug() << "calling readBundle";
    QJniObject bundle = parcel.callMethod<jobject>("readBundle", "()Landroid/os/Bundle;");

    qDebug() << "creating first string";
    auto ipc_key = QJniObject::fromString("IPC_SOCKET");
    qDebug() << "Getting first parceled fd";
    QJniObject ipc_fd_handle = bundle.callMethod<jobject>("getParcelable", "(Ljava/lang/String;)Landroid/os/Parcelable;", ipc_key.object<jstring>());
    qDebug() << "creating second string";
    auto pass_key = QJniObject::fromString("FD_PASSING_SOCKET");
    qDebug() << "Getting second parceled fd";
    QJniObject fd_passing_fd_handle = bundle.callMethod<jobject>("getParcelable", "(Ljava/lang/String;)Landroid/os/Parcelable;", pass_key.object<jstring>());

    qDebug() << "detaching first fd";
    int ipc_fd = ipc_fd_handle.callMethod<int>("detachFd");
    qDebug() << "detaching second fd";
    int fd_passing_fd = fd_passing_fd_handle.callMethod<int>("detachFd");

    auto takeover_string = DeprecatedString::formatted("WebContent:{}", ipc_fd);
    MUST(Core::System::setenv("SOCKET_TAKEOVER"sv, takeover_string, true));

    qDebug() << "Creating WebContentThread";
    m_thread = new WebContentThread(fd_passing_fd);
    m_thread->start();
    QObject::connect(m_thread, &WebContentThread::finished, m_thread, &QObject::deleteLater);

    qDebug() << "Replying with ok";
    reply.writeVariant("OK");
    return true;
}
