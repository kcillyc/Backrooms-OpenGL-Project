#define GLEW_STATIC
#include <iostream>
#include <GL/glew.h>
#include <GL/glut.h>

#include "environment.h"
#include "../../external/stb_image.h"

using namespace std;

// ============================================================================
// GLOBAL VARIABLES & GEOMETRY DATA
// ============================================================================
GLuint VBOid, wallTexID;

// Unit cube mesh used for all environment geometry (Walls, Pillars, Floor, Ceiling)
// Format: Position (X, Y, Z), Normal (NX, NY, NZ), Texture UV (U, V) -> 8 floats/vertex
GLfloat cubeEnvVertices[] = {
    // Front face (+Z)
     1.0f,  1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
    -1.0f,  1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f, 1.0f,
    -1.0f, -1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
     1.0f, -1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f,

    // Back face (-Z)
     1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f, 1.0f,
     1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
    -1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
    -1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,

    // Top face (+Y)
     1.0f,  1.0f,  1.0f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
     1.0f,  1.0f, -1.0f,   0.0f,  1.0f,  0.0f,   1.0f, 1.0f,
    -1.0f,  1.0f, -1.0f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
    -1.0f,  1.0f,  1.0f,   0.0f,  1.0f,  0.0f,   0.0f, 0.0f,

    // Bottom face (-Y)
     1.0f, -1.0f,  1.0f,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
    -1.0f, -1.0f,  1.0f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
     1.0f, -1.0f, -1.0f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,

    // Right face (+X)
     1.0f,  1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
     1.0f, -1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
     1.0f, -1.0f, -1.0f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
     1.0f,  1.0f, -1.0f,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f,

    // Left face (-X)
    -1.0f,  1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
    -1.0f,  1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
    -1.0f, -1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f
};

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================
GLuint loadTexture(const char* filename);
void initLightingAndFog();
void initEnvironment();
void drawCubeEnvironment(GLuint texID, float tileX, float tileY,
                         float posX, float posY, float posZ,
                         float scaleX, float scaleY, float scaleZ,
                         float rotAngle = 0.0f, float rotX = 0.0f, float rotY = 0.0f, float rotZ = 0.0f);
void drawEnvironment();

// ============================================================================
// ASSET & RESOURCE INITIALIZATION
// ============================================================================

// Loads an image from disk and uploads it as a repeating 2D OpenGL texture
GLuint loadTexture(const char* filename) {
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    // Enable repeating UVs and smooth linear filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(true); // Flip Y-axis to match OpenGL UV space
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);

    if (data) {
        GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    } else {
        cout << "Failed to load texture: " << filename << endl;
    }

    stbi_image_free(data);
    return texID;
}

// Configures single spotlight (GL_LIGHT0) and exponential distance fog
void initLightingAndFog() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);      // Keeps lighting accurate when objects are scaled
    glEnable(GL_COLOR_MATERIAL); // Allows glColor3f() to tint materials/textures
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // 1. Keep shadows pitch black, max out diffuse brightness
    GLfloat ambientLight[]  = { 0.03f, 0.03f, 0.02f, 1.0f };
    GLfloat diffuseLight[]  = { 1.00f, 0.96f, 0.78f, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);

    // 2. Attenuation so nearby walls & pillars are brightly lit while far ones fade
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION,  0.15f);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION,    0.14f);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.05f);

    // 3. Tight flashlight spotlight cone
    glLightf(GL_LIGHT0, GL_SPOT_CUTOFF,   36.0f);
    glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 10.0f);

    // 4. Exponential distance fog so distant walls dissolve into darkness
    glEnable(GL_FOG);
    GLfloat fogColor[] = { 0.01f, 0.01f, 0.01f, 1.0f };
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogi(GL_FOG_MODE, GL_EXP2);
    glFogf(GL_FOG_DENSITY, 0.10f);
    glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
}

// Initializes VBO geometry, textures, lighting, and fog
void initEnvironment() {
    // Upload cube vertex data to GPU
    glGenBuffers(1, &VBOid);
    glBindBuffer(GL_ARRAY_BUFFER, VBOid);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeEnvVertices), cubeEnvVertices, GL_STATIC_DRAW);

    // Load environment textures
    glEnable(GL_TEXTURE_2D);
    wallTexID = loadTexture("assets/wallpaper.jpg");

    // Configure scene lighting & atmosphere
    initLightingAndFog();
}

// ============================================================================
// RENDERING & SCENE CONSTRUCTION
// ============================================================================

