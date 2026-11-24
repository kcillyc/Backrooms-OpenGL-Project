#define GLEW_STATIC

#include <GL/glew.h>
#include <cmath>
#include <cstdio>

#include "../../external/stb_image.h"
#include "entity.h"

namespace Entity {
namespace {

GLuint texture = 0;
float imageAspect = 1.0f;
float elapsedTime = 0.0f;

constexpr float APPEAR = 1.0f;
constexpr float DISAPPEAR = 7.0f;
constexpr float FADE = 0.6f;
constexpr float HEIGHT = 1.7f;

}

void initialize()
{
    int width;
    int height;
    int channels;

    // Load the entity image from disk and force it to RGBA
    // so the image can use transparency.
    unsigned char* image =
        stbi_load(
            "assets/entity.png",
            &width,
            &height,
            &channels,
            4
        );

    if (!image) {
        std::printf("Could not load assets/entity.png\n");
        return;
    }

    // Preserve the original image proportions when creating the billboard.
    imageAspect =
        static_cast<float>(width) /
        static_cast<float>(height);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        image
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(image);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void update(float deltaSeconds)
{
    // Keep animation time independent from the rendering function.
    elapsedTime += deltaSeconds;
}

void draw()
{
    if (texture == 0) {
        return;
    }

    // Repeat the entity animation every ten seconds.
    const float t =
        std::fmod(elapsedTime, 10.0f);

    float alpha = 0.0f;

    // The entity is invisible outside its appearance interval.
    if (t >= APPEAR && t <= DISAPPEAR) {
        
        // Fade in at the beginning and fade out near the end
        // of the visibility interval.
        const float fadeIn =
            std::fmin((t - APPEAR) / FADE, 1.0f);

        const float fadeOut =
            std::fmin((DISAPPEAR - t) / FADE, 1.0f);

        alpha = std::fmin(fadeIn, fadeOut);
    }

    if (alpha <= 0.0f) {
        return;
    }

    // Move the entity from left to right over nine seconds.
    // The longer duration makes the movement slower.
    const float progress =
        std::fmin((t - APPEAR) / 9.0f, 1.0f);

    const float x =
        -3.5f + (3.5f - (-3.5f)) * progress;

    // Place the entity deep inside the environment.
    // More negative Z places it farther from the camera.
    const float y = 0.0f;
    const float z = -8.0f;

    const float halfHeight = HEIGHT * 0.5f;
    const float halfWidth = halfHeight * imageAspect;

    // Save the current OpenGL state so the entity does not affect
    // the environment, chair, balloon, or HUD.
    glPushAttrib(
        GL_ENABLE_BIT |
        GL_CURRENT_BIT |
        GL_TEXTURE_BIT
    );

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, texture);
    // Make the entity fade into the floor by reducing the alpha
    // of its bottom vertices.
    // A value of 0.0 makes the bottom completely transparent.
    const float bottomAlpha = alpha * 0.0f;

    glPushMatrix();

    // Position the textured quad in world space.
    glTranslatef(x, y, z);

    glBegin(GL_QUADS);
        // Bottom-left: mostly transparent
        glColor4f(1.0f, 1.0f, 1.0f, bottomAlpha);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-halfWidth, -halfHeight, 0.0f);

        // Bottom-right: mostly transparent
        glColor4f(1.0f, 1.0f, 1.0f, bottomAlpha);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(halfWidth, -halfHeight, 0.0f);

        // Top-right: fully visible
        glColor4f(1.0f, 1.0f, 1.0f, alpha);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(halfWidth, halfHeight, 0.0f);

        // Top-left: fully visible
        glColor4f(1.0f, 1.0f, 1.0f, alpha);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-halfWidth, halfHeight, 0.0f);
    glEnd();

    // Restore the previous lighting, blending, texture, and color state.
    glPopMatrix();
    glPopAttrib();
}

void destroy()
{
    if (texture != 0) {
        glDeleteTextures(1, &texture);
        texture = 0;
    }
}

}