#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 atexCoords;
layout(location = 3) in vec3 aTangent;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrixDirLight;
uniform mat4 lightSpaceMatrixSpotLight;


out vec3 normal;
out vec3 fragPos;
out vec2 texCoords;

out mat3 TBN;
out vec4 fragPosLightSpaceDirLight;
out vec4 fragPosLightSpaceSpotLight;

void main()
{

    vec3 T = normalize(vec3(model * vec4(aTangent, 0.0f)));
    vec3 N = normalize(vec3(model * vec4(aNormal, 0.0f)));
    //reorthogonalize T
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);

    fragPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * vec4(fragPos, 1.0);
    fragPosLightSpaceDirLight = lightSpaceMatrixDirLight * vec4(fragPos, 1.0);
    fragPosLightSpaceSpotLight = lightSpaceMatrixSpotLight * vec4(fragPos, 1.0);
    normal = mat3(transpose(inverse(model))) * aNormal;
    texCoords = atexCoords;
}
