// ==========================================================================
// $Id: obj_mesh.h,v 1.3 2016/03/12 03:39:35 jlang Exp $
// Basic obj mesh loader
// Based on Revision 1.1  2014/10/27 18:39:44  jlang
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
// $Log: obj_mesh.h,v $
// Revision 1.3  2016/03/12 03:39:35  jlang
// Fixed spot light
//
// Revision 1.2  2016/03/08 03:00:08  jlang
// Added directory handling, replaced short index with int in gouraud shading, added png loading, fixed empty mtl line errors
//
// Revision 1.1  2016/03/06 22:32:51  jlang
// Starter Code and solution for assignment 3
//
// ==========================================================================
#ifndef CSI4130_OBJ_MESH_H
#define CSI4130_OBJ_MESH_H

#include <string>
#include <vector>
#include <map>
#include <algorithm>

// openGL types
#include <GL/glew.h>
#include <GL/glut.h>

// glm types
#include <glm/glm.hpp>

#include "material.h"
#include "texture.h"

namespace CSI4130 {

/**
* Solution based on user James Kanze on stackoverflow
*/
#ifdef WIN32
struct MatchPathSeparator {
  bool operator()(char ch) const {
    return ch == '\\' || ch == '/';
  }
};
#else
struct MatchPathSeparator {
  bool operator()(char ch) const {
    return ch == '/';
  }
};
#endif
inline std::string xDirectory(std::string const& pathname) {
  return std::string(
    pathname.begin(), std::find_if(pathname.rbegin(), pathname.rend(), MatchPathSeparator()).base());
}
inline std::string xFilename(std::string const& pathname) {
  return std::string(
    std::find_if(pathname.rbegin(), pathname.rend(), MatchPathSeparator()).base(), pathname.end());
}


  class OBJMesh {
    // Vectors for element rendering
    std::vector<glm::vec3> d_vertArr;
    std::vector<glm::vec3> d_normalArr;

    // We assume that we will always get triangles
    std::vector<glm::uvec3> d_faceArr;


    // Vectors for rendering with all indexes unrolled
    std::vector<glm::vec3> d_uniVertArr;
    std::vector<glm::vec3> d_uniNormalArr;
    std::vector<glm::vec4> d_uniColorArr;
    std::vector<glm::vec2> d_uniTexArr;
    std::vector<GLuint> d_matIndex;
    MaterialArray d_mArray;
    std::vector<Texture> d_textureInfo;

    GLuint d_nTri; // no. of unique triangles
    GLuint d_nVert; // no. of unique vertices

    // state variables
    std::map<std::string,GLint> d_matNames;
    int d_matCount;
    bool d_usemtlLib;
    std::string d_mtlFileName;
		bool d_hasTexture;


    // helper arrays
    std::vector<GLuint> d_matFaceIndex;
    std::vector<glm::uvec3> d_normIndexVec;
    std::vector<glm::uvec3> d_texIndexVec;
    std::vector<glm::vec2> d_fileTexVec;
    std::vector<glm::vec3> d_fileNormalVec;


    enum Index { VERTEX, VERTEX_NORMAL, VERTEX_TEXTURE, VERTEX_TEXTURE_NORMAL };

    glm::vec3 d_minCoord;
    glm::vec3 d_maxCoord;

	std::string d_directory;

  public:
    inline OBJMesh(const std::string& _fileName);

    // Indicates if a texture image has been found
    inline bool hasTexture();

    // return mesh dimensions
    inline glm::vec3 getMinCoord() const;
    inline glm::vec3 getMaxCoord() const;

    // Get data for Element drawing
    // Available for all meshes read from obj indexed mesh file
    // const reference to a vector suitable for loading into openGL
    inline const std::vector<glm::vec3>& getVertexRef();
    // returns a vector of normals: one for each vertex
    // normal is calculated with gouraud shading if not available
    inline const std::vector<glm::vec3>& getNormalRef();
    inline const std::vector<glm::uvec3>& getElementRef();

    // Get data for Triangle drawing - each triangle has unique vertices
    // Normals, color and material are generated if not part of the file
    // Available for all meshes read from obj indexed mesh file
    // const reference to a vector suitable for loading into openGL
    inline const std::vector<glm::vec3>& getUniVertexRef();
    inline const std::vector<glm::vec3>& getUniNormalRef();
    inline const std::vector<glm::vec4>& getUniColorRef();
    inline const std::vector<GLuint>& getMatIdRef();
    // array of materials for Blinn-Phong material
    // array index as above
    inline MaterialArray getMaterial();

    // texture coordinates - only available if part of the file
    inline const std::vector<glm::vec2>& getUniTexRef();

    // texture image file names, may be size 0 - images are never loaded
    inline std::vector<Texture> getTextureInfo();

  protected:
    bool read(const std::string& _fileName);
    bool readMtl();
    void makeElements();
    void makeUnique();
    void gouraudNormals();
    Index parseField( std::string token );
  };


	/**
	 * Inline methods
	 */

  OBJMesh::OBJMesh(const std::string& _fileName )
    : d_usemtlLib(false) {
    d_directory = xDirectory(_fileName);
    std::string fileName = xFilename(_fileName);
    if ( read( fileName ) && readMtl() ) {
      makeElements();
      makeUnique();
    }
  }

	bool OBJMesh::hasTexture() {
		return d_hasTexture;
	}

  glm::vec3 OBJMesh::getMinCoord() const {
    return d_minCoord;
  }

  glm::vec3 OBJMesh::getMaxCoord() const {
    return d_maxCoord;
  }

  const std::vector<glm::vec3>& OBJMesh::getVertexRef() {
    return d_vertArr;
  }
  const std::vector<glm::vec3>& OBJMesh::getNormalRef() {
    return d_normalArr;
  }
  const std::vector<glm::uvec3>& OBJMesh::getElementRef() {
    return d_faceArr;
  }

  // Get data for Triangle drawing - each triangle has unique vertices
  const std::vector<glm::vec3>& OBJMesh::getUniVertexRef() {
    return d_uniVertArr;
  }
  const std::vector<glm::vec3>& OBJMesh::getUniNormalRef() {
    return d_uniNormalArr;
  }
  const std::vector<glm::vec4>& OBJMesh::getUniColorRef() {
    return d_uniColorArr;
  }
  const std::vector<GLuint>& OBJMesh::getMatIdRef() {
    return d_matIndex;
  }
  MaterialArray OBJMesh::getMaterial() {
    return d_mArray;
  }

  const std::vector<glm::vec2>& OBJMesh::getUniTexRef() {
    return d_uniTexArr;
  }

	// return a copy that can be manipulated
	std::vector<Texture> OBJMesh::getTextureInfo() {
		return d_textureInfo;
	}

} // end of namespace

#endif
