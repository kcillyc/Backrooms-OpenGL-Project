#pragma once

#include "../rendering/vertex.h"

namespace Chair {
    void build(float textureScale);

    const Vtx* vertices();
    int vertexCount();
}