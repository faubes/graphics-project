// ==========================================================================
// $Id: color.vs,v 1.2 2016/03/12 03:39:35 jlang Exp $
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
// $Log: color.vs,v $
// Revision 1.2  2016/03/12 03:39:35  jlang
// Fixed spot light
//
// Revision 1.1  2016/03/06 22:32:50  jlang
// Starter Code and solution for assignment 3
//
// ==========================================================================
#version 330 core

layout (location=0) in vec4 position;
layout (location=1) in vec3 normal;
layout (location=2) in vec3 color;

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
	
  // assume Modelview matrix has no non-uniform scaling or shearing 
  normalFrag = inverse(transpose(mat3(ModelViewMatrix))) * normal;

  colorVertFrag = vec4( color, 1.0);		

  gl_Position = ProjectionMatrix * posVec;
}
