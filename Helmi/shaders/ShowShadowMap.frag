#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;

const float near_plane = 0.1;
const float far_plane = 100.0;

float LinearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0;
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{
    float depth = texture(screenTexture, TexCoords).r;
    FragColor = vec4(vec3(LinearizeDepth(depth)/far_plane), 1.0);
    //FragColor = vec4(vec3(depth), 1.0);
}
