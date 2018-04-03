// ==========================================================================
// $Id: color.fs,v 1.2 2016/03/12 03:39:35 jlang Exp $
// Simple lighting with color as material
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
// $Log: color.fs,v $
// Revision 1.2  2016/03/12 03:39:35  jlang
// Fixed spot light
//
// Revision 1.1  2016/03/06 22:32:50  jlang
// Starter Code and solution for assignment 3
//
// ==========================================================================
#version 330 core

in vec4 colorVertFrag; 
in vec3 normalFrag; 
in vec3 eyeFrag; 
in vec3 lightFrag; 

out vec4 color;

struct LightSource {
  vec4 ambient;  
  vec4 diffuse;
  vec4 specular;
  // spot light
  // v is the vector to the vertex
  // if dir*v < cos(cutoff) then (dir * v)^N 
  vec3 spot_direction;
  float spot_exponent;
  float spot_cutoff;
  // attentuation 1/(k_c + k_l r + k_q r^2) 
  // r is the distance of a vertex from the light source
  float constant_attenuation;
  float linear_attenuation;
  float quadratic_attenuation;
};
 
uniform LightSource lights[2];


void main() {
  vec3 NVec = normalize(normalFrag);
  vec3 LVec = normalize(lightFrag);
  vec3 EVec = normalize(eyeFrag);

  float distanceLight = length(lightFrag.xyz);

  float attenuation = 1.0 / 
    (lights[0].constant_attenuation +
     lights[0].linear_attenuation * distanceLight +
     lights[0].quadratic_attenuation * distanceLight * distanceLight);

  // ambient term
  vec4 ambient =  colorVertFrag * lights[0].ambient;

  // diffuse term
  float dotNL = max(0.0,dot(NVec,LVec));
  vec4 diffuse = colorVertFrag * lights[0].diffuse * dotNL;

  // spot light
  float spot_attenuation = 1.0;
  float dotSV = dot(-LVec,normalize(lights[0].spot_direction));
  if ( dotSV < cos(radians(lights[0].spot_cutoff))) {
    spot_attenuation = 0.0;
  } else {
    spot_attenuation = pow(dotSV,lights[0].spot_exponent);
  }

  // color
  color = ambient + 
    attenuation * spot_attenuation * diffuse;

  //  color = colorVertFrag;
}
