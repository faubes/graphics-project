// ==========================================================================
// $Id: viewer.cpp,v 1.6 2018/03/11 03:24:27 jlang Exp $
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
// $Log: viewer.cpp,v $
// Revision 1.6  2018/03/11 03:24:27  jlang
// Update for project 2018
//
// Revision 1.5  2016/03/12 03:39:36  jlang
// Fixed spot light
//
// Revision 1.4  2016/03/08 03:00:09  jlang
// Added directory handling, replaced short index with int in gouraud shading, added png loading, fixed empty mtl line errors
//
// Revision 1.3  2016/03/06 22:38:49  jlang
// Deactivated light debugging
//
// Revision 1.2  2016/03/06 22:34:40  jlang
// Deactivated light debugging
//
// Revision 1.1  2016/03/06 22:32:51  jlang
// Starter Code and solution for assignment 3
//
// ==========================================================================
#include "viewer.h"
#include "obj_mesh.h"

using std::cerr;
using std::endl;

namespace CSI4130 {
  /**
   * Constructor which loads mesh file and copies data into OpenGL buffers
   */
  Viewer::Viewer( std::string _mFile, int _nLights ) : d_program(-1), d_unit(1.0) {
    OBJMesh oMesh( _mFile );
		d_withTexture = oMesh.hasTexture();
		// Determine transform to make mesh 2* unit size at center
		glm::vec3 min = oMesh.getMinCoord();
		glm::vec3 max = oMesh.getMaxCoord();
		// scale to +/- 1
		glm::vec3 scale = max-min;
		double sc =  std::max( std::max( scale.x, scale.y ), scale.z );
		scale = glm::vec3(2.0/sc);
		d_unit = glm::scale( d_unit, scale );
		// center =  -(min + (max - min)/2) = -min/2  -max/2
		glm::vec3 center = min+max;
		center *= -0.5;
		d_unit = glm::translate( d_unit, center );
#ifdef DEBUG_VIEWER_VERBOSE
		cerr << "Min: " << glm::to_string(min) << endl;
		cerr << "Max: " << glm::to_string(max) << endl;
		cerr << "Scale: " << glm::to_string(scale) << endl;
		cerr << "Center: " << glm::to_string(center) << endl;
#endif


    // all our meshes will support element (index) based rendering
    glGenVertexArrays(1, &d_vao_ele );
    glBindVertexArray( d_vao_ele );
    const std::vector<glm::vec3> vertices = oMesh.getVertexRef();
#ifdef DEBUG_VIEWER_VERBOSE
    printBuffer<float>( &vertices[0], 3*vertices.size() );
#endif
    //    generateBuffer( d_vbo_ele, sizeof(glm::vec3)*static_cast<int>(vertices.size()), &vertices[0] );
    generateBuffer( d_vbo_ele, sizeof(glm::vec3)*static_cast<int>(oMesh.getVertexRef().size()), &oMesh.getVertexRef()[0] );
    const std::vector<glm::vec3> normals = oMesh.getNormalRef();
    generateBuffer( d_nbo_ele, sizeof(glm::vec3)*static_cast<int>(normals.size()), &normals[0] );
    errorOut();
    // Here we also need to create our own colors and load them in an OpenGL Buffer
    // d_cbo_ele

    const std::vector<glm::uvec3> elements = oMesh.getElementRef();
    d_nIndices = static_cast<GLuint>(elements.size());
#ifdef DEBUG_VIEWER_VERBOSE
    printBuffer<unsigned int>( &elements[0], 3*d_nIndices );
    cerr << "No. vertics: " << vertices.size() << " " << *std::max_element(reinterpret_cast<const unsigned*>(&elements[0]),reinterpret_cast<const unsigned*>(&elements[0] + d_nIndices)+1) << endl;
#endif
    // Element array buffer object
    glGenBuffers(1, &d_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, d_ebo );
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		 sizeof(glm::uvec3) * d_nIndices,
		 &elements[0], GL_STATIC_DRAW );
    errorOut();