// Draws a transformed cube with optional repeating texture (pass texID = 0 for untextured)
void drawCubeEnvironment(GLuint texID, float tileX, float tileY,
                         float posX, float posY, float posZ,
                         float scaleX, float scaleY, float scaleZ,
                         float rotAngle, float rotX, float rotY, float rotZ) {
    // 1. Configure texture binding and UV repeat scale
    if (texID != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texID);
        glMatrixMode(GL_TEXTURE);
        glPushMatrix();
        glLoadIdentity();
        glScalef(tileX, tileY, 1.0f);
    } else {
        glDisable(GL_TEXTURE_2D);
    }

    // 2. Apply world transforms (Translate -> Rotate -> Scale)
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(posX, posY, posZ);
    glRotatef(rotAngle, rotX, rotY, rotZ);
    glScalef(scaleX, scaleY, scaleZ);

    // 3. Bind VBO attributes and draw
    glBindBuffer(GL_ARRAY_BUFFER, VBOid);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    GLsizei stride = 8 * sizeof(GLfloat);
    glVertexPointer(3, GL_FLOAT, stride, (void*)0);
    glNormalPointer(GL_FLOAT, stride, (void*)(3 * sizeof(GLfloat)));
    glTexCoordPointer(2, GL_FLOAT, stride, (void*)(6 * sizeof(GLfloat)));

    glDrawArrays(GL_QUADS, 0, 24);

    // 4. Clean up client states and restore matrices
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glPopMatrix();

    if (texID != 0) {
        glMatrixMode(GL_TEXTURE);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }
}

// Main display callback: positions camera spotlight and renders the Backrooms layout
void drawEnvironment() {

    // Lock spotlight to the camera's origin looking down -Z
    GLfloat lightPos[]      = { 0.0f, 0.0f,  0.0f, 1.0f };
    GLfloat spotDirection[] = { 0.0f, 0.0f, -1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION,       lightPos);
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spotDirection);

    // --- Ceiling & Floor (Untextured) ---
    glColor3f(0.92f, 0.88f, 0.66f); // Pale acoustic tile yellow
    drawCubeEnvironment(0, 1.0f, 1.0f,   0.0f,  1.0f, -10.0f,   10.0f, 0.1f, 13.0f);

    glColor3f(0.68f, 0.62f, 0.38f); // Damp carpet yellow-beige
    drawCubeEnvironment(0, 1.0f, 1.0f,   0.0f, -1.0f, -10.0f,   10.0f, 0.1f, 13.0f);

    // --- Textured Walls (Reset color to white for true texture brightness) ---
    glColor3f(1.0f, 1.0f, 1.0f);

    // Left-side walls (front to back)
    drawCubeEnvironment(wallTexID, 3.0f, 1.0f,  -3.5f, 0.0f,  -3.5f,   1.6f, 1.0f, 0.2f,  -3.0f, 0.0f, 1.0f, 0.0f);
    drawCubeEnvironment(wallTexID, 3.0f, 1.0f,  -3.0f, 0.0f,  -6.5f,   1.6f, 1.0f, 0.2f, -30.0f, 0.0f, 1.0f, 0.0f);
    drawCubeEnvironment(wallTexID, 3.0f, 1.0f,  -1.5f, 0.0f, -10.0f,   1.6f, 1.0f, 0.2f, -15.0f, 0.0f, 1.0f, 0.0f);
    drawCubeEnvironment(wallTexID, 4.0f, 1.0f,   0.0f, 0.0f, -16.0f,   2.0f, 1.0f, 0.2f, -45.0f, 0.0f, 1.0f, 0.0f);

    // Right-side walls (front to back)
    drawCubeEnvironment(wallTexID, 3.0f, 1.0f,   3.5f, 0.0f,  -4.0f,   1.6f, 1.0f, 0.2f,   5.0f, 0.0f, 1.0f, 0.0f);
    drawCubeEnvironment(wallTexID, 3.0f, 1.0f,   3.0f, 0.0f,  -7.0f,   1.6f, 1.0f, 0.2f,  31.0f, 0.0f, 1.0f, 0.0f);
    drawCubeEnvironment(wallTexID, 3.0f, 1.0f,   3.0f, 0.0f, -12.0f,   1.5f, 1.0f, 0.3f, -15.0f, 0.0f, 1.0f, 0.0f);

    // Far background wall
    drawCubeEnvironment(wallTexID, 10.0f, 1.0f,  0.0f, 0.0f, -25.0f,   5.0f, 1.0f, 0.2f);

    // --- Pillars ---
    drawCubeEnvironment(wallTexID, 0.5f, 1.0f,   2.3f, 0.0f,  -5.7f,   0.15f, 1.0f, 0.15f,  45.0f, 0.0f, 1.0f, 0.0f);
    drawCubeEnvironment(wallTexID, 0.5f, 1.0f,  -0.6f, 0.0f,  -7.0f,   0.15f, 1.0f, 0.15f, -30.0f, 0.0f, 1.0f, 0.0f);

}