#define GLEW_STATIC

#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../../external/stb_image.h"

#include "texture.h"

namespace Texture {

GLuint load(const char* filename)
{
    int width;
    int height;
    int channels;

    unsigned char* image =
        stbi_load(filename, &width, &height, &channels, 3);

    if (image == nullptr) {
        printf("Failed to load texture: %s\n", filename);
        exit(1);
    }

    GLuint texture = 0;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        width,
        height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        image
    );

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    stbi_image_free(image);

    printf(
        "texture: %d x %d, %d channels\n",
        width,
        height,
        channels
    );

    return texture;
}

}