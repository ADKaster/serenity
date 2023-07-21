/*
 * Copyright (c) 2022, Andrew Kaster <akaster@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include "WebContentThread.h"
#include <LibCore/System.h>
#include <QtCore/private/qandroidextras_p.h>

class WebContentBinder : public QAndroidBinder {
public:
    WebContentBinder() = default;

    virtual ~WebContentBinder() override;

private:
    virtual bool onTransact(int code, QAndroidParcel const& data, QAndroidParcel const& reply, QAndroidBinder::CallType flags) override;
    WebContentThread* m_thread { nullptr };
};
