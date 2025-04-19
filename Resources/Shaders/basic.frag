#version 330 core

out vec4 FragColor;

in vec2 TexCoord; // Texture coordinates from the vertex shader

uniform sampler2D texture1; // The texture sampler

void main()
{
    FragColor =  texture(texture1, TexCoord); // Sample the texture using the texture coordinates
}