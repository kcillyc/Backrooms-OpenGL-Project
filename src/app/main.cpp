#define GLEW_STATIC

#include <GL/glew.h>
#include <GL/glut.h>
#include <stdio.h>
#include <iostream>
using namespace std;

#include "../ui/hud.h"
#include "../rendering/texture.h"
#include "../objects/chair.h"
#include "../scene/environment.h"

// Window and timing settings
#define WIN_W          900
#define WIN_H          700
#define TIMER_MS       16        // roughly 60 fps
#define LONG_TICKS     38        // how long to hold before spin kicks in (~0.6s)


static const float TEX_SCALE  = 3.0f;    // scales texture coords when building quads

// Rotation and hold-to-spin state

static bool  gHolding = false;   // left mouse button is down
static bool  gLongPress = false; // held long enough to start spinning
static int   gHoldTicks = 0;     // counts timer ticks while held
static bool  gUseTexture = true; // toggle with 't'

/* -------------------------- VBO setup -------------------------- */

// Create VBOs, allocate storage, upload mesh data, set up vertex/texcoord pointers
static GLuint gTexture = 0;

/* ----------------------------- GLUT callbacks ----------------------------- */

static void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawEnvironment();

    Chair::draw(
        gTexture,
        gUseTexture
    );

    Hud::draw(
        WIN_W,
        WIN_H,
        gHolding,
        gLongPress,
        gHoldTicks,
        LONG_TICKS
    );

    glutSwapBuffers();
}

// Timer callback: update hold/spin state, advance rotation, request redraw
static void animate(int value)
{
    (void)value;

    // Track how long the button has been held
    if (gHolding && !gLongPress) {
        gHoldTicks++;
        if (gHoldTicks >= LONG_TICKS) {
            gLongPress = true;
            gHoldTicks = LONG_TICKS;
        }
    }

    Chair::update(
        gLongPress,
        0.016f
    );

    glutPostRedisplay();
    glutTimerFunc(TIMER_MS, animate, 0);
}

// Mouse callback: left button press starts hold tracking, release resets it
static void mouseMonitor(int button, int state, int x, int y)
{
    (void)x;
    (void)y;

    if (button != GLUT_LEFT_BUTTON) return;

    if (state == GLUT_DOWN) {
        gHolding = true;
        gLongPress = false;
        gHoldTicks = 0;
    } else if (state == GLUT_UP) {
        gHolding = false;
        gLongPress = false;
        gHoldTicks = 0;
    }
}

// Keyboard callback: r resets, x quits, t toggles texture
static void keyMonitor(unsigned char key, int x, int y)
{
    (void)x;
    (void)y;

    if (key == 'r' || key == 'R') {
        Chair::resetRotation();
        gHolding = false;
        gLongPress = false;
        gHoldTicks = 0;
    } else if (key == 'x' || key == 'X') {
        exit(0);
    } else if (key == 't' || key == 'T') {
        gUseTexture = !gUseTexture;
    }
}

/* --------------------------------- setup ---------------------------------- */

// Initial OpenGL state, mesh build, VBO upload, texture load
static void init(void)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    initEnvironment();

    Chair::build(TEX_SCALE);
    Chair::initialize();
    gTexture = Texture::load("assets/wood.png");

    printf(
        "chair mesh: %d vertices\n",
        Chair::vertexCount()
    );
    printf("OpenGL version: %s\n", (const char *)glGetString(GL_VERSION));
    printf("OpenGL vendor : %s\n", (const char *)glGetString(GL_VENDOR));
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(
        GLUT_DOUBLE |
        GLUT_RGB |
        GLUT_DEPTH
    );

    glutCreateWindow("The Backrooms");

    GLenum err = glewInit();

    if (err != GLEW_OK) {
        cout << "GLEW Initialization Failed: "
             << glewGetErrorString(err)
             << endl;

        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(
        45.0,
        16.0 / 9.0,
        0.1,
        100.0
    );

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    init();

    glutDisplayFunc(display);
    glutMouseFunc(mouseMonitor);
    glutKeyboardFunc(keyMonitor);
    glutTimerFunc(TIMER_MS, animate, 0);

    glutFullScreen();

    glutMainLoop();

    return 0;
}
