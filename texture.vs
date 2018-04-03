// ==========================================================================
// $Id: texture.vs,v 1.2 2016/03/12 03:39:36 jlang Exp $
// Texturing and material indexed
// Based on lab7 Revision 1.3  2014/11/08 03:28:04  jlang
// ===================================================================
// (C)opyright:
//
//   Jochen Lang
//   EECS, University of Ottawa
//   800 King Edward Ave.
//   Ottawa, On., K1N 6N5
//   Canada.
//   http://www.eecs.uottawa.ca
//
// Creator: jlang (Jochen Lang)
// Email:   jlang@eecs.uottawa.ca
// ==========================================================================
// $Log: texture.vs,v $
// Revision 1.2  2016/03/12 03:39:36  jlang
// Fixed spot light
//
// Revision 1.1  2016/03/06 22:32:51  jlang
// Starter Code and solution for assignment 3
//
// ==========================================================================
#version 330 core

layout (location=0) in vec4 position;
layout (location=1) in vec3 normal;
layout (location=2) in vec2 texCoord;
layout (location=3) in int matIndex;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

uniform vec4 lightPosition[1];

out vec3 normalFrag; // Pass the normal to rasterization
out vec3 eyeFrag; // Pass an eye vector along
out vec3 lightFrag; // Pass a light vector along
flat out int materialIndex; // Pass the material index along as is
out vec4 texCoordFrag; // Pass the texture coordinate along

void main() {
  texCoordFrag = vec4( texCoord, 0, 1 );

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

  gl_Position = ProjectionMatrix * posVec;

  materialIndex = matIndex; // Get the material index
}
