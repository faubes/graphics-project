// ==========================================================================
// $Id: viewer.h,v 1.3 2018/03/11 03:24:27 jlang Exp $
// Viewer class encapsulating state changes for rendering
// Based on Revision 1.2  2016/02/06 16:52:56  jlang
// ==========================================================================
// (C)opyright:
//
//   Jochen Lang
//   EECS, University of Ottawa
//   800 King Edward Ave.
//   Ottawa, On., K1N 6N5
//   Canada.
//   http://www.eecs.uottawa.ca
//
// Creator: Jochen Lang
// Email:   jlang@eecs.uottawa.ca
// ==========================================================================
// $Log: viewer.h,v $
// Revision 1.3  2018/03/11 03:24:27  jlang
// Update for project 2018
//
// Revision 1.2  2016/03/08 03:00:09  jlang
// Added directory handling, replaced short index with int in gouraud shading, added png loading, fixed empty mtl line errors
//
// Revision 1.1  2016/03/06 22:32:51  jlang
// Starter Code and solution for assignment 3
//
// ==========================================================================
#ifndef CSI4130_VIEWER_H
#define CSI4130_VIEWER_H

#define NOMINMAXS
// #include <cstdlib>
#include <iostream>
#include <algorithm>

#include <GL/glew.h>
#include <GL/glut.h>

// glm types
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
// matrix manipulation
#include <glm/gtc/matrix_transform.hpp>
// value_ptr
#include <glm/gtc/type_ptr.hpp>
// to_string
#include <glm/gtx/string_cast.hpp>

#include "shader.h"
#include "light.h"
#include "material.h"
#include "texture.h"

using std::cerr;
using std::endl;

// #define DEBUG_VIEWER_VERBOSE
// #define DEBUG_VIEWER


namespace CSI4130 {

#ifdef DEBUG_VIEWER //_VERBOSE
template<typename T>
void printBuffer( const void* buffer, int sz, int stride=3 ) {
        const T* ptr = reinterpret_cast<const T*>(buffer);
        for ( int i=0; i<sz; ++i ) {
                cerr << *ptr << " ";
                if ( (i+1)%stride==0 ) cerr << endl;
                ++ptr;
        }
        return;
}
#endif


/*
 * Helper structure holding the transformation matrices and the
 * locations of the uniforms of transform matrices
 */
struct Transformations {
        GLint locP;
        GLint locVM;
        GLint locMM;
        glm::mat4 d_ProjM;
        glm::mat4 d_ViewM;
        glm::mat4 d_ModelM;

        Transformations() : locP(-1), locVM(-1), locMM(-1)
        {
        }

        /**
         *  Sets the uniform transform locations for the current program
         */
        void updateLocation( GLuint program ) {
                // transform uniforms and attributes
                locP = glGetUniformLocation( program, "ProjectionMatrix");
                locVM = glGetUniformLocation( program, "ViewMatrix");
                locMM = glGetUniformLocation( program, "ModelMatrix");

                errorOut();
        }

        void setUniforms() {
                glUniformMatrix4fv(locP, 1, GL_FALSE, glm::value_ptr(d_ProjM));
                errorOut();

                glUniformMatrix4fv(locVM, 1, GL_FALSE, glm::value_ptr(d_ViewM));
                errorOut();

                glUniformMatrix4fv(locMM, 1, GL_FALSE, glm::value_ptr(d_ModelM));
                errorOut();

#ifdef DEBUG_VIEWER_VERBOSE
                cerr << "Uniform locations: " << locP << " " << locVM << " " << locMM << endl;
                cerr << "Projection matrix: " << endl;
                printBuffer<float>( glm::value_ptr(d_ProjM), 16, 4 );
                cerr << "View matrix: " << endl;
                printBuffer<float>( glm::value_ptr(d_ViewM), 16, 4 );
                cerr << "Model matrix: " << endl;
                printBuffer<float>( glm::value_ptr(d_ModelM), 16, 4 );
#endif
                errorOut();
                return;
        }
};

struct Attributes {
        GLint locPos;
        GLint locNorm;
        GLint locColor;
        GLint locTexture;
        GLint locMatIndex;
        Attributes() : locPos(-1), locNorm(-1), locColor(-1),
                locTexture(-1), locMatIndex(-1) {
        }

