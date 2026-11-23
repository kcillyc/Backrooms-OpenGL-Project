#define GLEW_STATIC
#define BUFFER_OFFSET(i) ((void*)(i))

#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut_ext.h>
#include <stdio.h>
#include <stdlib.h>
#include "../ui/hud.h"

// stb_image pulls in the implementation here (only once in the whole project)
#define STB_IMAGE_IMPLEMENTATION
#include "../../external/stb_image.h"

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

/* ---------- small math helpers and mesh storage ---------- */

// Simple 3D point/vector
typedef struct {
    float x, y, z;
} P3;

// Vertex used for both chair and floor: position + texture coords
typedef struct {
    float x, y, z;
    float u, v;
} Vtx;

// Make a P3 quickly
static P3 mk(float x, float y, float z)
{
    P3 p;
    p.x = x; p.y = y; p.z = z;
    return p;
}

// Vector from b to a (a - b)
static P3 sub(P3 a, P3 b)
{
    return mk(a.x - b.x, a.y - b.y, a.z - b.z);
}

// Cross product of a and b
static P3 cross(P3 a, P3 b)
{
    return mk(a.y * b.z - a.z * b.y,
              a.z * b.x - a.x * b.z,
              a.x * b.y - a.y * b.x);
}

// Absolute value without pulling in math.h
static float abs2(float v)
{
    return v < 0.0f ? -v : v;
}

// Chair mesh lives in this array; gChairN is how many vertices are used
static Vtx gChair[MAX_VERTS];
static int  gChairN = 0;

// Floor is just a single quad (4 vertices)
static Vtx  gFloor[4];

// Add a quad (a,b,c,d) to the chair mesh, generating simple UVs based on dominant normal axis
static void addQuad(P3 a, P3 b, P3 c, P3 d, float s)
{
    P3 n = cross(sub(b, a), sub(d, a));
    float ax = abs2(n.x), ay = abs2(n.y), az = abs2(n.z);
    float ua, va, ub, vb, uc, vc, ud, vd;

    // Pick which 2D plane gives reasonable texture coords depending on face direction
    if (ax >= ay && ax >= az) {
        ua = a.z * s; va = a.y * s;
        ub = b.z * s; vb = b.y * s;
        uc = c.z * s; vc = c.y * s;
        ud = d.z * s; vd = d.y * s;
    } else if (ay >= ax && ay >= az) {
        ua = a.x * s; va = a.z * s;
        ub = b.x * s; vb = b.z * s;
        uc = c.x * s; vc = c.z * s;
        ud = d.x * s; vd = d.z * s;
    } else {
        ua = a.x * s; va = a.y * s;
        ub = b.x * s; vb = b.y * s;
        uc = c.x * s; vc = c.y * s;
        ud = d.x * s; vd = d.y * s;
    }

    // Append the four corners
    gChair[gChairN++] = (Vtx){ a.x, a.y, a.z, ua, va };
    gChair[gChairN++] = (Vtx){ b.x, b.y, b.z, ub, vb };
    gChair[gChairN++] = (Vtx){ c.x, c.y, c.z, uc, vc };
    gChair[gChairN++] = (Vtx){ d.x, d.y, d.z, ud, vd };
}

// Build an axis-aligned box centered at c with half-extents h, then add all 6 faces
static void addBox(P3 c, P3 h, float s)
{
    P3 p000 = mk(c.x - h.x, c.y - h.y, c.z - h.z);
    P3 p001 = mk(c.x - h.x, c.y - h.y, c.z + h.z);
    P3 p010 = mk(c.x - h.x, c.y + h.y, c.z - h.z);
    P3 p011 = mk(c.x - h.x, c.y + h.y, c.z + h.z);
    P3 p100 = mk(c.x + h.x, c.y - h.y, c.z - h.z);
    P3 p101 = mk(c.x + h.x, c.y - h.y, c.z + h.z);
    P3 p110 = mk(c.x + h.x, c.y + h.y, c.z - h.z);
    P3 p111 = mk(c.x + h.x, c.y + h.y, c.z + h.z);

    addQuad(p001, p101, p111, p011, s); // front
    addQuad(p100, p000, p010, p110, s); // back
    addQuad(p000, p001, p011, p010, s); // left
    addQuad(p101, p100, p110, p111, s); // right
    addQuad(p010, p011, p111, p110, s); // top
    addQuad(p000, p100, p101, p001, s); // bottom
}

