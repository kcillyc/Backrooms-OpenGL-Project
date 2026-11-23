#define GLEW_STATIC
#define BUFFER_OFFSET(i) ((void*)(i))

#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut_ext.h>
#include <stdio.h>
#include <stdlib.h>

#include "../ui/hud.h"
#include "../rendering/texture.h"
#include "../rendering/vertex.h"
#include "../objects/chair.h"

// Window and timing settings
#define WIN_W          900
#define WIN_H          700
#define TIMER_MS       16        // roughly 60 fps
#define LONG_TICKS     38        // how long to hold before spin kicks in (~0.6s)
#define MAX_VERTS      2000

// Spin behavior
static const float SPIN_SPEED = 20.0f;   // degrees per frame step (scaled in animate)
static const float TEX_SCALE  = 3.0f;    // scales texture coords when building quads

// Rotation and hold-to-spin state
static float gAngle = 0.0f;      // current Y rotation of the chair
static float gSpin  = 0.0f;      // smoothed spin speed
static bool  gHolding = false;   // left mouse button is down
static bool  gLongPress = false; // held long enough to start spinning
static int   gHoldTicks = 0;     // counts timer ticks while held
static bool  gUseTexture = true; // toggle with 't'

/* -------------------------- VBO setup -------------------------- */

static GLuint gChairVBO = 0;

// Upload n vertices from data into the VBO bound to id
static void uploadVBO(GLuint id, const Vtx* data, int n)
{
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferSubData(GL_ARRAY_BUFFER, 0, n * sizeof(Vtx), data);
}

// Create VBOs, allocate storage, upload mesh data, set up vertex/texcoord pointers
static void initVBOs(void)
{
    // Chair VBO
    glGenBuffers(1, &gChairVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gChairVBO);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERTS * sizeof(Vtx), NULL, GL_STATIC_DRAW);
    uploadVBO(
        gChairVBO,
        Chair::vertices(),
        Chair::vertexCount()
    );

    // Tell OpenGL where position and texcoord data live inside each vertex
    glVertexPointer(3, GL_FLOAT, sizeof(Vtx), BUFFER_OFFSET(0));
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vtx), BUFFER_OFFSET(12));
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

// Bind a VBO and draw n vertices as quads
static void drawVBO(GLuint id, int n)
{
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glVertexPointer(3, GL_FLOAT, sizeof(Vtx), BUFFER_OFFSET(0));
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vtx), BUFFER_OFFSET(12));
    glDrawArrays(GL_QUADS, 0, n);
}
static GLuint gTexture = 0;

/* ----------------------------- GLUT callbacks ----------------------------- */

// Draw the scene: chair (textured or flat color), then HUD
static void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0, 1.05, 1.75, 0.0, 0.48, 0.0, 0.0, 1.0, 0.0);

    // Chair: use wood texture or a fallback brown color
    if (gUseTexture) {
        glBindTexture(GL_TEXTURE_2D, gTexture);
        glEnable(GL_TEXTURE_2D);
        glColor3f(1.0f, 1.0f, 1.0f);
    } else {
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.62f, 0.42f, 0.24f);
    }

    glRotatef(gAngle, 0.0f, 1.0f, 0.0f);
    drawVBO(
        gChairVBO,
        Chair::vertexCount()
    );

    if (!gUseTexture) glColor3f(1.0f, 1.0f, 1.0f);

    Hud::draw(
        WIN_W,
        WIN_H,
        gHolding,
        gLongPress,
        gHoldTicks,
        LONG_TICKS
    );

    glFlush();
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

    // Ease spin speed toward target
    if (gLongPress) {
        gSpin += (SPIN_SPEED - gSpin) * 0.06f;
    } else {
        gSpin += (0.0f - gSpin) * 0.06f;
    }

    // Apply rotation
    gAngle += gSpin * 0.016f;
    if (gAngle > 360.0f) gAngle -= 360.0f;

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
        gAngle = 0.0f;
        gSpin = 0.0f;
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
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)WIN_W / (double)WIN_H, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    Chair::build(TEX_SCALE);
    initVBOs();
    gTexture = Texture::load("assets/wood.png");

    printf(
        "chair mesh: %d vertices\n",
        Chair::vertexCount()
    );
    printf("OpenGL version: %s\n", (const char *)glGetString(GL_VERSION));
    printf("OpenGL vendor : %s\n", (const char *)glGetString(GL_VENDOR));
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(WIN_W, WIN_H);
    glutInitWindowPosition(60, 60);
    glutCreateWindow("Wooden chair - VBO, no lighting");

    glewInit();
    init();

    glutDisplayFunc(display);
    glutMouseFunc(mouseMonitor);
    glutKeyboardFunc(keyMonitor);
    glutTimerFunc(TIMER_MS, animate, 0);

    glutMainLoop();
    return 0;
}
