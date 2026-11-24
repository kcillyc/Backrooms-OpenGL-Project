// TODO: Add Instructions for the Balloon:DONE
#include <GL/glut.h>
#include <GL/freeglut_ext.h>
#include <cstdio>
#include "hud.h"
#include "../objects/balloon.h"

namespace Hud {

static void drawText(float x, float y, const char* text)
{
    glRasterPos2f(x, y);
    glutBitmapString(
        GLUT_BITMAP_9_BY_15,
        reinterpret_cast<const unsigned char*>(text)
    );
}

void draw(
    float windowWidth,
    float windowHeight,
    bool holding,
    bool longPress,
    int holdTicks,
    int longTicks
)
{
    glPushAttrib(
        GL_ENABLE_BIT |
        GL_CURRENT_BIT |
        GL_TRANSFORM_BIT
    );

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    gluOrtho2D(
        0.0,
        static_cast<GLdouble>(windowWidth),
        0.0,
        static_cast<GLdouble>(windowHeight)
    );

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(0.96f, 0.93f, 0.78f);

    drawText(16.0f, windowHeight - 26.0f, "Wooden chair");
    drawText(16.0f, windowHeight - 46.0f, "VBO + texture, no lighting");

    drawText(
        16.0f,
        windowHeight - 66.0f,
        "hold LEFT mouse button for 0.6 s to spin"
    );

    drawText(16.0f, windowHeight - 86.0f, "r reset   x quit");

    if (holding && !longPress) {
        float progress = longTicks > 0
            ? static_cast<float>(holdTicks) /
              static_cast<float>(longTicks)
            : 0.0f;

        if (progress > 1.0f) {
            progress = 1.0f;
        }

        glColor3f(0.85f, 0.70f, 0.35f);

        glBegin(GL_QUADS);
            glVertex2f(16.0f, windowHeight - 102.0f);
            glVertex2f(
                16.0f + 220.0f * progress,
                windowHeight - 102.0f
            );
            glVertex2f(
                16.0f + 220.0f * progress,
                windowHeight - 97.0f
            );
            glVertex2f(16.0f, windowHeight - 97.0f);
        glEnd();
    }

    glColor3f(0.96f, 0.93f, 0.78f);

    if (longPress) {
        drawText(
            16.0f,
            windowHeight - 106.0f,
            "spinning - release to stop"
        );
    }

    drawText(
        16.0f,
        windowHeight - 136.0f,
        "Balloon | Scroll up/down: size"
    );

    const auto& s = Balloon::state();

    const char* phase = !s.released
        ? "Inflate to 1.8x to release"
        : s.progress < 1.0f
            ? "Floating toward the chair"
            : "Beside the chair";

    char status[160];

    std::snprintf(
        status,
        sizeof(status),
        "Scale: %.1fx | %s",
        s.scale(),
        phase
    );

    drawText(16.0f, windowHeight - 156.0f, status);

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glPopAttrib();
}

}
