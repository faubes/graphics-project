// ==========================================================================
// $Id: main.cpp,v 1.3 2016/03/12 03:39:35 jlang Exp $
// Viewer main routine
// Based on Revision 1.2  2016/02/06 16:52:56  jlang
// ==========================================================================
// (C)opyright:
//
//   Jochen Lang
//   SITE, University of Ottawa
//   800 King Edward Ave.
//   Ottawa, On., K1N 6N5
//   Canada.
//   http://www.site.uottawa.ca
//
// Creator: Jochen Lang
// Email:   jlang@site.uottawa.ca
// ==========================================================================
// $Log: main.cpp,v $
// Revision 1.3  2016/03/12 03:39:35  jlang
// Fixed spot light
//
// Revision 1.2  2016/03/08 03:00:08  jlang
// Added directory handling, replaced short index with int in gouraud shading, added png loading, fixed empty mtl line errors
//
// Revision 1.1  2016/03/06 22:32:50  jlang
// Starter Code and solution for assignment 3
//
// ==========================================================================


// might need these defines/include for windows
//#define GLM_ENABLE_EXPERIMENTAL
//#define _USE_MATH_DEFINES
//#include <cmath>

#ifndef _WIN32
#define JOELAPTOP
#endif

#define NOMINMAXS
#include <cstdlib>
#include <stack>
#include <iostream>
#include <GL/glew.h>
#ifdef JOELAPTOP
#include <GL/freeglut.h>
#else
#include <GL/glut.h>
#endif


// glm types
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
// matrix manipulation
#include <glm/gtc/matrix_transform.hpp>
// value_ptr
#include <glm/gtc/type_ptr.hpp>

#include "viewer.h"
#include "skybox.h"

using namespace CSI4130;
using std::cerr;
using std::endl;