// Construct the chair mesh from boxes: seat, legs, back posts, rail, slats
static void buildChair(void)
{
    int sx, sz, i;
    float y;

    gChairN = 0;

    // Seat
    addBox(mk(0.0f, 0.455f, 0.0f), mk(0.225f, 0.018f, 0.205f), TEX_SCALE);

    // Four legs (front-left, front-right, back-left, back-right)
    for (sx = -1; sx <= 1; sx += 2) {
        for (sz = -1; sz <= 1; sz += 2) {
            addBox(mk(0.180f * sx, 0.218f, 0.165f * sz),
                   mk(0.020f, 0.218f, 0.020f), TEX_SCALE);
        }
    }

    // Two back posts
    for (sx = -1; sx <= 1; sx += 2) {
        addBox(mk(0.180f * sx, 0.635f, -0.175f),
               mk(0.018f, 0.180f, 0.018f), TEX_SCALE);
    }

    // Top rail of the backrest
    addBox(mk(0.0f, 0.815f, -0.175f),
           mk(0.205f, 0.022f, 0.018f), TEX_SCALE);

    // Three horizontal slats across the back
    for (i = 0; i < 3; i++) {
        y = 0.545f + 0.090f * i;
        addBox(mk(0.0f, y, -0.175f),
               mk(0.160f, 0.014f, 0.012f), TEX_SCALE);
    }
}

// Build a large flat floor quad under the chair
static void buildFloor(void)
{
    float s = 6.0f;
    float h = -0.005f;
    gFloor[0] = (Vtx){ -s, h, -s, -s, -s };
    gFloor[1] = (Vtx){  s, h, -s,  s, -s };
    gFloor[2] = (Vtx){  s, h,  s,  s,  s };
    gFloor[3] = (Vtx){ -s, h,  s, -s,  s };
}

/* -------------------------- VBO setup -------------------------- */

static GLuint gChairVBO = 0;
static GLuint gFloorVBO = 0;

// Upload n vertices from data into the VBO bound to id
static void uploadVBO(GLuint id, Vtx *data, int n)
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
    uploadVBO(gChairVBO, gChair, gChairN);

    // Floor VBO
    glGenBuffers(1, &gFloorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gFloorVBO);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vtx), NULL, GL_STATIC_DRAW);
    uploadVBO(gFloorVBO, gFloor, 4);

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

/* ---------------------- texture loading (stb_image) ---------------------- */

static GLuint gTexture = 0;

// Load wood.png from disk and upload it as a GL texture
static void loadTexture(void)
{
    int w, h, channels;
    unsigned char *img = stbi_load("assets/wood.png", &w, &h, &channels, 3);

    if (img == NULL) {
        printf("Failed to load texture: wood.png\n");
        exit(1);
    }

    glGenTextures(1, &gTexture);
    glBindTexture(GL_TEXTURE_2D, gTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, img);

    // Modulate texture color with current vertex/color state
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    printf("texture: %d x %d, %d channels\n", w, h, channels);
}


/* ----------------------------- GLUT callbacks ----------------------------- */

// Draw the scene: floor, then chair (textured or flat color), then HUD
static void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0, 1.05, 1.75, 0.0, 0.48, 0.0, 0.0, 1.0, 0.0);

    // Draw floor with a fixed color (no texture)
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.22f, 0.34f, 0.40f);
    drawVBO(gFloorVBO, 4);

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
    drawVBO(gChairVBO, gChairN);

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

    buildChair();
    buildFloor();
    initVBOs();
    loadTexture();

    printf("chair mesh: %d vertices\n", gChairN);
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
