// ==========================================================================
// $Id: texture.h,v 1.3 2018/03/11 03:24:27 jlang Exp $
// Texture will hold texture names and the images
// ==========================================================================
// (C)opyright:
//
//   Jochen Lang
//   EECS, University of Ottawa
//   800 King Edward Ave.
//   Ottawa, On., K1N 6N5
//   Canada.
//   http://www.site.uottawa.ca
//
// Creator: Jochen Lang
// Email:   jlang@eecs.uottawa.ca
// ==========================================================================
// $Log: texture.h,v $
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
#ifndef CSI4130_TEXTURE_H
#define CSI4130_TEXTURE_H

#include <cassert>
#include <sstream>
#include <string>

#include <GL/glew.h>  // GL_BGRA_EXT
#include <GL/glut.h>

// Sticking to C to minimize install effort
extern "C" { // guard probably not needed
#include <FreeImage.h>
}

// #include "shader.h"

using namespace std;

namespace CSI4130 {

struct Texture {
								std::string d_textureFile;
								GLuint d_texObject;

								static inline void bindAllTexture( std::vector<Texture>& textures, bool _mipmap=false );
								static inline GLuint bindTexture( const std::string& fileName, bool _mipmap=false );
								static inline GLuint bindCubeMapTexture(const std::string& path,
																																																const std::string& fileName, bool _mipmap=false);
};

// Helper rourine to loop over a vector of textures and bind them
void Texture::bindAllTexture( std::vector<Texture>& textures, bool _mipmap )
{
								for ( std::vector<Texture>::iterator iter = textures.begin();
														iter != textures.end();
														++iter ) {
																iter->d_texObject = bindTexture( iter->d_textureFile, _mipmap );
								}
								return;
}


// This routine does the setup of texture objects, including
// loading the image, binding the texture, setting up the
// texture environment, and optional mipmapping.
// Note that we call this routine in init() and not in display().
GLuint Texture::bindTexture( const std::string& fileName, bool _mipmap )
{
								GLuint texObject;

								// Generate four texture objects
								glGenTextures( 1, &texObject );

								glPixelStorei(GL_UNPACK_ALIGNMENT, 1 );
								// Bind texture object
								glBindTexture( GL_TEXTURE_2D, texObject );

								FIBITMAP *pBitmap=0;
								switch ( FreeImage_GetFileType(fileName.c_str(), 0)) {
								case FIF_JPEG:
																pBitmap = FreeImage_Load(FIF_JPEG, fileName.c_str(), JPEG_ACCURATE);
																break;
								case FIF_PNG:
																pBitmap = FreeImage_Load(FIF_PNG, fileName.c_str(), PNG_IGNOREGAMMA);
																break;
								case FIF_TARGA:
																pBitmap = FreeImage_Load(FIF_TARGA, fileName.c_str()); // TARGA_LOAD_RGB888 other flag option
								default:
																cerr << "Unexpected file format: ("
																					<< FreeImage_GetFileType(fileName.c_str(), 0) << ") "
																					<< fileName << endl;
																return 0;
								}
								FIBITMAP *pIm = FreeImage_ConvertTo32Bits(pBitmap);
								FreeImage_Unload( pBitmap );

								// Set up texture environment
								glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
								glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

								if ( _mipmap ) {
																// Pass texture to OpenGL
																glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
																													FreeImage_GetWidth(pIm), FreeImage_GetHeight(pIm),
																													0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(pIm));
																FreeImage_Unload( pIm );
																glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
								} else {
																// Let glu calculate mipmap levels
																gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA,
																																		FreeImage_GetWidth(pIm), FreeImage_GetHeight(pIm),
																																		GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(pIm));
																FreeImage_Unload( pIm );
																// mipmap filter nearest neighbor for minimification
																glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
																																GL_NEAREST_MIPMAP_NEAREST);
								}
								// filter bilinear interpolation
								glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
																								GL_LINEAR);
								_printOpenGLerrors(__FILE__,__LINE__);

								return texObject;
}

GLuint Texture::bindCubeMapTexture(const std::string &path, const std::string &fileName, bool _mipmap ) {

								GLuint texCubeMap;

								const std::string face_ext[6] = {"_rt", "_lf", "_up", "_dn", "_bk", "_ft"};
  							glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &texCubeMap);
								errorOut();
								glTextureStorage2D(texCubeMap, 1, GL_RGBA8, 512, 512);
								errorOut();
								for (int face=0; face < 6; face++) {
																std::stringstream ss;
																ss << path << "/" << fileName << face_ext[face] << ".tga";
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
																								pBitmap = FreeImage_Load(FIF_TARGA, ss.str().c_str(), TARGA_LOAD_RGB888); // TARGA_LOAD_RGB888 other flag option
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
																cerr << "converted image" <<endl;
																FreeImage_Unload( pBitmap );
//cerr << "GL_MAX_TEXTURE_SIZE: " << GL_MAX_TEXTURE_SIZE << endl;
																glTextureSubImage3D(texCubeMap, // target
																																				0, // level
																																				0, 0, // x,y offsets
																																				face, // z offset
																																				FreeImage_GetWidth(pIm), //width
																																				FreeImage_GetHeight(pIm), //height
																																				1,
																																				GL_RGBA,
																																				GL_UNSIGNED_BYTE,
																																				(void*)FreeImage_GetBits(pIm)
																																				);
																FreeImage_Unload( pIm);
																// format cube map texture
																glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
																glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
																glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
																glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
																glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
																errorOut();
								}
								return texCubeMap;

}

}
#endif