namespace CSI4130 {

// Window dimensions
struct WindowSize {
        GLfloat d_near;
        GLfloat d_far;
        GLint d_widthPixel;
        GLfloat d_width;
        GLint d_heightPixel;
        GLfloat d_height;
        bool d_perspective;
        WindowSize() : // d_near(1.0f), d_far(21.0f),
                       //	d_widthPixel(512), d_width(12.5f),
                       // d_heightPixel(512), d_height(12.5f)
                // d_near(-1.0f), d_far(1.0f),
                // d_near(1.0f), d_far(3.0f),
                // d_near(3.5f), d_far(5.5f),
                d_near(4.0f), d_far(8.0f),
                // d_widthPixel(512), d_width(4.0f),
                // d_heightPixel(512), d_height(4.0f)
                d_widthPixel(512), d_width(4.0f),
                d_heightPixel(512), d_height(4.0f)
        {
        }
};

struct ControlParameter {
        bool d_spot;
        bool d_attenuation;
        GLint d_startX,d_startY;
        glm::mat4 d_rotMatrix;
        ControlParameter() : d_spot(false), d_attenuation(false),
                d_startX(0), d_startY(0) {
        }
};

/** Global variables */
WindowSize g_winSize;
ControlParameter g_control;
Viewer* g_pViewer;
Skybox* g_pSkybox;

#define NUM_CUBE_MAPS 4
std::string cubePaths[] = {"../hw_morning", "../ame_nebula", "../ame_shadow", "../darkskies"};
std::string cubeFiles[] = {"morning", "purplenebula", "shadowpeak", "darkskies"};
int cubeIndex = 0;

void init(void)
{
        glClearColor (1.0, 1.0, 1.0, 1.0);
        glEnable( GL_DEPTH_TEST );
        glDisable(GL_CULL_FACE);
        errorOut();
        // Set the distance of the light source from the camera
        g_pViewer->setLightDist( g_winSize.d_width/2.0 );

        // set the projection matrix with a uniform
        glm::mat4 Projection =
                glm::ortho( -g_winSize.d_width/2.0f, g_winSize.d_width/2.0f,
                            -g_winSize.d_height/2.0f, g_winSize.d_height/2.0f,
                            g_winSize.d_near, g_winSize.d_far );
        g_pViewer->setProjMat( Projection );
        errorOut();
}

void display(void)
{
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        // Instead of moving the coordinate system into the scene, use
        // lookAt -- use the center of the viewing volume as the reference
        // coordinates glm::mat4 ModelView =
#if 1
        glm::mat4 viewM = glm::lookAt( glm::vec3(0, 0, -(g_winSize.d_far+g_winSize.d_near)/2.0f ),
                                       // glm::vec3(-2.0f, 0.0f, 0.0f ),
                                       // glm::vec3(0.0f, 2.0f, -4.0f ),
                                       // glm::vec3(-0.1f, 0.0f, 0.0f ), // buste
                                       glm::vec3(0, 0, 0), // at is the center of the mesh
                                       glm::vec3(0, 1.0f, 0 )); // y is up
#else
        glm::mat4 viewM(1.0);
#endif
        //cerr << "Draw skybox" << endl;
        g_pSkybox->draw();
        g_pSkybox->setViewMat(viewM );
        // set the view transform
        //cerr << "Set View transform" << endl;
        g_pViewer->setViewMat( viewM );
        // set the model transform
        //cerr << "Set Model transform" << endl;
        g_pViewer->setModelMat( g_control.d_rotMatrix );
        // actual draw command
        //cerr << "Draw model" <<endl;
        g_pViewer->draw();
        // swap buffers
        glFlush();
        glutSwapBuffers();
}


/**
 * OpenGL reshape function - main window
 */
void reshape( GLsizei _width, GLsizei _height ) {
        GLfloat minDim = std::min(g_winSize.d_width,g_winSize.d_height);
        // adjust the view volume to the correct aspect ratio
        if ( _width > _height ) {
                g_winSize.d_width = minDim  * (GLfloat)_width/(GLfloat)_height;
                g_winSize.d_height = minDim;
        } else {
                g_winSize.d_width = minDim;
                g_winSize.d_height = minDim * (GLfloat)_height/(GLfloat)_width;
        }
        glm::mat4 Projection;
        if ( g_winSize.d_perspective ) {
                Projection = glm::frustum( -g_winSize.d_width/2.0f, g_winSize.d_width/2.0f,
                                           -g_winSize.d_height/2.0f, g_winSize.d_height/2.0f,
                                           g_winSize.d_near, g_winSize.d_far );
        } else {
                Projection = glm::ortho( -g_winSize.d_width/2.0f, g_winSize.d_width/2.0f,
                                         -g_winSize.d_height/2.0f, g_winSize.d_height/2.0f,
                                         g_winSize.d_near, g_winSize.d_far );
        }
        g_pViewer->setProjMat(Projection);
        g_winSize.d_widthPixel = _width;
        g_winSize.d_heightPixel = _height;
        // reshape our viewport
        glViewport( 0, 0,
                    g_winSize.d_widthPixel,
                    g_winSize.d_heightPixel );
}



/**
 * Rotates the model around x and y axis
 */
void rotateModel( int _x, int _y ) {
        // static GLfloat angleX = 0.0f, angleY = 0.0f;
        float angleX = static_cast<GLfloat>(_x - g_control.d_startX)/50.0f;
        float angleY = -static_cast<GLfloat>(_y - g_control.d_startY)/50.0f;
        // cerr << "Angle X: " << angleX << endl;
        // cerr << "Angle Y: " << angleY << endl;
        // Rotation matrix assembly
        // rotate around current y and x
        glm::vec3 rX = glm::vec3( g_control.d_rotMatrix[0].y,
                                  g_control.d_rotMatrix[1].y,
                                  g_control.d_rotMatrix[2].y);
        glm::vec3 rY = glm::vec3( g_control.d_rotMatrix[0].x,
                                  g_control.d_rotMatrix[1].x,
                                  g_control.d_rotMatrix[2].x);
        g_control.d_rotMatrix =
                glm::rotate(g_control.d_rotMatrix, angleY, rY );
        g_control.d_rotMatrix =
                glm::rotate(g_control.d_rotMatrix, angleX, rX);
        g_control.d_startX = _x;
        g_control.d_startY = _y;
        glutPostRedisplay();
}

/**
 * Mouse function callback - called on a mouse event
 */
void trackball( int _button, int _state, int _x, int _y ) {
        if ( _button == GLUT_LEFT_BUTTON && _state == GLUT_DOWN ) {
                g_control.d_startX = _x;
                g_control.d_startY = _y;
                glutMotionFunc(rotateModel);
        }
        if ( _button == GLUT_LEFT_BUTTON && _state == GLUT_UP ) {
                glutMotionFunc(NULL);
        }
        glutPostRedisplay();
}

void keyboard (unsigned char key, int x, int y)
{
        LightSource light;

        switch (key) {
        case 27:
        case 'q':
                exit(0);
                break;
        case '+':
                g_pViewer->incLAngle(0.1f);
                break;
        case '-':
                g_pViewer->incLAngle(-0.1f);
                break;
        case 'P':
                // switch to perspective
                g_winSize.d_perspective = true;
                reshape( g_winSize.d_widthPixel, g_winSize.d_heightPixel );
                break;
        case 'p':
                // switch to perspective
                g_winSize.d_perspective = false;
                reshape( g_winSize.d_widthPixel, g_winSize.d_heightPixel );
                break;
        case 'Z':
                // increase near plane
                g_winSize.d_near += 0.1f;
                g_winSize.d_far += 0.1f;
                reshape( g_winSize.d_widthPixel, g_winSize.d_heightPixel );
                break;
        case 'z':
                // decrease near plane
                if ( g_winSize.d_near > 0.1f ) {
                        g_winSize.d_near -= 0.1f;
                        g_winSize.d_far -= 0.1f;
                }
                reshape( g_winSize.d_widthPixel, g_winSize.d_heightPixel );
                break;
        case 'L':
                // light on
                break;
        case 'l':
                // light off
                break;
        case 'w':
                delete g_pViewer;
                g_pViewer = new Viewer("box.obj");
                g_pViewer->setProgMaterial();
                init();
                break;
        case 'W':
                delete g_pViewer;
                g_pViewer = new Viewer("tiger.obj");
                g_pViewer->setProgMaterial();
                init();
                break;
        case 'r':
                cubeIndex++;
                cubeIndex %= NUM_CUBE_MAPS;
                delete g_pSkybox;
                g_pSkybox = new Skybox(cubePaths[cubeIndex], cubeFiles[cubeIndex]);
                init();
                break;
        case 'R':
                delete g_pSkybox;
                g_pSkybox = new Skybox(std::string("../hw_morning"), std::string("morning"));
                init();
                break;
        case 'c':
                // colors on
                g_pViewer->setProgColor();
                break;
        case 'e':
                // textures off
                g_pViewer->setProgElement();
                break;
        case 'x':
                // textures on
                g_pViewer->setProgTexture();
                break;
        case 'm':
                // material rendering on
                g_pViewer->setProgMaterial();
                break;
        case 'A':
                // ambient term on
                light = g_pViewer->getLight();
                light.d_ambient = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f);
                g_pViewer->setLight( light );
                break;
        case 'a':
                // ambient term off
                light = g_pViewer->getLight();
                light.d_ambient = glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f);
                g_pViewer->setLight( light );
                break;
        case 'D':
                // directional light
                light = g_pViewer->getLight();
                light.d_spot_cutoff = 180.0f; // No spot light
                light.d_pointLight = false;
                g_pViewer->setLight( light );
                break;
        case 'd':
                // point light
                light = g_pViewer->getLight();
                light.d_pointLight = true;
                g_pViewer->setLight( light );
                break;
        // spot light on/off
        case 'S':
                light = g_pViewer->getLight();
                light.d_pointLight = true; // No directional lighting
                light.d_spot_cutoff = 90.0f;
                g_control.d_spot = true;
                g_control.d_attenuation = false;
                g_pViewer->setLight( light );
                cerr << "Spot light: " <<  light.d_spot_exponent
                     << " " << light.d_spot_cutoff << endl;
                break;
        case 's':
                light = g_pViewer->getLight();
                light.d_spot_cutoff = 180.0f;
                g_control.d_spot = false;
                g_pViewer->setLight( light );
                cerr << "Spot light: " <<  light.d_spot_exponent
                     << " " << light.d_spot_cutoff << endl;
                break;
        case 'T':
                // attenuation on/off
                light = g_pViewer->getLight();
                light.d_constant_attenuation = 1.0f;
                light.d_linear_attenuation = 0.001f;
                light.d_quadratic_attenuation = 0.0005f;
                g_control.d_attenuation = true;
                g_control.d_spot = false;
                g_pViewer->setLight( light );
                cerr << "Attenuation: " <<  light.d_constant_attenuation
                     << " " <<light.d_linear_attenuation
                     << " " << light.d_quadratic_attenuation << endl;
                break;
        case 't':
                light = g_pViewer->getLight();
                light.d_constant_attenuation = 1.0f;
                light.d_linear_attenuation = 0.0f;
                light.d_quadratic_attenuation = 0.0f;
                g_control.d_attenuation = false;
                g_pViewer->setLight( light );
                cerr << "Attenuation: " <<  light.d_constant_attenuation
                     << " " << light.d_linear_attenuation
                     << " " << light.d_quadratic_attenuation << endl;
                break;
        default:
                break;
        }
        glutPostRedisplay();
}

