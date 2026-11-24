#define GLEW_STATIC
#define BUFFER_OFFSET(i) ((void*)(i))

#include <GL/glew.h>
#include "chair.h"
#include <cmath>
#include <cstddef>

// Chair mesh construction, drawing, and rotation
namespace Chair {

// Chair vertex layout: position + lighting normal + texture coords
struct ChairVertex {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
};

// GPU buffer and maximum space reserved for the chair mesh
static GLuint gChairVBO = 0;
static const int MAX_VERTS = 2000;

// Chair mesh lives in this array; gChairN is how many vertices are used
static ChairVertex gChair[MAX_VERTS];
static int gChairN = 0;

// Spin behavior
static const float SPIN_SPEED = 20.0f; // degrees per second (scaled in update)

// Current rotation and smoothed spin speed
static float gAngle = 0.0f;      // current Y rotation of the chair
static float gSpin  = 0.0f;      // smoothed spin speed

/* ---------- small math helpers and mesh storage ---------- */

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

// Absolute value of one vector component
static float abs2(float v)
{
    return v < 0.0f ? -v : v;
}

// Add a quad with an outward face normal and UVs based on its dominant axis
static void addQuad(P3 a, P3 b, P3 c, P3 d, float s)
{
    // Counterclockwise corners give an outward-facing normal
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

    // Give all four corners the same unit normal for crisp, flat box faces.
    const float length = std::sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
    n.x /= length;
    n.y /= length;
    n.z /= length;

    // Append the four corners
    gChair[gChairN++] = ChairVertex{ a.x, a.y, a.z, n.x, n.y, n.z, ua, va };
    gChair[gChairN++] = ChairVertex{ b.x, b.y, b.z, n.x, n.y, n.z, ub, vb };
    gChair[gChairN++] = ChairVertex{ c.x, c.y, c.z, n.x, n.y, n.z, uc, vc };
    gChair[gChairN++] = ChairVertex{ d.x, d.y, d.z, n.x, n.y, n.z, ud, vd };
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
void build(float textureScale)
{
    int sx, sz, i;
    float y;

    gChairN = 0;

    // Seat
    addBox(mk(0.0f, 0.455f, 0.0f), mk(0.225f, 0.018f, 0.205f), textureScale);

    // Four legs (front-left, front-right, back-left, back-right)
    for (sx = -1; sx <= 1; sx += 2) {
        for (sz = -1; sz <= 1; sz += 2) {
            addBox(mk(0.180f * sx, 0.218f, 0.165f * sz),
                   mk(0.020f, 0.218f, 0.020f), textureScale);
        }
    }

    // Two back posts
    for (sx = -1; sx <= 1; sx += 2) {
        addBox(mk(0.180f * sx, 0.635f, -0.175f),
               mk(0.018f, 0.180f, 0.018f), textureScale);
    }

    // Top rail of the backrest
    addBox(mk(0.0f, 0.815f, -0.175f),
           mk(0.205f, 0.022f, 0.018f), textureScale);

    // Three horizontal slats across the back
    for (i = 0; i < 3; i++) {
        y = 0.545f + 0.090f * i;
        addBox(mk(0.0f, y, -0.175f),
               mk(0.160f, 0.014f, 0.012f), textureScale);
    }
}

/* -------------------------- VBO setup -------------------------- */

// Create the VBO, upload the mesh, and set up position/normal/texcoord pointers
void initialize()
{
    glGenBuffers(1, &gChairVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gChairVBO);

    // Reserve enough GPU storage for the maximum vertex count
    glBufferData(
        GL_ARRAY_BUFFER,
        MAX_VERTS * sizeof(ChairVertex),
        NULL,
        GL_STATIC_DRAW
    );

    // Upload only the vertices used by the completed chair mesh
    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        gChairN * sizeof(ChairVertex),
        gChair
    );

    // Tell OpenGL where position, normal, and texcoord data live inside each vertex
    glVertexPointer(
        3,
        GL_FLOAT,
        sizeof(ChairVertex),
        BUFFER_OFFSET(offsetof(ChairVertex, x))
    );

    glNormalPointer(
        GL_FLOAT,
        sizeof(ChairVertex),
        BUFFER_OFFSET(offsetof(ChairVertex, nx))
    );

    glTexCoordPointer(
        2,
        GL_FLOAT,
        sizeof(ChairVertex),
        BUFFER_OFFSET(offsetof(ChairVertex, u))
    );

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

/* -------------------------- chair drawing -------------------------- */

// Draw the chair with the scene lighting and either wood texture or a brown color
void draw(
    GLuint texture,
    bool useTexture
)
{
    // Use the wood texture or a fallback brown color
    if (useTexture) {
        glBindTexture(GL_TEXTURE_2D, texture);
        glEnable(GL_TEXTURE_2D);
        glColor3f(1.0f, 1.0f, 1.0f);
    } else {
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.62f, 0.42f, 0.24f);
    }


    glPushMatrix();

    // Position the chair in the room, then apply its current Y rotation
    glTranslatef(0.0f, -1.0f, -4.0f);
    glRotatef(gAngle, 0.0f, 1.0f, 0.0f);

    glBindBuffer(GL_ARRAY_BUFFER, gChairVBO);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    // Tell OpenGL where position, normal, and texcoord data live inside each vertex
    glVertexPointer(
        3,
        GL_FLOAT,
        sizeof(ChairVertex),
        BUFFER_OFFSET(offsetof(ChairVertex, x))
    );

    glNormalPointer(
        GL_FLOAT,
        sizeof(ChairVertex),
        BUFFER_OFFSET(offsetof(ChairVertex, nx))
    );

    glTexCoordPointer(
        2,
        GL_FLOAT,
        sizeof(ChairVertex),
        BUFFER_OFFSET(offsetof(ChairVertex, u))
    );

    // Draw all box faces using their stored lighting normals
    glDrawArrays(GL_QUADS, 0, gChairN);

    // Restore the transform and stop using the chair's normal array
    glPopMatrix();

    glDisableClientState(GL_NORMAL_ARRAY);

    // Reset the color for the next object or HUD
    glColor3f(1.0f, 1.0f, 1.0f);
}

/* -------------------------- rotation updates -------------------------- */

// Ease the spin speed toward its target and advance the chair rotation
void update(bool spinning, float deltaSeconds)
{
    // Gradually speed up while spinning, or slow down after release
    if (spinning) {
        gSpin += (SPIN_SPEED - gSpin) * 0.06f;
    } else {
        gSpin += (0.0f - gSpin) * 0.06f;
    }

    // Apply rotation using the elapsed time in seconds
    gAngle += gSpin * deltaSeconds;

    // Wrap the angle after a full turn
    if (gAngle > 360.0f) {
        gAngle -= 360.0f;
    }
}

// Return the chair to its starting angle and stop any remaining spin
void resetRotation()
{
    gAngle = 0.0f;
    gSpin = 0.0f;
}

// Return the number of vertices used by the chair mesh
int vertexCount()
{
    return gChairN;
}

}