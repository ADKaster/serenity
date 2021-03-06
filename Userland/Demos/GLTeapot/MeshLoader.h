/*
 * Copyright (c) 2021, Jesse Buhagiar <jooster669@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/String.h>

#include "Common.h"
#include "Mesh.h"

class MeshLoader {
public:
    MeshLoader() { }
    virtual ~MeshLoader() { }

    virtual RefPtr<Mesh> load(const String& fname) = 0;
};
