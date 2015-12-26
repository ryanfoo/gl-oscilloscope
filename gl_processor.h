/*
 * ==================================================================================
 *
 *      Filename:   gl_processor.h
 *
 *   Description:   Open GL Processor Header File
 *                  Creates Waveform
 *       Version:   1.0
 *       Created:   12/24/2015
 *
 *        Author:   Ryan Foo (ryanfoo@nyu.edu)
 *       Website:   https://github.com/ryanfoo
 *
 * ==================================================================================
 */

#ifndef GL_PROCESSOR_H
#define GL_PROCESSOR_H

// Open GL
// #ifdef __MACOSX_CORE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>

// GL Definitions
#define INIT_WIDTH              900             // GL View Width
#define INIT_HEIGHT             700             // GL View Height

// Width/Height of GL window
GLsizei g_width         = INIT_WIDTH;
GLsizei g_height        = INIT_HEIGHT;
GLsizei g_last_width    = INIT_WIDTH;
GLsizei g_last_height   = INIT_HEIGHT;

// GL global variables
GLint g_buffer_size     = BUFFER_SIZE;
float g_buffer[BUFFER_SIZE];
float g_window[BUFFER_SIZE];
unsigned int g_channels = STEREO;

// Threads Management
GLboolean g_ready = false;
// Fill Mode
GLenum g_fillmode = GL_FILL;
// Light 0 Position
GLfloat g_light0_pos[4] = {2.0f, 1.2f, 4.0f, 1.0f};
// Light 1 Parameters
GLfloat g_light1_ambient[]  = {.2f, .2f, .2f, 1.0f};
GLfloat g_light1_diffuse[]  = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat g_light1_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat g_light1_pos[4]     = {-2.0f, 0.0f, -4.0f, 1.0f};

// Fullscreen
GLboolean g_fullscreen = false;
// Modelview stuff
GLfloat g_linewidth = 2.0f;

/* 
 *  Name: void drawWindowedTimeDomain(float *buffer)
 *  Desc: Draws the Windowed Time Domain signal in the top of the screen
 */
void drawWindowedTimeDomain(float *buffer) {
    // Initialize initial x
    GLfloat x = -5;

    // Calculate increment x
    GLfloat xinc = fabs((2*x)/g_buffer_size);

    glPushMatrix();
    {
        // Blue Color
        glColor3f(0, 0, 1.0);

        glBegin(GL_LINE_STRIP);

        // Draw Windowed Time Domain
        for (int i = 0;  i < g_buffer_size; i++)
        {
            glVertex3f(x, 4*buffer[i], 0.0f);
            x += xinc;
        }

        glEnd();

    }
    glPopMatrix();
}

/*
 *  Name: idleFunc()
 *  Desc: callback from GLUT
 */
void idleFunc()
{
    // render the scene
    glutPostRedisplay();
}

/*
 *  Name: reshapeFunc(int w, int h)
 *  Desc: called when window size changes
 */
void reshapeFunc(int w, int h)
{
    // save the new window size
    g_width = w; g_height = h;
    // map the view port to the client area
    glViewport(0, 0, w, h);
    // set the matrix mode to project
    glMatrixMode(GL_PROJECTION);
    // load the identity matrix
    glLoadIdentity();
    // create the viewing frustum
    //gluPerspective( 45.0, (GLfloat) w / (GLfloat) h, .05, 50.0 );
    gluPerspective(45.0, (GLfloat) w / (GLfloat) h, 1.0, 1000.0);
    // set the matrix mode to modelview
    glMatrixMode(GL_MODELVIEW);
    // load the identity matrix
    glLoadIdentity();

    gluLookAt(0.0f, 0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
}

/*
 *  Name: displayFunc()
 *  Desc: callback function invoked to draw the client area
 */
void displayFunc()
{
    // local variables
    float buffer[g_buffer_size];

    // wait for data
    while (!g_ready) usleep(1000);

    // copy currently playing audio into buffer
    memcpy(buffer, g_buffer, g_buffer_size * sizeof(float));

    // Hand off to audio callback thread
    g_ready = false;

    // clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Windowed Time Domain
    drawWindowedTimeDomain(buffer);

    // flush gl commands
    glFlush();

    // swap the buffers
    glutSwapBuffers();
}

/*
 *  Name: specialUpKey(int key, int x, int y)
 *  Desc: Callback to know when a special key is pressed
 */
void specialKey(int key, int x, int y) { 
    // Check which (arrow) key is pressed
    switch (key) {
        case GLUT_KEY_LEFT : // Arrow key left is pressed
            break;
        case GLUT_KEY_RIGHT :    // Arrow key right is pressed
            break;
        case GLUT_KEY_UP :        // Arrow key up is pressed
            break;
        case GLUT_KEY_DOWN :    // Arrow key down is pressed
            break;   
    }
}  

/*
 *  Name: specialUpKey(int key, int x, int y)
 *  Desc: Callback to know when a special key is up
 */
void specialUpKey(int key, int x, int y) {
    // Check which (arrow) key is unpressed
    switch (key) {
        case GLUT_KEY_LEFT : // Arrow key left is unpressed
            break;
        case GLUT_KEY_RIGHT :    // Arrow key right is unpressed
            break;
        case GLUT_KEY_UP :        // Arrow key up is unpressed
            break;
        case GLUT_KEY_DOWN :    // Arrow key down is unpressed
            break;   
    }
}

/* 
 *  Name: initialize_graphics( )
 *  Desc: sets initial OpenGL states and initializes any application data
 */
void initialize_graphics()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);                 // Black Background
    // set the shading model to 'smooth'
    glShadeModel(GL_SMOOTH);
    // enable depth
    glEnable(GL_DEPTH_TEST);
    // set the front faces of polygons
    glFrontFace(GL_CCW);
    // set fill mode
    glPolygonMode(GL_FRONT_AND_BACK, g_fillmode);
    // enable lighting
    glEnable(GL_LIGHTING);
    // enable lighting for front
    glLightModeli(GL_FRONT_AND_BACK, GL_TRUE);
    // material have diffuse and ambient lighting 
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    // enable color
    glEnable(GL_COLOR_MATERIAL);
    // normalize (for scaling)
    glEnable(GL_NORMALIZE);
    // line width
    glLineWidth(g_linewidth);

    // enable light 0
    glEnable(GL_LIGHT0);

    // setup and enable light 1
    glLightfv(GL_LIGHT1, GL_AMBIENT, g_light1_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, g_light1_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, g_light1_specular);
    glEnable(GL_LIGHT1);
}

/* 
 *  Name: initialize_glut(int argc, char *argv[])
 *  Desc: Initializes Glut with the global vars
 */
void initialize_glut(int argc, char *argv[]) {
    // initialize GLUT
    glutInit(&argc, argv);
    // double buffer, use rgb color, enable depth buffer
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    // initialize the window size
    glutInitWindowSize(g_width, g_height);
    // set the window postion
    glutInitWindowPosition(400, 100);
    // create the window
    glutCreateWindow("foo oscilloscope ");
    // full screen
    if (g_fullscreen) glutFullScreen();

    // set the idle function - called when idle
    glutIdleFunc(idleFunc);
    // set the display function - called when redrawing
    glutDisplayFunc(displayFunc);
    // set the reshape function - called when client area changes
    glutReshapeFunc(reshapeFunc);
    // set window's to specialKey callback
    glutSpecialFunc(specialKey);
    // set window's to specialUpKey callback (when the key is up is called)
    glutSpecialUpFunc(specialUpKey);

    // do our own initialization
    initialize_graphics();
}

#endif  // GL_PROCESSOR_H
