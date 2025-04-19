#version 330 core

layout(location = 0) in vec3 aPos;       // Vertex position
layout(location = 1) in vec3 aNormal;    // Vertex normal
layout(location = 2) in vec2 aTexCoord;  // Vertex texture coordinate

out vec2 TexCoord;       // Pass texture coordinates to the fragment shader

uniform mat4 projection; // Projection matrix
uniform mat4 view;       // View matrix

void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
    TexCoord = aTexCoord; // Pass texture coordinates to the fragment shader
}