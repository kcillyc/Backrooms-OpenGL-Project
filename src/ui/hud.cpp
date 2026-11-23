#include <GL/glut.h>
#include <GL/freeglut_ext.h>
#include "hud.h"

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

    glDisable(GL_TEXTURE_2D);

    drawText(16.0f, windowHeight - 26.0f, "Wooden chair");
    drawText(16.0f, windowHeight - 46.0f, "VBO + texture, no lighting");
    drawText(
        16.0f,
        windowHeight - 66.0f,
        "hold LEFT mouse button for 0.6 s to spin"
    );
    drawText(16.0f, windowHeight - 86.0f, "r reset   x quit");

    if (holding && !longPress) {
        float progress =
            static_cast<float>(holdTicks) /
            static_cast<float>(longTicks);

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

    if (longPress) {
        drawText(
            16.0f,
            windowHeight - 106.0f,
            "spinning - release to stop"
        );
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
}

}