#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

// Output vertex attributes (to the fragment shader)
out vec2 fragTexCoord;
out vec4 fragColor;

uniform mat4 mvp;  // Model-View-Projection matrix

void main()
{
    // Pass the texture coordinates and vertex color to the fragment shader
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    // Transform the vertex position
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
