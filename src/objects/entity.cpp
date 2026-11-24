#include <GL/glut.h>          // freeglut
#include <cmath>
#include <cstdio>
#include <cstdlib>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"        // single-header image loader (see note below)

GLuint tex;
float imgAspect = 1.0f;
int   winW = 800, winH = 600;

const float APPEAR    = 1.0f;  // seconds until it shows up
const float DISAPPEAR = 7.0f;  // seconds until it vanishes
const float FADE      = 0.6f;  // fade in/out duration
const float HEIGHT    = 1.0f;  // entity height in world units (screen is 2 tall)

void display() {
    float totalTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float t = fmodf(totalTime, 10.0f);

    // visibility + fade in/out
    float alpha = 0.0f;
    if (t >= APPEAR && t <= DISAPPEAR) {
        float fadeIn  = fminf((t - APPEAR) / FADE, 1.0f);
        float fadeOut = fminf((DISAPPEAR - t) / FADE, 1.0f);
        alpha = fminf(fadeIn, fadeOut);
    }

    // wandering movement (drifting figure-eight)
    float lt = t - APPEAR;
    //Entity Movement speed
    float progress = fminf(lt / 4.0f, 1.0f);

    //Entity position here if need further fixing
    //where entity starting and finish line
    float startX = -3.5f;
    float endX = 3.5f;
    //Entity pathing
    float x = startX + ((endX - startX) * progress);
    float y = -0.5f;
    float z = -4.0f;

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    if (alpha > 0.0f) {
        float hh = HEIGHT * 0.5f;
        float hw = hh * imgAspect;     // keep the image's aspect ratio

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex);
        glColor4f(1, 1, 1, alpha);     // alpha controls the fade

        glLoadIdentity();
        glTranslatef(x, y, z);

        glBegin(GL_QUADS);
            glTexCoord2f(0, 1); glVertex2f(-hw, -hh);
            glTexCoord2f(1, 1); glVertex2f( hw, -hh);
            glTexCoord2f(1, 0); glVertex2f( hw,  hh);
            glTexCoord2f(0, 0); glVertex2f(-hw,  hh);
        glEnd();

        glDisable(GL_TEXTURE_2D);
    }

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) {
        h = 1;
    }

    winW = w;
    winH = h;

    glViewport(0, 0, w, h);

    float aspect = (float)w / (float)h;
    float viewHeight = 3.0f;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(
        -viewHeight * aspect, // Left
         viewHeight * aspect, // Right
        -viewHeight,          // Bottom
         viewHeight,          // Top
        -10.0f,               // Near Z
         10.0f                // Far Z
    );

    glMatrixMode(GL_MODELVIEW);
}

void idle() { glutPostRedisplay(); }          // redraw continuously

void keyboard(unsigned char key, int, int) {
    if (key == 27) exit(0);                   // ESC quits
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(winW, winH);
    glutCreateWindow("Entity");

    // If you later want shaders/modern GL, add this right after glutCreateWindow:
    //   #include <GL/glew.h>  (before glut.h)
    //   glewInit();

    // load texture
    int w, h, n;
    //can change entity here
    unsigned char* data = stbi_load("Headman.png", &w, &h, &n, 4);
    if (!data) { printf("Could not load Headman.png\n"); return 1; }
    imgAspect = (float)w / h;

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}