    // OBJMesh will always produce unrolled indexes
    // with normal, material and color attributes
    glGenVertexArrays(1, &d_vao );
    glBindVertexArray( d_vao );
    const std::vector<glm::vec3> uniVertices = oMesh.getUniVertexRef();
    d_nVert = static_cast<GLuint>(uniVertices.size());
    generateBuffer( d_vbo, sizeof(glm::vec3)*d_nVert, &uniVertices[0] );
    const std::vector<glm::vec3> uniNormals = oMesh.getUniNormalRef();
#ifdef DEBUG_VIEWER
		cerr << "No. normals: " << uniNormals.size() << endl;
#endif
    generateBuffer( d_nbo, sizeof(glm::vec3)*static_cast<int>(uniNormals.size()), &uniNormals[0] );
    const std::vector<glm::vec4> colors = oMesh.getUniColorRef();
#ifdef DEBUG_VIEWER
		cerr << "Colors: " << colors.size() << endl;
#endif
    generateBuffer( d_cbo, sizeof(glm::vec4)*static_cast<int>(colors.size()), &colors[0] );
    const std::vector<GLuint> mIndices =  oMesh.getMatIdRef();
#ifdef DEBUG_VIEWER
		cerr << "Mat. indices: " << mIndices.size() << endl;
		// printBuffer<unsigned int>( &mIndices[0], mIndices.size(), 80 );
#endif
    generateBuffer( d_mbo, sizeof(GLuint)*static_cast<int>(mIndices.size()), &mIndices[0] );
		if ( d_withTexture ) {
			const std::vector<glm::vec2> texCoord = oMesh.getUniTexRef();
#ifdef DEBUG_VIEWER
			cerr << "Texture: " << texCoord.size() << endl;
#endif
			d_texInfo = oMesh.getTextureInfo();
			// bind textures
			Texture::bindAllTexture( d_texInfo );
			generateBuffer( d_tbo, sizeof(glm::vec2)*static_cast<int>(texCoord.size()), &texCoord[0] );
		}

		d_mArray = oMesh.getMaterial();
    // Generate an uniform buffer object
    glGenBuffers(1, &d_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, d_ubo);
    // reserve the memory
    glBufferData(GL_UNIFORM_BUFFER, d_mArray.getSize(), 0, GL_STATIC_DRAW );
    // copy the data to OpenGL
    d_mArray.setMaterialsUBO(d_ubo);
    errorOut();

    // Done with mesh loader

    // setting up programs
    d_progElement = loadShader( "element" );
		d_progColor = loadShader( "color" );
		d_progMaterial = loadShader( "material" );
		if ( oMesh.hasTexture() ) {
			d_progTexture = loadShader( "texture" );
		}

    // finally make element drawing active
    setProgElement();
  }


	bool Viewer::switchProgram( GLuint program ) {
    if ( d_program == program ) return false;
#ifdef DEBUG_VIEWER_VERBOSE
    cerr << "Use program: " << program << endl;
#endif
    d_attrib.disableAll();
    // Install program
    d_program = program;
    glUseProgram(d_program);
    d_tfm.updateLocation( d_program );
    d_attrib.updateLocation( d_program );
    errorOut();
    // Light source uniforms
    d_light.d_lightArray.setLights(d_program);
    errorOut();
		return true;
	}


  /**
   * Make element drawing active
   */
  bool Viewer::setProgElement() {
		if ( !switchProgram( d_progElement )) return true;
    // switch to corresponding vao
    //cerr << "Use vao: " << d_vao_ele << endl;
    glBindVertexArray( d_vao_ele );
#ifdef DEBUG_VIEWER_VERBOSE
    cerr << "Binding ebo: " << d_ebo << endl;
#endif
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,d_ebo);
    errorOut();
#ifdef DEBUG_VIEWER_VERBOSE
    cerr << "Binding nbo_ele: " << d_nbo_ele << endl;
#endif
    glBindBuffer(GL_ARRAY_BUFFER,d_nbo_ele);
    d_attrib.enableVANorm();
#ifdef DEBUG_VIEWER_VERBOSE
    cerr << "Binding cbo_ele: " << d_cbo_ele << endl;
#endif
    // Here bind cbo for element drawing
#ifdef DEBUG_VIEWER_VERBOSE
    cerr << "Binding vbo_ele: " << d_vbo_ele << endl;
