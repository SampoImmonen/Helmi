#version 330 core

layout(location = 0) out vec4 FragColor;

in vec2 TexCoords;
uniform vec3 color = vec3(1.0);

void main()
{
	//vec3 c = vec3(1.0, 0.0, 0.0);
	FragColor = vec4(color, 1.0);
}

