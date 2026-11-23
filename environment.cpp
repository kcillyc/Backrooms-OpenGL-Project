#define GLEW_STATIC 
#include <iostream>
#include <GL/glew.h> 
#include <GL/glut.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
using namespace std;

GLuint VBOid, wallTexID;

// Cube used for Environment (Walls, Pillars)
// Format: X, Y, Z,  NormalX, NormalY, NormalZ (4 vertices per face * 6 faces = 24 vertices)
// Format: X, Y, Z,  NormalX, NormalY, NormalZ,  U, V (8 floats per vertex)
GLfloat cubeEnvVertices[] = {
    // Front face
     1.0f,  1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
    -1.0f,  1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f, 1.0f,
    -1.0f, -1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
     1.0f, -1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f,

    // Back face
     1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f, 1.0f,
     1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
    -1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
    -1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,

    // Top face
     1.0f,  1.0f,  1.0f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
     1.0f,  1.0f, -1.0f,   0.0f,  1.0f,  0.0f,   1.0f, 1.0f,
    -1.0f,  1.0f, -1.0f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
    -1.0f,  1.0f,  1.0f,   0.0f,  1.0f,  0.0f,   0.0f, 0.0f,

    // Bottom face
     1.0f, -1.0f,  1.0f,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
    -1.0f, -1.0f,  1.0f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
     1.0f, -1.0f, -1.0f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,

    // Right face
     1.0f,  1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
     1.0f, -1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
     1.0f, -1.0f, -1.0f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
     1.0f,  1.0f, -1.0f,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f,

    // Left face
    -1.0f,  1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
    -1.0f,  1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
    -1.0f, -1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f
};
GLuint loadTexture(const char* filename) {
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    // Repeat the texture if UVs go past 1.0, and use smooth filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(true); // OpenGL expects Y=0 at the bottom
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

void initEnvironment(){
    glGenBuffers(1, &VBOid);
    glBindBuffer(GL_ARRAY_BUFFER, VBOid);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeEnvVertices), cubeEnvVertices, GL_STATIC_DRAW);

    // Enable Fixed-Function Lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);      // Keeps lighting accurate when using glScalef()
    glEnable(GL_COLOR_MATERIAL); // Allows glColor3f() to    set the material color
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glEnable(GL_TEXTURE_2D);
    wallTexID = loadTexture("wallpaper.jpg"); // Put your image in the same folder as the .exe

    // Set light colors (gives nice contrast between lit and shadowed faces)
    GLfloat ambientLight[]  = { 0.30f, 0.28f, 0.20f, 1.0f }; // Soft warm shadow tint
    GLfloat diffuseLight[]  = { 0.90f, 0.88f, 0.75f, 1.0f }; // Fluorescent Backrooms glow
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.08f);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.02f);  
    
    // Fog - Added Feature Searched
    glEnable(GL_FOG);
    GLfloat fogColor[] = { 0.05f, 0.05f, 0.04f, 1.0f }; // Very dark shadow/black
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogi(GL_FOG_MODE, GL_EXP2); // Smooth exponential falloff
    glFogf(GL_FOG_DENSITY, 0.10f); // Increase for thicker darkness, decrease to see further

    // Match background clear color to fog color
    glClearColor(0.05f, 0.05f, 0.04f, 1.0f);
}

void drawCubeEnvironment(GLuint texID, float tileX, float tileY,
                         float posX, float posY, float posZ, 
                         float scaleX, float scaleY, float scaleZ,
                         float rotAngle = 0.0, float rotX = 0.0, float rotY = 0.0, float rotZ = 0.0){

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
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glTranslatef(posX, posY, posZ);
    glRotatef(rotAngle, rotX, rotY, rotZ);
    glScalef(scaleX, scaleY, scaleZ);

    glBindBuffer(GL_ARRAY_BUFFER, VBOid);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    GLsizei stride = 8 * sizeof(GLfloat);
    glVertexPointer(3, GL_FLOAT, stride, (void*)0);
    glNormalPointer(GL_FLOAT, stride, (void*)(3 * sizeof(GLfloat)));
    glTexCoordPointer(2, GL_FLOAT, stride, (void*)(6 * sizeof(GLfloat)));

    glDrawArrays(GL_QUADS, 0, 24);

    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glPopMatrix();

    if (texID != 0) {
        glMatrixMode(GL_TEXTURE);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }
}

