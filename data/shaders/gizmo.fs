#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Output fragment color
out vec4 finalColor;

// Uniform inputs
uniform sampler2D texture0;  // Diffuse map texture
uniform vec4 colDiffuse;     // Diffuse color

void main()
{
    // Sample the texture using the texture coordinates
    vec4 texelColor = texture(texture0, fragTexCoord);
    
    // Combine the texture color with the diffuse color
    finalColor = texelColor * colDiffuse;
}