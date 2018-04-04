#ifndef CSI4130_SKYBOX_H
#define CSI4130_SKYBOX_H

//#include <string>
//#include <vector>
//#include <map>
//#include <algorithm>

// openGL types
#include <GL/glew.h>
#include <GL/glut.h>

// glm types
#include <glm/glm.hpp>
#include "viewer.h"
#include "shader.h"
//#include "light.h"
//#include "material.h"
//#include "texture.h"

using namespace std;

namespace CSI4130 {


struct Skybox {
        static float skyboxVertices[3*36];
        GLuint d_progSkybox; // program that uses materials and textures
        GLuint d_skybox_vao; // VAO for skybox
        GLuint d_skybox_vbo;
        // stores name of texture
        GLuint d_cubeMapTexture;
        // stores transformations
        Transformations d_tfm;

        void draw();
// constructor
        Skybox(const std::string & path, const std::string & fileName, const std::string & ext, int size);
// cubemap loader
        GLuint bindCubeMapTexture(const std::string &path, const std::string &fileName, const std::string &ext, int size );
// copied from Viewer
        GLuint loadShader( std::string _basename );

        // public transform interface
        inline void setProjMat( const glm::mat4& _projM );
        inline void setViewMat( const glm::mat4& _viewM );
        inline void setModelMat( const glm::mat4& _modelM );
};
        /**
         *  Public transform interface
         */
        inline void Skybox::setProjMat( const glm::mat4& _projM ) {
                d_tfm.d_ProjM = _projM;
        }

        inline void Skybox::setViewMat( const glm::mat4& _viewM ) {
                d_tfm.d_ViewM = _viewM;
        }

        inline void Skybox::setModelMat( const glm::mat4& _modelM ) {
                d_tfm.d_ModelM = _modelM;
        }



}

#endif