void displayEnvironment(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Position the light (w = 0.0 directional point light)
    GLfloat lightPos[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    
    // Ceiling (No Texture: texId = 0)
    glColor3f(0.92f, 0.88f, 0.66f);
    drawCubeEnvironment(0, 1.0f, 1.0f,
                        0.0f, 1.0f, -10.0f,
                        10.0f, 0.1f, 13.0f);

    // Floor (No texture: texID = 0)
    glColor3f(0.68f, 0.62f, 0.38f);
    drawCubeEnvironment(0, 1.0f, 1.0f,
                        0.0f, -1.0f, -10.0f,
                        10.0f, 0.1f, 13.0f);


    // Reset color to pure white so wallpaper texture renders at true brightness
    glColor3f(1.0f, 1.0f, 1.0f);

    // Draw Walls
    // Front Left Wall
    drawCubeEnvironment(wallTexID, 3.0f, 1.0f,
                        -3.5f, 0.0f, -3.5f,
                        1.6f, 1.0f, 0.2f,
                        -3.0f, 0.0f, 1.0f, 0.0f);

    // Back Left Wall
    drawCubeEnvironment(wallTexID, 3.0f, 1.0f,
                        -3.0f, 0.0f, -6.5f,
                        1.6f, 1.0f, 0.2f,
                        -30.0f, 0.0f, 1.0f, 0.0f);

    // Back Left Wall
    drawCubeEnvironment(wallTexID, 3.0f, 1.0f,
                        -1.5f, 0.0f, -10.0f,
                        1.6f, 1.0f, 0.2f,
                        -15.0f, 0.0f, 1.0f, 0.0f);
    
    // Behind Left Wall
    drawCubeEnvironment(wallTexID, 4.0f, 1.0f,
                        0.0f, 0.0f, -16.0f,
                        2.0f, 1.0f, 0.2f,
                        -45.0f, 0.0f, 1.0f, 0.0f);
    
    // Front Right Wall
    drawCubeEnvironment(wallTexID, 3.0f, 1.0f,
                        3.5f, 0.0f, -4.0f,
                        1.6f, 1.0f, 0.2f,
                        5.0f, 0.0f, 1.0f, 0.0f);

    // Back Right Wall
    drawCubeEnvironment(wallTexID, 3.0f, 1.0f,
                        3.0f, 0.0f, -7.0f,
                        1.6f, 1.0f, 0.2f,
                        31.0f, 0.0f, 1.0f, 0.0f);
    
    // Back Right Wall
    drawCubeEnvironment(wallTexID, 3.0f, 1.0f,
                        3.0f, 0.0f, -12.0f,
                        1.5f, 1.0f, 0.3f,
                        -15.0f, 0.0f, 1.0f, 0.0f);

    // Background Wall (No rotation needed, uses default rotAngle = 0.0f)
    drawCubeEnvironment(wallTexID, 10.0f, 1.0f,
                        0.0f, 0.0f, -25.0f,
                        5.0f, 1.0f, 0.2f);

    // Draw Pillars
    // Right Pillar
    drawCubeEnvironment(wallTexID, 0.5f, 1.0f,
                        2.3f, 0.0f, -5.7f,
                        0.15f, 1.0f, 0.15f,
                        45.0f, 0.0f, 1.0f, 0.0f);

    // Left Pillar
    drawCubeEnvironment(wallTexID, 0.5f, 1.0f,
                        -0.6f, 0.0f, -7.0f,
                        0.15f, 1.0f, 0.15f,
                        -30.0f, 0.0f, 1.0f, 0.0f);
    
    glFlush();
}

int main(int argc, char** argv){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glutCreateWindow("The Backrooms");
    glutFullScreen();
    
    GLenum err = glewInit();

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 16.0 / 9.0, 0.1, 100.0);

    initEnvironment();
    glutDisplayFunc(displayEnvironment);

    if (err == GLEW_OK) {
        glutMainLoop();
    }

    
}