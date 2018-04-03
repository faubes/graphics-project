// ==========================================================================
// $Id: material.fs,v 1.2 2016/03/12 03:39:35 jlang Exp $
// Draw mesh with matrial array and indices
// Based on lit_boxes Revision 1.2  2014/10/18 14:13:57  jlang
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
// Creator: jlang (Jochen Lang)
// Email:   jlang@eecs.uottawa.ca
// ==========================================================================
// $Log: material.fs,v $
// Revision 1.2  2016/03/12 03:39:35  jlang
// Fixed spot light
//
// Revision 1.1  2016/03/06 22:32:50  jlang
// Starter Code and solution for assignment 3
//
// ==========================================================================
#version 330 core

in vec3 normalFrag;
in vec3 eyeFrag;
in vec3 lightFrag;
flat in int materialIndex;

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

struct Material {
  vec4 emissive;
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float shininess;
};

uniform samplerCube tex;

uniform LightSource lights[1];

layout (std140) uniform MaterialBlock {
  uniform Material materials[5];
};


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
  vec4 ambient = materials[materialIndex].ambient * lights[0].ambient;

  // diffuse term
  float dotNL = max(0.0,dot(NVec,LVec));
  vec4 diffuse = materials[materialIndex].diffuse * lights[0].diffuse * dotNL;

  vec3 HVec = normalize(LVec+EVec);
  float dotNH = max(0.0,dot(NVec,HVec));
  vec4 specular = materials[materialIndex].specular * lights[0].specular
       * pow(dotNH,materials[materialIndex].shininess);

  // spot light
  float spot_attenuation = 1.0;
  float dotSV = dot(-LVec,normalize(lights[0].spot_direction));
  if ( dotSV < cos(radians(lights[0].spot_cutoff))) {
    spot_attenuation = 0.0;
  } else {
    spot_attenuation = pow(dotSV,lights[0].spot_exponent);
  }

  vec3 tc = reflect(eyeFrag, NVec);

  // color
  color = (ambient +
  	 attenuation * spot_attenuation * (diffuse + specular)) * texture(tex, tc);
  // color = vec4(LVec,1.0);
}