void specialkeys( int key, int x, int y )
{
        LightSource light;

        switch (key) {
        case GLUT_KEY_LEFT:
                light = g_pViewer->getLight();
                if (g_control.d_spot) {
                        light.d_spot_exponent = std::max(light.d_spot_exponent-0.1f, 0.0f);
                        cerr << "Spot light: " <<  light.d_spot_exponent
                             << " " << light.d_spot_cutoff << endl;
                }
                if (g_control.d_attenuation) {
                        light.d_quadratic_attenuation =
                                std::max(light.d_quadratic_attenuation-0.0005f, 0.0f);
                        cerr << "Attenuation: " <<  light.d_constant_attenuation
                             << " " << light.d_linear_attenuation
                             << " " << light.d_quadratic_attenuation << endl;
                }
                g_pViewer->setLight( light );
                break;
        case GLUT_KEY_RIGHT:
                light = g_pViewer->getLight();
                if (g_control.d_spot) {
                        light.d_spot_exponent =
                                std::min( light.d_spot_exponent+0.1f, 128.0f);
                        cerr << "Spot light: " <<  light.d_spot_exponent
                             << " " << light.d_spot_cutoff << endl;
                }
                if (g_control.d_attenuation) {
                        light.d_quadratic_attenuation += 0.0005f;
                        cerr << "Attenuation: " <<  light.d_constant_attenuation
                             << " " << light.d_linear_attenuation
                             << " " << light.d_quadratic_attenuation << endl;
                }
                g_pViewer->setLight( light );
                break;
        case GLUT_KEY_UP:
                light = g_pViewer->getLight();
                if (g_control.d_spot) {
                        light.d_spot_cutoff = std::min(light.d_spot_cutoff+1.0f, 90.0f);
                        cerr << "Spot light: " <<  light.d_spot_exponent
                             << " " << light.d_spot_cutoff << endl;
                }
                if (g_control.d_attenuation) {
                        light.d_linear_attenuation = light.d_linear_attenuation+0.001f;
                        cerr << "Attenuation: " <<  light.d_constant_attenuation
                             << " " << light.d_linear_attenuation
                             << " " << light.d_quadratic_attenuation << endl;
                }
                g_pViewer->setLight( light );
                break;
        case GLUT_KEY_DOWN:
                light = g_pViewer->getLight();
                if (g_control.d_spot) {
                        light.d_spot_cutoff = std::max(light.d_spot_cutoff-1.0f, 0.0f);
                        cerr << "Spot light: " <<  light.d_spot_exponent
                             << " " << light.d_spot_cutoff << endl;
                }
                if (g_control.d_attenuation) {
                        light.d_linear_attenuation =
                                std::max(light.d_linear_attenuation-0.001f, 0.0f);
                        cerr << "Attenuation: " <<  light.d_constant_attenuation
                             << " " << light.d_linear_attenuation
                             << " " << light.d_quadratic_attenuation << endl;
                }
                g_pViewer->setLight( light );
                break;
        case GLUT_KEY_PAGE_UP:
                g_winSize.d_height += 0.2f;
                g_winSize.d_width += 0.2f;
                reshape( g_winSize.d_widthPixel, g_winSize.d_heightPixel );
                break;
        case GLUT_KEY_PAGE_DOWN:
                g_winSize.d_height -= 0.2f;
                g_winSize.d_width -= 0.2f;
                reshape( g_winSize.d_widthPixel, g_winSize.d_heightPixel );
                break;
        default: break;
        }
        glutPostRedisplay();
}

}

