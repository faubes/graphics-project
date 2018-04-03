// ==========================================================================
// $Id: obj_mesh.cpp,v 1.4 2018/03/11 03:24:27 jlang Exp $
// Basic obj mesh loader
// Based on Revision 1.1  2014/10/27 18:39:44  jlang
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
// $Log: obj_mesh.cpp,v $
// Revision 1.4  2018/03/11 03:24:27  jlang
// Update for project 2018
//
// Revision 1.3  2016/03/27 17:25:20  jlang
// Fixed divide zero for vertices without face
//
// Revision 1.2  2016/03/08 03:00:08  jlang
// Added directory handling, replaced short index with int in gouraud shading, added png loading, fixed empty mtl line errors
//
// Revision 1.1  2016/03/06 22:32:51  jlang
// Starter Code and solution for assignment 3
//
// ==========================================================================
#include <fstream>
#include <iostream>
#include <algorithm>
#include <string>

// openGL types
#include <GL/glew.h>
// glm types
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "obj_mesh.h"


// #define UNIT_TEST_OBJ_MESH
// #define DEBUG_OBJ_MESH

using namespace std;

namespace CSI4130 {

// define helper stream extracter
inline std::istream& operator>>(std::istream& is, glm::vec3& v) {
  is >> v.x >> v.y >> v.z;
  return is;
}

inline std::istream& operator>>(std::istream& is, glm::vec2& v) {
  is >> v.x >> v.y;
  return is;
}

class vec3H : public glm::vec4 {
};

inline std::istream& operator>>(std::istream& is, vec3H& v) {
  is >> v.x >> v.y >> v.z;
  v.w = 1.0f;
  return is;
}

inline std::ostream& operator<<(std::ostream& os, glm::vec3& v) {
  os << v.x << " " << v.y << " " << v.z;
  return os;
}

inline std::ostream& operator<<(std::ostream& os, glm::uvec3& v) {
  os << v.x << " " << v.y << " " << v.z;
  return os;
}



bool OBJMesh::read(const std::string& _fileName)
{
  ifstream file((d_directory + _fileName).c_str());
  if (!file.is_open()){
    cerr << "Error: " << d_directory << _fileName << " does not exist" << endl;
    return false;
  }
	d_hasTexture = false;
  const glm::uvec3 one(1,1,1);
  GLfloat Xmax, Xmin, Ymax, Ymin, Zmax, Zmin;
  GLuint cMatId = 0;
  d_matCount = -1;
  Index iType = VERTEX;
  // Parse one line at a time
  string line, token;
  cerr << "Reading ..." << endl;
  while (getline(file, line)) {
    istringstream streamLine(line);
#ifdef DEBUG_OBJ_MESH
    cerr << "Got: " << line << endl;
#endif
    streamLine.peek();
    if (streamLine.eof()) continue;
    streamLine >> token;
    // Process key
#ifdef DEBUG_OBJ_MESH
    cerr << token << endl;
#endif
    if ( token.compare("mtllib")==0) {
      streamLine >> d_mtlFileName;
			//#ifdef DEBUG_OBJ_MESH
      cerr << "Found mtllib: " << d_mtlFileName << endl;
			//#endif
      d_usemtlLib = true;
      continue;
    }
    switch ( token[0] ) {
    case 'v':
      // read vertex and vertex normals
      if ( token.compare("v")==0 ) {
	// read vertex
	glm::vec3 vert;
	streamLine >> vert;
	//find minimum and maximum in each dimension
	if (d_vertArr.size()==0) {
	  Xmax = Xmin = vert.x;
	  Ymax = Ymin = vert.y;
	  Zmax = Zmin = vert.z;
	} else {
	  Xmin = min( Xmin, vert.x );
	  Xmax = max( Xmax, vert.x );
	  Ymin = min( Ymin, vert.y );
	  Ymax = max( Ymax, vert.y );
	  Zmin = min( Zmin, vert.z );
	  Zmax = max( Zmax, vert.z );
	}
	d_vertArr.push_back(vert);
	continue;
      }
      if( token.compare("vn")==0 ) {
	// read vertex normals
	glm::vec3 norm;
	streamLine >> norm;
	d_fileNormalVec.push_back(norm);
	continue;
      }
      if( token.compare("vt")==0 ) {
	// read vertex normals
	glm::vec2 tex;
	streamLine >> tex;
	d_fileTexVec.push_back(tex);
	continue;
      }
      break;
    case 'f':
      // read face
      if ( token.compare("f")==0 ) {
	glm::uvec3 face;
	glm::uvec3 faceTex;
	glm::uvec3 faceNorm;
	streamLine >> token;
	if ( d_faceArr.size() == 0 ) {
	  iType = parseField( token );
	}
	istringstream stm(token);
	switch ( iType ) {
	case VERTEX:
	  stm >> face.x;
	  streamLine >> token;
	  stm.str(token); stm.clear();
	  stm >> face.y;
	  streamLine >> token;
	  stm.str(token); stm.clear();
	  stm >> face.z;
#ifdef DEBUG_OBJ_MESH
	  cerr << "Face: " << face << endl;
#endif
	  break;
	case VERTEX_TEXTURE:
	  stm >> face.x;
	  stm.ignore(); // discard slash
	  stm >> faceTex.x;
	  streamLine >> token;
	  stm.str(token); stm.clear();
	  stm >> face.y;
	  stm.ignore(); // discard slash
	  stm >> faceTex.y;
	  streamLine >> token;
	  stm.str(token); stm.clear();
	  stm >> face.z;
	  stm.ignore(); // discard slash
	  stm >> faceTex.z;
#ifdef DEBUG_OBJ_MESH
	  cerr << "Face: " << face << endl;
	  cerr << "Face Tex: " << faceTex << endl;
#endif
	  faceTex -= one;;
	  d_texIndexVec.push_back(faceTex);
	  break;
	case VERTEX_NORMAL:
	  stm >> face.x;
	  stm.ignore(2); // discard 2 slashes
	  stm >> faceNorm.x;
	  streamLine >> token;
	  stm.str(token); stm.clear();
	  stm >> face.y;
	  stm.ignore(2); // discard 2 slashes
	  stm >> faceNorm.y;
	  streamLine >> token;
	  stm.str(token); stm.clear();
	  stm >> face.z;
	  stm.ignore(2); // discard 2 slashes
	  stm >> faceNorm.z;
#ifdef DEBUG_OBJ_MESH
	  cerr << "Face: " << face << endl;
	  cerr << "Face Norm: " << faceNorm << endl;
#endif
	  faceNorm -= one;
	  d_normIndexVec.push_back(faceNorm);
	  break;
	case VERTEX_TEXTURE_NORMAL:
	  stm >> face.x;
	  stm.ignore(); // discard slash
	  stm >> faceTex.x;
	  stm.ignore(); // discard slash
	  stm >> faceNorm.x;
	  streamLine >> token;
	  stm.str(token); stm.clear();
	  stm >> face.y;
	  stm.ignore(); // discard slash
	  stm >> faceTex.y;
	  stm.ignore(); // discard slash
	  stm >> faceNorm.y;
	  streamLine >> token;
	  stm.str(token); stm.clear();
	  stm >> face.z;
	  stm.ignore(); // discard slash
	  stm >> faceTex.z;
	  stm.ignore(); // discard slash
	  stm >> faceNorm.z;
#ifdef DEBUG_OBJ_MESH
	  cerr << "Face: " << face << endl;
	  cerr << "Face Tex: " << faceTex << endl;
	  cerr << "Face Norm: " << faceNorm << endl;
#endif
	  faceTex -= one;
	  d_texIndexVec.push_back(faceTex);
	  faceNorm -= one;
	  d_normIndexVec.push_back(faceNorm);
	  break;
	}
	face -= one;
	d_faceArr.push_back(face);
	// register the current material with the face; may be -1
	d_matFaceIndex.push_back(cMatId);
      }
      break;
    case 'u':
      // use material
      if ( token.compare("usemtl")==0 ){
        streamLine >> token;
				std::map<string,GLint>::iterator iter = d_matNames.find(token);
				if (iter == d_matNames.end()) {
					cMatId = ++d_matCount;
					d_matNames[token] = cMatId;
				} else {
					cMatId = iter->second;
				}
				cerr << "Material index: " << cMatId << endl;
      }
      break;
    default:
      //cerr << "Unknown line: " << line << endl;
      break;
    }
  }
  d_nTri = static_cast<GLuint>(d_faceArr.size());
  file.close();
  d_minCoord = glm::vec3( Xmin, Ymin, Zmin );
  d_maxCoord = glm::vec3( Xmax, Ymax, Zmax );
  d_nVert = static_cast<GLuint>(d_vertArr.size());
  return true;
}


bool OBJMesh::readMtl() {
  Material mat;
	Texture tex;
  if (!d_usemtlLib ) {
    d_mArray.append( mat );
    return true;
  }
  ifstream fileMtl( (d_directory + d_mtlFileName).c_str() );
  if( !(d_usemtlLib = fileMtl.is_open()) ) {
    cerr << d_mtlFileName << " does not exist" << endl;
    return false;
  }
#ifdef DEBUG_OBJ_MESH
  cerr << "Materials assigned: " << d_matCount+1 << endl;
#endif
  // Initialize the array with default materials
  for ( int i=0; i<d_matCount+1; ++i ) {
    d_mArray.append( mat );
  }
  int cMatId = -1;
  vec3H color;
  // Parse one line at a time
  string line, token, name;
  cerr << "Reading " << d_mtlFileName << " ..." << endl;
  while(getline(fileMtl,line)) {
    if (line.size() == 0) continue; // guard against empty lines
    istringstream streamLine(line);
#ifdef DEBUG_OBJ_MESH
    cerr << "Got: " << line << endl;
#endif
    streamLine >> token;
    // Process key
    // cerr << token << " ";
    if (token.compare("newmtl")==0) {
      // Save previous material
      if ( cMatId != -1 ) {
				d_mArray.set( cMatId, mat );
      }
      // Start a new material
      streamLine >> name;
      std::map<string,GLint>::iterator iter = d_matNames.find(name);
      if (iter == d_matNames.end()) {
				cerr << "Unused material: " << name;
      } else {
				cMatId = iter->second;
				//#ifdef DEBUG_OBJ_MESH
				cerr << "Using material: " << name << " id: " << cMatId << endl;
				//#endif
      }
      continue;
    }
    if (token.compare("Ka")==0){
      streamLine >> color;
      mat.d_ambient = color;
      continue;
    }
    if (token.compare("Kd")==0){
      streamLine >> color;
      mat.d_diffuse = color;
      continue;
    }
    if (token.compare("Ks")==0){
      streamLine >> color;
      mat.d_specular = color;
      continue;
    }
    if (token.compare("Ns")==0){
      streamLine >> mat.d_shininess;
      continue;
    }
    if (token.compare("map_Kd")==0){
      streamLine >> tex.d_textureFile;
      tex.d_textureFile = d_directory + tex.d_textureFile;
			cerr << "Using texture: " << tex.d_textureFile << endl;
			d_hasTexture = true;
			d_textureInfo.push_back( tex );
      continue;
    }
  } // while
  if ( cMatId != -1 ) {
    d_mArray.set( cMatId, mat );
  }
  return true;
}


void OBJMesh::makeElements() {
   // ensure we have one normal per vertex
   if ( d_normIndexVec.size() == 0 || d_normIndexVec.size() != d_vertArr.size()) {
     // use gouraud shading to create normal for elements
     gouraudNormals();
     return;
   }
   // copy normals
   d_normalArr = d_fileNormalVec;
   return;
}


void OBJMesh::makeUnique() {
  glm::vec4 color( 0.8f, 0.8f, 0.8f, 1.0f ); // default color is white
  d_uniVertArr.reserve( 3*d_nTri );
  d_uniNormalArr.reserve( 3*d_nTri );
  d_uniColorArr.reserve( 3*d_nTri );
  d_matIndex.reserve( 3*d_nTri );

  GLuint prevMatFaceIndex = -1;
  GLuint cMatIndex = 0;
  for (GLuint i = 0; i != d_nTri; ++i) {
    if ( d_matFaceIndex[i] != -1 ) {
      if ( prevMatFaceIndex != d_matFaceIndex[i] ) {
				cMatIndex = d_matFaceIndex[i];
				Material m = d_mArray.get(cMatIndex);
				color = m.d_diffuse;
				prevMatFaceIndex = cMatIndex;
      }
    }

    GLuint p = d_faceArr[i].x;
    d_uniVertArr.push_back( d_vertArr[p] );
    d_uniColorArr.push_back( color );
    d_matIndex.push_back( cMatIndex );

    p = d_faceArr[i].y;
    d_uniVertArr.push_back( d_vertArr[p] );
    d_uniColorArr.push_back( color );
    d_matIndex.push_back( cMatIndex );

    p = d_faceArr[i].z;
    d_uniVertArr.push_back( d_vertArr[p] );
    d_uniColorArr.push_back( color );
    d_matIndex.push_back( cMatIndex );
  }

  // Copy normals read frome file or the Gouraud shading normals
  if ( d_normIndexVec.size() > 0 )  {
#ifdef DEBUG_OBJ_MESH
    cerr << "File normal array: " << d_fileNormalVec.size() << endl;
    cerr << "Normal index array: " << d_normIndexVec.size() << endl;
#endif
    for (GLuint i = 0; i != d_nTri; ++i) {
#ifdef DEBUG_OBJ_MESH
      cerr << "NormIndex: " << d_normIndexVec[i] << endl;
#endif
      d_uniNormalArr.push_back( d_fileNormalVec[d_normIndexVec[i].x] );
      d_uniNormalArr.push_back( d_fileNormalVec[d_normIndexVec[i].y] );
      d_uniNormalArr.push_back( d_fileNormalVec[d_normIndexVec[i].z] );
    }
  } else {
#ifdef DEBUG_OBJ_MESH
    cerr << "Normal array: " << d_normalArr.size() << endl;
#endif
    for (GLuint i = 0; i != d_nTri; ++i) {
      d_uniNormalArr.push_back( d_normalArr[d_faceArr[i].x]);
      d_uniNormalArr.push_back( d_normalArr[d_faceArr[i].y]);
      d_uniNormalArr.push_back( d_normalArr[d_faceArr[i].z]);
    }
  }
  if ( d_texIndexVec.size() > 0 ) {
    for (GLuint i = 0; i != d_nTri; ++i) {
      d_uniTexArr.push_back( d_fileTexVec[d_texIndexVec[i].x] );
      d_uniTexArr.push_back( d_fileTexVec[d_texIndexVec[i].y] );
      d_uniTexArr.push_back( d_fileTexVec[d_texIndexVec[i].z] );
    }
  }
  return;
}

void OBJMesh::gouraudNormals() {
  //Generate normals
  d_normalArr.resize( d_nVert, glm::vec3(0.0f,0.0f,0.0f) );
  for (GLuint i = 0; i != d_nTri; ++i){
    GLuint p0 = d_faceArr[i].x;
    GLuint p1 = d_faceArr[i].y;
    GLuint p2 = d_faceArr[i].z;

    glm::vec3 a = d_vertArr[p1];
    a -= d_vertArr[p0];
    glm::vec3 b = d_vertArr[p2];
    b -= d_vertArr[p1];
    // cross product a, b
    glm::vec3 faceNormal;
    faceNormal.x = a.y*b.z - a.z*b.y;
    faceNormal.y = a.z*b.x - a.x*b.z;
    faceNormal.z = a.x*b.y - a.y*b.x;
		// we don't normalize here assuming bigger triangles have more
		// weight in the normal
    d_normalArr[p0] += faceNormal;
    d_normalArr[p1] += faceNormal;
    d_normalArr[p2] += faceNormal;
  }
	// Normalize
  for (GLuint i = 0; i < d_nVert; ++i) {
		if (glm::length(d_normalArr[i]) < 1e-6 ) {
			std::cerr << "Zero normal at " << i << " :";
			std::cerr << glm::to_string(d_normalArr[i]) << std::endl;
		} else {
			d_normalArr[i] = glm::normalize(d_normalArr[i]);
		}
	}
  return;
}


OBJMesh::Index OBJMesh::parseField( std::string token ) {
  // Count the number of slashes
  std::size_t fPos = token.find('/');
  if ( fPos == std::string::npos ) {
    // vertex indeces only
    return VERTEX;
  }
  // see if second slash is found
  std::size_t sPos = token.find('/', fPos+1);
  if ( sPos == std::string::npos ) {
    // Only one  slash
    return VERTEX_TEXTURE;
  }
  if ( fPos+1 == sPos ) {
    // Two slashes without gap
    return VERTEX_NORMAL;
  }
  return VERTEX_TEXTURE_NORMAL;
}

}  // end namespace


#ifdef UNIT_TEST_OBJ_MESH
using namespace CSI4130;

int main(int argc, char** argv) {
  std::string fName = "cube.obj";
  if ( argc > 1 ) {
    fName = argv[1];
  }
  OBJMesh oM(fName);
  return 0;
}
#endif
