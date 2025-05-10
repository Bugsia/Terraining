#version 330

// Input vertex attributes (from the vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;  // The texture bound to texture unit 0
uniform vec4 colDiffuse;      // The diffuse color passed from Raylib

// Output fragment color
out vec4 finalColor;

void main()
{
    // Force everything to be red
    finalColor = vec4(1.0, 0.0, 0.0, 1.0);
}
