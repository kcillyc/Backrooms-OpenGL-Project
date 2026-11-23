#define GLEW_STATIC 
#include<iostream>
#include <GL/glew.h> 
#include <GL/glut.h>
using namespace std;

GLuint VBOid;

// Cube used for Environment (Walls, Pillars)
// Format: X, Y, Z,  NormalX, NormalY, NormalZ (4 vertices per face * 6 faces = 24 vertices)
GLfloat cubeEnvVertices[] = {
    // Front face (Normal: 0, 0, 1)
     1.0f,  1.0f,  1.0f,   0.0f,  0.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,   0.0f,  0.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,   0.0f,  0.0f,  1.0f,
     1.0f, -1.0f,  1.0f,   0.0f,  0.0f,  1.0f,

    // Back face (Normal: 0, 0, -1)
     1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,
     1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,

    // Top face (Normal: 0, 1, 0)
     1.0f,  1.0f,  1.0f,   0.0f,  1.0f,  0.0f,
     1.0f,  1.0f, -1.0f,   0.0f,  1.0f,  0.0f,
    -1.0f,  1.0f, -1.0f,   0.0f,  1.0f,  0.0f,
    -1.0f,  1.0f,  1.0f,   0.0f,  1.0f,  0.0f,

    // Bottom face (Normal: 0, -1, 0)
     1.0f, -1.0f,  1.0f,   0.0f, -1.0f,  0.0f,
    -1.0f, -1.0f,  1.0f,   0.0f, -1.0f,  0.0f,
    -1.0f, -1.0f, -1.0f,   0.0f, -1.0f,  0.0f,
     1.0f, -1.0f, -1.0f,   0.0f, -1.0f,  0.0f,

    // Right face (Normal: 1, 0, 0)
     1.0f,  1.0f,  1.0f,   1.0f,  0.0f,  0.0f,
     1.0f, -1.0f,  1.0f,   1.0f,  0.0f,  0.0f,
     1.0f, -1.0f, -1.0f,   1.0f,  0.0f,  0.0f,
     1.0f,  1.0f, -1.0f,   1.0f,  0.0f,  0.0f,

    // Left face (Normal: -1, 0, 0)
    -1.0f,  1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,
    -1.0f,  1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,
    -1.0f, -1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,
    -1.0f, -1.0f,  1.0f,  -1.0f,  0.0f,  0.0f
};

void initEnvironment(){
    glGenBuffers(1, &VBOid);
    glBindBuffer(GL_ARRAY_BUFFER, VBOid);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeEnvVertices), cubeEnvVertices, GL_STATIC_DRAW);

    // Enable Fixed-Function Lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);      // Keeps lighting accurate when using glScalef()
    glEnable(GL_COLOR_MATERIAL); // Allows glColor3f() to set the material color
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Set light colors (gives nice contrast between lit and shadowed faces)
    GLfloat ambientLight[]  = { 0.30f, 0.28f, 0.20f, 1.0f }; // Soft warm shadow tint
    GLfloat diffuseLight[]  = { 0.90f, 0.88f, 0.75f, 1.0f }; // Fluorescent Backrooms glow
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.08f);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.02f);    
}

void drawCubeEnvironment(float posX, float posY, float posZ, 
                         float scaleX, float scaleY, float scaleZ,
                         float rotAngle = 0.0, float rotX = 0.0, float rotY = 0.0, float rotZ = 0.0){
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glTranslatef(posX, posY, posZ);
    glRotatef(rotAngle, rotX, rotY, rotZ);
    glScalef(scaleX, scaleY, scaleZ);

    glBindBuffer(GL_ARRAY_BUFFER, VBOid);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    GLsizei stride = 6 * sizeof(GLfloat);
    glVertexPointer(3, GL_FLOAT, stride, (void*)0);
    glNormalPointer(GL_FLOAT, stride, (void*)(3 * sizeof(GLfloat)));

    glDrawArrays(GL_QUADS, 0, 24);

    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glPopMatrix();
}

void displayEnvironment(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Position the light (w = 0.0 directional point light)
    GLfloat lightPos[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    

    // Draw Walls - Arranged by Z Distance
    // Front Left Wall
    drawCubeEnvironment(-3.5, 0.0, -3.5,            // Position
                        1.6, 1.0, 0.2,              // Scale
                        -3.0, 0.0, 1.0, 0.0);       // Rotation

    // Back Left Wall
    drawCubeEnvironment(-3.0, 0.0, -6.5,            // Position
                        1.6, 1.0, 0.2,              // Scale
                        -30.0, 0.0, 1.0, 0.0);      // Rotation

    // Back Left Wall
    drawCubeEnvironment(-1.5, 0.0, -10.0,           // Position
                        1.6, 1.0, 0.2,              // Scale
                        -15.0, 0.0, 1.0, 0.0);      // Rotation
    
    // Behind Left Wall
    drawCubeEnvironment(0.0, 0.0, -16.0,            // Position
                        2.0, 1.0, 0.2,              // Scale
                        -45.0, 0.0, 1.0, 0.0);      // Rotation
    
    // Front Right Wall
    drawCubeEnvironment(3.5, 0.0, -4.0,             // Position
                        1.6, 1.0, 0.2,              // Scale
                        5.0, 0.0, 1.0, 0.0);        // Rotation

    // Back Right Wall
    drawCubeEnvironment(3.0, 0.0, -7.0,             // Position
                        1.6, 1.0, 0.2,              // Scale
                        31.0, 0.0, 1.0, 0.0);       // Rotation
    
    // Back Right Wall
    drawCubeEnvironment(3.0, 0.0, -12.0,            // Position
                        1.5, 1.0, 0.3,              // Scale
                        -15.0, 0.0, 1.0, 0.0);      // Rotation

    // Draw Pillars
    // Right Pillar
    drawCubeEnvironment(2.3, 0.0, -5.7,             // Position
                        0.15, 1.0, 0.15,            // Scale
                        45.0, 0.0, 1.0, 0.0);       // Rotation

    // Left Pillar
    drawCubeEnvironment(-0.6, 0.0, -7.0,            // Position
                        0.15, 1.0, 0.15,            // Scale
                        -30.0, 0.0, 1.0, 0.0);      // Rotation
    
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