#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

// camera parameters: controls the view of the scene
float cam_radius = 5.0f;
float cam_theta = 45.0f;
float cam_phi = 45.0f;

// speed of the automatic rotation for the octahedron
float auto_rotation_speed = 0.008f;

// openGL objects for the octahedron
GLuint VBO, VAO, EBO;

// spotlight data (position, color, direction)
struct Spotlight {
    GLfloat position[4];
    GLfloat color[4];
    GLfloat direction[3];
};

Spotlight spotlights[] = {
    {{2.0f, 2.0f, 2.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},
    {{-2.0f, 3.0f, -2.0f, 1.0f}, {0.5f, 0.0f, 0.5f, 1.0f}, {1.0f, -1.0f, 1.0f}},
    {{0.0f, -3.0f, 3.0f, 1.0f}, {0.0f, 0.5f, 1.0f, 1.0f}, {0.0f, 1.0f, -1.0f}}
};

// octahedron vertices and indices for rendering
GLfloat vertices[] = {
    0.0f,  1.0f,  0.0f,  // top
    0.0f, -1.0f,  0.0f,  // bottom
    1.0f,  0.0f,  0.0f,  // right
   -1.0f,  0.0f,  0.0f,  // left
    0.0f,  0.0f,  1.0f,  // front
    0.0f,  0.0f, -1.0f   // back
};

GLuint indices[] = {
    0, 2, 4,  0, 4, 3,  0, 3, 5,  0, 5, 2,  // top triangles
    1, 4, 2,  1, 3, 4,  1, 5, 3,  1, 2, 5   // bottom triangles
};

GLuint edges[] = {
    0, 2, 0, 4, 0, 3, 0, 5,  // top edges
    1, 2, 1, 4, 1, 3, 1, 5,  // bottom edges
    2, 4, 4, 3, 3, 5, 5, 2   // middle edges
};

// rotation angles for the octahedron
float rot_x = 0.0f, rot_y = 0.0f, rot_z = 0.0f;
float rotation_delta = 0.1f;

// update camera position based on spherical coordinates
void updateCamera() {
    float x = cam_radius * sinf(cam_theta * M_PI / 180.0f) * cosf(cam_phi * M_PI / 180.0f);
    float y = cam_radius * cosf(cam_theta * M_PI / 180.0f);
    float z = cam_radius * sinf(cam_theta * M_PI / 180.0f) * sinf(cam_phi * M_PI / 180.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(x, y, z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
}

// draw edges of the octahedron, to help visualize the wireframe
void drawEdges() {
    glDisable(GL_LIGHTING);
    glColor3f(0.8f, 0.8f, 1.0f);  // wireframe color

    glBegin(GL_LINES);
    for (int i = 0; i < 24; i += 2) {
        glVertex3fv(&vertices[edges[i] * 3]);
        glVertex3fv(&vertices[edges[i + 1] * 3]);
    }
    glEnd();

    glEnable(GL_LIGHTING);
}

// initialize the VBO, VAO, and EBO for the octahedron
void initBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// initialize the lighting with spotlights
void initLighting() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    // ambient light
    GLfloat ambientLight[] = { 0.1f, 0.1f, 0.2f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);

    // set up spotlights
    for (int i = 0; i < 3; i++) {
        glEnable(GL_LIGHT0 + i);
        glLightfv(GL_LIGHT0 + i, GL_POSITION, spotlights[i].position);
        glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, spotlights[i].color);
        glLightfv(GL_LIGHT0 + i, GL_SPOT_DIRECTION, spotlights[i].direction);
        glLightf(GL_LIGHT0 + i, GL_SPOT_CUTOFF, 30.0f);
        glLightf(GL_LIGHT0 + i, GL_SPOT_EXPONENT, 10.0f);
    }
}

// setup the perspective projection matrix
void initProjection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 1.0, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

// draw the octahedron with its materials and rotation
void drawOctahedron() {
    // material properties
    GLfloat matAmbient[] = { 0.3f, 0.3f, 0.4f, 1.0f };
    GLfloat matDiffuse[] = { 0.6f, 0.4f, 0.8f, 1.0f };
    GLfloat matSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat matShininess[] = { 100.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);

    // apply rotation to the octahedron
    glPushMatrix();
    glRotatef(rot_x, 1.0f, 0.0f, 0.0f); // rotate around X axis
    glRotatef(rot_y, 0.0f, 1.0f, 0.0f); // rotate around Y axis
    glRotatef(rot_z, 0.0f, 0.0f, 1.0f); // rotate around Z axis

    // draw the octahedron faces (triangles)
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // draw edges for a wireframe effect
    drawEdges();

    glPopMatrix();
}

// automatically rotate the octahedron when idle
void idle() {
    rot_x += auto_rotation_speed;
    rot_y += auto_rotation_speed;
    rot_z += auto_rotation_speed;

    glutPostRedisplay();
}

// main display function
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    updateCamera();
    drawOctahedron();
    glutSwapBuffers();
}

// handle keyboard input for camera control
void keyboard(unsigned char key, int x, int y) {
    float delta = 5.0f;
    switch (key) {
        case 'w': cam_theta -= delta; break;
        case 's': cam_theta += delta; break;
        case 'a': cam_phi -= delta; break;
        case 'd': cam_phi += delta; break;
        case 27: exit(0); break; // escape key to quit
    }
    if (cam_theta < 0) cam_theta = 0;
    if (cam_theta > 180) cam_theta = 180;
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    srand(time(0)); // seed the random number generator for rotations

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Etherium-inspired Octahedron");

    if (glewInit() != GLEW_OK) {
        std::cerr << "error initializing glew" << std::endl;
        return -1;
    }

    initBuffers();
    initLighting();
    initProjection();

    glutDisplayFunc(display);
    glutIdleFunc(idle); // automatic rotation
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return 0;
}
