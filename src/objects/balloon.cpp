#include <GL/glew.h>
#include "balloon.h"
#include <cmath>
#include <cstddef>
#include <vector>

namespace Balloon {
namespace {

// Constants for building the 3D ellipsoid balloon mesh
constexpr float PI = 3.14159265358979323846f;
constexpr int STACKS = 24;
constexpr int SLICES = 48;
constexpr int STRIP_VERTICES = (SLICES + 1) * 2;

// Data layout for each vertex (position, lighting normal, and texture coordinates)
struct Vertex {
    GLfloat x, y, z;
    GLfloat nx, ny, nz;
    GLfloat u, v;
};

// Internal state tracking balloon movement, VBO geometry, and latex texture ID
Motion motion;
GLuint bodyVbo = 0;
GLuint latexTexture = 0;

// Calculates a single 3D point, normal vector, and UV coordinate on the balloon's ellipsoid surface
Vertex makeVertex(int row, int column) {
    const float u = static_cast<float>(column) / SLICES;
    const float v = static_cast<float>(row) / STACKS;
    const float longitude = u * 2.0f * PI;
    const float latitude = (v - 0.5f) * PI;

    // Unit sphere coordinates
    const float x = std::cos(latitude) * std::cos(longitude);
    const float y = std::sin(latitude);
    const float z = std::cos(latitude) * std::sin(longitude);

    // Adjust and normalize surface normals for an ellipsoid (0.35 x 0.45 x 0.35)
    float nx = x / 0.35f;
    float ny = y / 0.45f;
    float nz = z / 0.35f;
    const float length = std::sqrt(nx * nx + ny * ny + nz * nz);
    nx /= length;
    ny /= length;
    nz /= length;

    return {0.35f * x, 0.45f * y, 0.35f * z, nx, ny, nz, u, v};
}

// Binds the red latex texture and VBO to render the main balloon body
void drawBody() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, latexTexture);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glColor3f(0.55f, 0.10f, 0.13f); // darker muted red

    // Set up vertex array pointers for position, normals, and texture UVs
    glBindBuffer(GL_ARRAY_BUFFER, bodyVbo);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, sizeof(Vertex),
                    reinterpret_cast<const void*>(offsetof(Vertex, x)));
    glNormalPointer(GL_FLOAT, sizeof(Vertex),
                    reinterpret_cast<const void*>(offsetof(Vertex, nx)));
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex),
                      reinterpret_cast<const void*>(offsetof(Vertex, u)));

    // Render the balloon body one horizontal triangle strip at a time
    for (int row = 0; row < STACKS; ++row) {
        glDrawArrays(GL_TRIANGLE_STRIP, row * STRIP_VERTICES, STRIP_VERTICES);
    }
}

// Draws the pyramid-shaped knot at the bottom of the balloon and the hanging string
void drawKnotAndString() {
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Draw the 4-sided pyramid knot in darker red
    glColor3f(0.48f, 0.025f, 0.04f);
    const GLfloat corners[4][3] = {
        {-0.04f, -0.52f, 0.04f}, {0.04f, -0.52f, 0.04f},
        {0.04f, -0.52f, -0.04f}, {-0.04f, -0.52f, -0.04f}
    };
    const GLfloat normals[4][3] = {
        {0.0f, 0.45f, 1.0f}, {1.0f, 0.45f, 0.0f},
        {0.0f, 0.45f, -1.0f}, {-1.0f, 0.45f, 0.0f}
    };
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 4; ++i) {
        glNormal3fv(normals[i]);
        glVertex3f(0.0f, -0.43f, 0.0f);
        glVertex3fv(corners[i]);
        glVertex3fv(corners[(i + 1) % 4]);
    }
    glEnd();

    // Draw the unlit, slightly curved string dangling below the knot
    const GLfloat stringVertices[] = {
        0.00f, -0.52f, 0.00f,
        0.02f, -0.64f, 0.00f,
        -0.02f, -0.77f, 0.01f,
        0.00f, -0.90f, 0.00f
    };
    glDisable(GL_LIGHTING);
    glColor3f(0.68f, 0.65f, 0.53f);
    glLineWidth(1.0f);
    glVertexPointer(3, GL_FLOAT, 0, stringVertices);
    glDrawArrays(GL_LINE_STRIP, 0, 4);
}

}