int main(int argc, char** argv) {
  #ifdef JOELAPTOP
// annoying version stuff
        glewExperimental = GL_TRUE;
        cerr << "Set GLEW Experimental Version" <<endl;
        errorOut();
        cerr << "Specity Context Version: 4,5"<<endl;
        glutInitContextVersion(4,5);
        errorOut();
  #endif

        std::string fileName = "box.obj";
        glutInit(&argc, argv);
        glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
        glutInitWindowSize (512, 512);
        glutInitWindowPosition (0, 0);
        glutCreateWindow (argv[0]);
        GLenum err = glewInit();
        if (GLEW_OK != err) {
                /* Problem: glewInit failed, something is seriously wrong. */
                cerr << "Error: " << glewGetErrorString(err) << endl;
                return -1;
        }
        cerr << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;
        // Make sure that our shaders run
        int major, minor;
        getGlVersion( major, minor );
        cerr << "Running OpenGL "<< major << "." << minor << endl;
        if ( major < 3 || (major==3 && minor<3)) {
                cerr << "No OpenGL 3.3 or higher" <<endl;
                exit(-1);
        }
        if ( argc > 1 ) fileName = argv[1];
        std::string skyboxPath = "../hw_morning";
        std::string skyboxFileName = "morning";
        g_pViewer = new Viewer(fileName);
        g_pViewer->setProgMaterial();
        //g_pViewer->setProgColor();
        g_pSkybox = new Skybox(skyboxPath, skyboxFileName);
        //g_pViewer->setProgTexture();
        init();
        glutReshapeFunc(reshape);
        glutDisplayFunc(display);
        glutSpecialFunc(specialkeys);
        glutMouseFunc(trackball);
        glutKeyboardFunc(keyboard);
        glutMainLoop();
        return 0;
}
