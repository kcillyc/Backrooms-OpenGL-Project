#pragma once

#include <GL/glew.h>
#include "../rendering/vertex.h"

namespace Chair {
    void build(float textureScale);
    void initialize();

    void update(bool spinning, float deltaSeconds);
    void resetRotation();

    void draw(
        GLuint texture,
        bool useTexture
    );

    int vertexCount();
}