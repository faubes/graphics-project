#include "skybox.h"

namespace CSI4130 {

// https://learnopengl.com/code_viewer.php?code=advanced/cubemaps_skybox_data
float Skybox::skyboxVertices[3*36] = {
        // positions
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
};

Skybox::Skybox(const std::string & path, const std::string & fileName, const std::string & ext, int size) {

        // load cube map texture
        d_cubeMapTexture = Skybox::bindCubeMapTexture(path, fileName, ext, size);
        errorOut();
        // send skybox vertices to openGL
        glGenBuffers(1, &d_skybox_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, d_skybox_vbo);
        glBufferData(GL_ARRAY_BUFFER, 3 * 36 * sizeof(float), &skyboxVertices, GL_STATIC_DRAW);
        glGenVertexArrays(1, &d_skybox_vao);
        glBindVertexArray(d_skybox_vao);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, d_skybox_vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        // set up program
        d_progSkybox = Skybox::loadShader( "skybox" );
        d_tfm.updateLocation( d_progSkybox );

}

GLuint Skybox::bindCubeMapTexture(const std::string &path, const std::string &fileName, const std::string &ext, int size ) {

        GLuint texCubeMap;
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        const std::string face_ext[6] = {"_rt", "_lf", "_up", "_dn", "_bk", "_ft"};
        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &texCubeMap);
        errorOut();
        glTextureStorage2D(texCubeMap, 1, GL_RGBA8, size, size);
        errorOut();
        for (int face=0; face < 6; face++) {
                std::stringstream ss;
                ss << path << "/" << fileName << face_ext[face] << "." << ext;
                cerr << "loading file " << ss.str() << endl;
                FIBITMAP *pBitmap=0;
                switch ( FreeImage_GetFileType(ss.str().c_str(), 0)) {
                case FIF_JPEG:
                        pBitmap = FreeImage_Load(FIF_JPEG, ss.str().c_str(), JPEG_ACCURATE);
                        break;
                case FIF_PNG:
                        pBitmap = FreeImage_Load(FIF_PNG, ss.str().c_str(), PNG_IGNOREGAMMA);
                        break;
                case FIF_TARGA:
                        pBitmap = FreeImage_Load(FIF_TARGA, ss.str().c_str()); // TARGA_LOAD_RGB888 other flag option
                        break;
                default:
                        cerr << "Unexpected file format: ("
                             << FreeImage_GetFileType(ss.str().c_str(), 0) << ") "
                             << fileName << endl;
                        return 0;
                }

                cerr << "Width: " << FreeImage_GetWidth(pBitmap)
                     << ", Height: " << FreeImage_GetHeight(pBitmap) << endl;
                FIBITMAP *pIm = FreeImage_ConvertTo32Bits(pBitmap);
                
                  FreeImage_FlipVertical(pIm);

                cerr << "converted image" <<endl;
                FreeImage_Unload( pBitmap );
//cerr << "GL_MAX_TEXTURE_SIZE: " << GL_MAX_TEXTURE_SIZE << endl;
                glTextureSubImage3D(texCubeMap,                 // target
                                    0,                                     // level
                                    0, 0,                                     // x,y offsets
                                    face,                                     // z offset
                                    FreeImage_GetWidth(pIm),                                     //width
                                    FreeImage_GetHeight(pIm),                                     //height
                                    1,
                                    GL_RGBA,
                                    GL_UNSIGNED_BYTE,
                                    (void*)FreeImage_GetBits(pIm)
                                    );
                                    errorOut();
                FreeImage_Unload( pIm);
                // format cube map texture
                //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                errorOut();
        }
        return texCubeMap;

}
void Skybox::draw() {
        glDepthMask(GL_FALSE);
        glUseProgram(d_progSkybox);
        // d_tfm.updateLocation( d_progSkybox );
        // d_tfm.setUniforms();
        errorOut();
        glBindTexture(GL_TEXTURE_CUBE_MAP, d_cubeMapTexture);
        glBindVertexArray(d_skybox_vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthMask(GL_TRUE);
}

/**
 * Loads, compiles, links a vertex and fragment shader into a program.
 */
GLuint Skybox::loadShader( std::string _basename ) {
  vector<GLuint> sHandles;
  GLuint handle;
  Shader sh;
  GLuint program;
  if ( !sh.load(_basename + ".vs", GL_VERTEX_SHADER )) {
    sh.installShader( handle, GL_VERTEX_SHADER );
    Shader::compile( handle );
    sHandles.push_back( handle );
  }
  if ( !sh.load(_basename + ".fs", GL_FRAGMENT_SHADER )) {
    sh.installShader( handle, GL_FRAGMENT_SHADER );
    Shader::compile( handle );
    sHandles.push_back( handle );
  }
  cerr << "No of handles: " << sHandles.size() << endl;
  Shader::installProgram(sHandles, program);
  errorOut();
  return program;
}

}