        void updateLocation( GLuint program ) {
                locPos = glGetAttribLocation(program, "position");
                locNorm = glGetAttribLocation(program, "normal");
                locColor = glGetAttribLocation(program, "color");
                locTexture = glGetAttribLocation(program, "texCoord");
                locMatIndex = glGetAttribLocation(program, "matIndex");
#ifdef DEBUG_VIEWER_VERBOSE
                cerr << "Attrib location update: ";
                cerr << locPos << " " << locNorm << " " << locColor << " " << locTexture << " " << locMatIndex << endl;
#endif
                return;
        }

        void disableAll() {
                if ( locPos != -1 ) glDisableVertexAttribArray(locPos);
                if ( locNorm != -1 ) glDisableVertexAttribArray(locNorm);
                if ( locColor != -1 ) glDisableVertexAttribArray(locColor);
                if ( locTexture != -1 ) glDisableVertexAttribArray(locTexture);
                if ( locMatIndex != -1 ) glDisableVertexAttribArray(locMatIndex);
        }

        void enableVAPos() {
#ifdef DEBUG_VIEWER_VERBOSE
                cerr << "Enabling vbo at " << locPos << endl;
#endif
                glVertexAttribPointer(locPos, 3, GL_FLOAT, GL_FALSE, 0, 0 );
                glEnableVertexAttribArray(locPos);
        }

        void enableVANorm() {
#ifdef DEBUG_VIEWER_VERBOSE
                cerr << "Enabling nbo at " << locNorm << endl;
#endif
                glVertexAttribPointer(locNorm, 3, GL_FLOAT, GL_FALSE, 0, 0 );
                glEnableVertexAttribArray(locNorm);
        }

        void enableVAColor() {
#ifdef DEBUG_VIEWER_VERBOSE
                cerr << "Enabling cbo at " << locColor << endl;
#endif
                glVertexAttribPointer(locColor, 4, GL_FLOAT, GL_FALSE, 0, 0 );
                glEnableVertexAttribArray(locColor);
        }

        void enableVATexture() {
#ifdef DEBUG_VIEWER_VERBOSE
                cerr << "Enabling tbo at " << locTexture << endl;
#endif
                glVertexAttribPointer(locTexture, 2, GL_FLOAT, GL_FALSE, 0, 0 );
                glEnableVertexAttribArray(locTexture);
        }

        void enableVAMat() {
       #ifdef DEBUG_VIEWER_VERBOSE
                cerr << "Enabling matIndex at " << locMatIndex << endl;
       #endif
                glVertexAttribPointer(locMatIndex, 1, GL_FLOAT, GL_FALSE, 0, 0 );
                glEnableVertexAttribArray(locMatIndex);
                errorOut();
        }

};

struct LightView {
        // Light related variables
        bool d_lightOn;
        int d_cLight;
        LightArray d_lightArray;
        GLfloat d_angle;
        GLfloat d_dist;