#endif
    glBindBuffer(GL_ARRAY_BUFFER,d_vbo_ele);
    d_attrib.enableVAPos();
    errorOut();
    return true;
  }

	void Viewer::setArrayVAO() {
    // switch to corresponding vao
    //cerr << "Use vao: " << d_vao << endl;
    glBindVertexArray( d_vao );
#ifdef DEBUG_VIEWER_VERBOSE
    cerr << "Binding nbo: " << d_nbo << endl;
#endif
    glBindBuffer(GL_ARRAY_BUFFER,d_nbo);
    d_attrib.enableVANorm();
#ifdef DEBUG_VIEWER_VERBOSE
	  cerr << "Binding vbo: " << d_vbo << endl;
#endif
	  glBindBuffer(GL_ARRAY_BUFFER, d_vbo);
	  d_attrib.enableVAPos();
    errorOut();
		return;
	}

	void Viewer::setMaterialBuffer() {
    // Here bind mbo for indexed material rendering
		//#ifdef DEBUG_VIEWER_VERBOSE
    cerr << "Binding mbo: " << d_mbo << endl;
		//#endif
    glBindBuffer(GL_ARRAY_BUFFER,d_mbo);
    d_attrib.enableVAMat();
    // Link the buffer object to the material uniform block
    GLuint bI = glGetUniformBlockIndex(d_program, "MaterialBlock" );
    glUniformBlockBinding( d_program, bI, 0);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, d_ubo);
    errorOut();
		return;
	}

  /**
   * Make color drawing active: to be implemented
   */
  bool Viewer::setProgColor() {
		if ( !switchProgram( d_progColor )) return true;
		setArrayVAO();
#ifdef DEBUG_VIEWER_VERBOSE
    cerr << "Binding cbo: " << d_cbo << endl;
#endif
    glBindBuffer(GL_ARRAY_BUFFER,d_cbo);
    d_attrib.enableVAColor();
    return true;
  }


  /**
   * Make material drawing active: to be implemented
   */
  bool Viewer::setProgMaterial() {
		if ( !switchProgram( d_progMaterial )) return true;
		setArrayVAO();
		setMaterialBuffer();
		glBindBuffer(GL_ARRAY_BUFFER,d_mbo);
		d_attrib.enableVAMat();
    return true;
  }


  /**
   * Texture drawing active: to be implemented
   */
  bool Viewer::setProgTexture() {
		if ( !d_withTexture ) return false;
		if ( !switchProgram( d_progTexture )) return true;
		setArrayVAO();
		setMaterialBuffer();
#ifdef DEBUG_VIEWER_VERBOSE
    cerr << "Binding tbo: " << d_tbo << endl;
#endif
    glBindBuffer(GL_ARRAY_BUFFER,d_tbo);
    d_attrib.enableVATexture();
		errorOut();
		// Link the texture to the sampler
		GLuint texLoc = glGetUniformLocation( d_program, "texImage");
		glUniform1i(texLoc, 0); // set to texture unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, d_texInfo[0].d_texObject );
		errorOut();
    return true;
  }



  /************************************************************************************/
  /* Helper functions
  /************************************************************************************/


  /**
   * Loads, compiles, links a vertex and fragment shader into a program.
   */
  GLuint Viewer::loadShader( std::string _basename ) {
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



  /**
   * Reserves a buffer object _bo of size _sz bytes in openGL
   * Data will be loaded from _ptr if not null
   * It will be bound to an ARRAY_BUFFER binding point
   **/
  void Viewer::generateBuffer( GLuint& _bo, int _sz, const void* _ptr ) {
    glGenBuffers(1, &_bo);
    glBindBuffer(GL_ARRAY_BUFFER, _bo );
//#ifdef DEBUG_VIEWER_VERBOSE
    cerr << "Setting buffer data: " << _bo << " " << _sz << " " << std::hex <<
      _ptr << std::dec << endl;
//#endif
    glBufferData(GL_ARRAY_BUFFER,
		 _sz,
		 _ptr, GL_STATIC_DRAW );
    errorOut();
  }


  void Viewer::setLightPosition( GLuint program, GLfloat dist ) {
    // Place the current light source at a radius from the camera
    LightSource light = d_light.d_lightArray.get( d_light.d_cLight );
    glm::vec4 lightPos =
      glm::vec4( cos(d_light.d_angle)*dist,
		 sin(d_light.d_angle)*dist + dist/2.0 + 0.5,
		 dist,
		 static_cast<GLfloat>( light.d_pointLight ));
#ifdef DEBUG_LIGHT
    cerr << "Light pos: " << glm::to_string(lightPos) << endl;
#endif
    light.d_position = lightPos;
    d_light.d_lightArray.set( d_light.d_cLight, light );
    d_light.d_lightArray.setPosition( program, d_light.d_cLight );
    errorOut();
    return;
  }

} // end namespace
