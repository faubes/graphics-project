// https://learnopengl.com/Advanced-OpenGL/Cubemaps
#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

void main()
{
    TexCoords = -aPos;
    gl_Position = vec4(aPos, 1.0);
}