// Generates the balloon's 3D mesh VBO and procedural noisy latex texture on startup
void initialize() {
    if (bodyVbo != 0) return;

    // Save current OpenGL buffer and texture bindings so we can restore them later
    GLint previousBuffer = 0;
    GLint previousTexture = 0;
    GLint previousActiveTexture = 0;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &previousBuffer);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &previousActiveTexture);
    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);

    // Build the ellipsoid triangle-strip vertices and upload them to a GPU VBO
    std::vector<Vertex> vertices;
    vertices.reserve(STACKS * STRIP_VERTICES);
    for (int row = 0; row < STACKS; ++row) {
        for (int column = 0; column <= SLICES; ++column) {
            vertices.push_back(makeVertex(row, column));
            vertices.push_back(makeVertex(row + 1, column));
        }
    }
    glGenBuffers(1, &bodyVbo);
    glBindBuffer(GL_ARRAY_BUFFER, bodyVbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
                 vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(previousBuffer));

    // Generate a subtle 64x64 grayscale noise texture to give the balloon a rubbery grain
    constexpr int SIZE = 64;
    unsigned char pixels[SIZE * SIZE * 3];
    for (int y = 0; y < SIZE; ++y) {
        for (int x = 0; x < SIZE; ++x) {
            const int noise = (x * 17 + y * 31 + x * y * 7) % 19;
            const auto value = static_cast<unsigned char>(228 + noise);
            for (int c = 0; c < 3; ++c) pixels[(y * SIZE + x) * 3 + c] = value;
        }
    }
    glGenTextures(1, &latexTexture);
    glBindTexture(GL_TEXTURE_2D, latexTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SIZE, SIZE, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, pixels);

    // Restore previous OpenGL bindings and reset balloon position/scale
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previousTexture));
    glActiveTexture(previousActiveTexture);
    reset();
}

// Applies position, scale, and glossy material settings, then renders the full balloon
void draw() {
    if (bodyVbo == 0) return;

    // Save current OpenGL states and matrices to avoid affecting other scene objects
    GLint previousMode = 0;
    GLint previousBuffer = 0;
    glGetIntegerv(GL_MATRIX_MODE, &previousMode);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &previousBuffer);
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LIGHTING_BIT |
                 GL_TEXTURE_BIT | GL_LINE_BIT);
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glActiveTexture(GL_TEXTURE0);
    glClientActiveTexture(GL_TEXTURE0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    // Move and scale the balloon based on its current motion state
    glTranslatef(motion.position.x, motion.position.y, motion.position.z);
    glScalef(motion.scale(), motion.scale(), motion.scale());

    // Configure smooth shading and shiny specular highlights for latex rubber
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glShadeModel(GL_SMOOTH);
    const GLfloat specular[] = {0.65f, 0.65f, 0.65f, 1.0f};
    const GLfloat emission[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 70.0f);

    // Draw the balloon body, knot, and string
    drawBody();
    drawKnotAndString();

    // Restore previous OpenGL states and matrices
    glPopMatrix();
    glMatrixMode(previousMode);
    glPopClientAttrib();
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(previousBuffer));
    glPopAttrib();
}

// Pass-through helpers forwarding input, frame updates, resets, and state queries to Motion
void onScroll(int direction) { motion.scroll(direction); }
void update(float deltaSeconds) { motion.update(deltaSeconds); }
void reset() { motion.reset(); }
const Motion& state() { return motion; }

// Frees GPU buffer and texture memory when the application closes
void destroy() {
    if (bodyVbo != 0) glDeleteBuffers(1, &bodyVbo);
    if (latexTexture != 0) glDeleteTextures(1, &latexTexture);
    bodyVbo = 0;
    latexTexture = 0;
}

}