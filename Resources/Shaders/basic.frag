#version 330 core

out vec4 FragColor;

in vec2 TexCoord; // Texture coordinates from the vertex shader
in vec4 FragPos;

uniform sampler2D texture1; // The texture sampler
uniform float TexOpacity;
uniform vec2 screenDimensions;


void main()
{    
    vec4 texColor = vec4(texture(texture1, TexCoord).xyz, TexOpacity);


    // Calculate the center of the screen in window coordinates
    vec2 center = screenDimensions / 2.0;

    // Calculate the distance from the fragment to the center
    float dist = distance(center, gl_FragCoord.xy);

    vec2 d = abs(gl_FragCoord.xy - center);

    if ((d.x < screenDimensions.x * .02 && d.y < screenDimensions.y * .005) || (d.x < screenDimensions.y * .005 && d.y < screenDimensions.x * .02)) 
    {
        FragColor = vec4(1, 1, 1, 1);
    }
    else
    {
        FragColor = texColor;
    }
}