        LightView( int _nLights=1 ) : d_lightOn(false), d_cLight(0), d_angle(0.0f), d_dist(1.0f)
        {
                for ( int i=0; i<_nLights; ++i ) {
                        d_lightArray.append( LightSource() );
                }
        }
};


class Viewer {
// Programs and buffers
GLuint d_program;     // Currently active program
GLuint d_progElement;     // Element array rendering
GLuint d_progColor;     // direct rendering without elements and a color buffer
GLuint d_progMaterial;     // program that uses materials
GLuint d_progTexture;

// Element rendering
GLuint d_vao_ele;     // Vertex array object
GLuint d_vbo_ele;     // Vertex buffer object
GLuint d_ebo;     // element buffer object
GLuint d_nbo_ele;     // normal buffer object
GLuint d_cbo_ele;     // Vertex buffer object
GLuint d_nIndices;
// Rendering with vertex indices unrolled
GLuint d_vao;     // Vertex array object
GLuint d_vbo;     // Vertex buffer object

GLuint d_nbo;     // normal buffer object
GLuint d_cbo;     // color buffer object
GLuint d_mbo;     // material id buffer object
GLuint d_tbo;     // texture buffer object
GLuint d_ubo;     // uniform buffer object for material
GLuint d_nVert;


// Transform to scale and translate the model to the cannonical viewing volume
glm::mat4 d_unit;

// Uniform locations - need to change with currently active program
Transformations d_tfm;
Attributes d_attrib;

LightView d_light;

MaterialArray d_mArray;
std::vector<Texture> d_texInfo;

bool d_withTexture;




public:
Viewer( std::string _meshFile, int _nLights = 1 );
// switch between programs
bool setProgElement();
bool setProgColor();
bool setProgMaterial();
bool setProgTexture();

inline void draw();

// public light interface
inline void setCurrentLight( int _cLight );
inline int getCurrentLight();
inline float incLAngle( float _angle );
inline void setLightDist( GLfloat _dist );
// Get and set the current light
inline LightSource getLight();
inline void setLight( const LightSource& _light );

// public transform interface
inline void setProjMat( const glm::mat4& _projM );
inline void setViewMat( const glm::mat4& _viewM );
inline void setModelMat( const glm::mat4& _modelM );


protected:
bool switchProgram( GLuint program );
void setArrayVAO();
void setMaterialBuffer();
GLuint loadShader( std::string _basename );
void setLightPosition( GLuint program, GLfloat dist );
void generateBuffer( GLuint& _bo, int _sz, const void* _ptr=0 );
};


void Viewer::draw() {
        setArrayVAO();
        glUseProgram(d_program);

        d_tfm.setUniforms();
        //cerr << "Set lights" << endl;
        setLightPosition(d_program,d_light.d_dist);
        if ( d_program == d_progElement ) {
#ifdef DEBUG_VIEWER_VERBOSE
                cerr << "Element drawing " << 3*d_nIndices << " indices." << endl;
#endif
                glDrawElements(GL_TRIANGLES, 3*d_nIndices, GL_UNSIGNED_INT, 0 );
                errorOut();
                return;
        }
        if ( d_program == d_progColor ) {
#ifdef DEBUG_VIEWER_VERBOSE
                cerr << "Color drawing " << d_nVert/3 << " triangles." << endl;
#endif
                glDrawArrays(GL_TRIANGLES, 0, d_nVert );
                errorOut();
                return;
        }
        if ( d_program == d_progMaterial ) {
#ifdef DEBUG_VIEWER_VERBOSE
                cerr << "Material drawing " << d_nVert/3 << " triangles." << endl;
#endif
                glDrawArrays(GL_TRIANGLES, 0, d_nVert );
                errorOut();
                return;
        }
        if ( d_program == d_progTexture ) {
#ifdef DEBUG_VIEWER_VERBOSE
                cerr << "Texture drawing " << d_nVert/3 << " triangles." << endl;
#endif
                glDrawArrays(GL_TRIANGLES, 0, d_nVert );
                errorOut();
                return;
        }
        cerr << "Draw error: Unknown program" << endl;
        return;
}

/**
 * Public light interface
 */
void Viewer::setCurrentLight( int _cLight ) {
        d_light.d_cLight = std::min( _cLight, static_cast<int>(d_light.d_lightArray.size()-1));
        return;
}

int Viewer::getCurrentLight() {
        return d_light.d_cLight;
}

float Viewer::incLAngle( float _angle ) {
        d_light.d_angle += _angle;
        return d_light.d_angle;
}

void Viewer::setLightDist( GLfloat _dist ) {
        d_light.d_dist = _dist;
}

inline LightSource Viewer::getLight() {
        return d_light.d_lightArray.get( d_light.d_cLight );
}

inline void Viewer::setLight( const LightSource& _light ) {
        d_light.d_lightArray.set( d_light.d_cLight, _light );
        if ( d_program != -1 ) {
                d_light.d_lightArray.setLight(d_program, d_light.d_cLight );
        }
        return;
}

/**
 *  Public transform interface
 */
inline void Viewer::setProjMat( const glm::mat4& _projM ) {
        d_tfm.d_ProjM = _projM;
}

inline void Viewer::setViewMat( const glm::mat4& _viewM ) {
        d_tfm.d_ViewM = _viewM;
}

inline void Viewer::setModelMat( const glm::mat4& _modelM ) {
        d_tfm.d_ModelM = _modelM * d_unit;
}



}

#endif
