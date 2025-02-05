#version 330 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
	vec3 color = vec3(1.0f);
	color = textureLod(skybox, TexCoords, 1.2).rgb;
    FragColor = texture(skybox, TexCoords);
	FragColor = vec4(color, 1.0);
	float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
	if (brightness > 1.0) {
		BrightColor = vec4(FragColor.rgb, 1.0);
	}
	else {
		BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
}
