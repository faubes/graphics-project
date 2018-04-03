// ==========================================================================
// $Id: element.vs,v 1.2 2016/03/12 03:39:35 jlang Exp $
// Draw with normal
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
// Creator: jlang (Jochen Lang)
// Email:   jlang@eecs.uottawa.ca
// ==========================================================================
// $Log: element.vs,v $
// Revision 1.2  2016/03/12 03:39:35  jlang
// Fixed spot light
//
// Revision 1.1  2016/03/06 22:32:50  jlang
// Starter Code and solution for assignment 3
//
// Revision 1.2  2016/02/06 16:52:56  jlang
// Made the object 2x2x2
//
// Revision 1.1  2014/10/27 18:39:43  jlang
// Adapted starter code for assignment fall 2014
//
// Revision 1.2  2014/03/15 04:01:14  jlang
// Fixed material and light uniform for Intel.
//
// Revision 1.1  2014/03/10 05:53:20  jlang
// Added core, debug option
//
// ==========================================================================
#version 330 core

layout (location=0) in vec4 position;
layout (location=1) in vec3 normal;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;	

uniform vec4 lightPosition[2];

out vec4 colorVertFrag; // Pass the color on to rasterization
out vec3 normalFrag; // Pass the normal to rasterization
out vec3 eyeFrag; // Pass an eye vector along
out vec3 lightFrag; // Pass a light vector along


void main() {

  // map the vertex position into clipping space 
  mat4 ModelViewMatrix = ViewMatrix * ModelMatrix;
  // postion in camera coordinates
  vec4 posVec = ModelViewMatrix * position;
  eyeFrag = -posVec.xyz;  

  // light vector in camera coordinates
  // Check for directional lighting	
  vec4 lPos = lightPosition[0];
  if ( lPos.w > 0.0 ) {
    lightFrag = lPos.xyz - posVec.xyz;
  } else {
    lightFrag = lPos.xyz;
  }

  normalFrag = inverse(transpose(mat3(ModelViewMatrix))) * normal;  

  colorVertFrag = vec4( 0.7f, 0.7f, 0.7f, 1.0f );		

  gl_Position = ProjectionMatrix * posVec;
}
