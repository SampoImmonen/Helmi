#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 atexCoords;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in ivec4 boneIds;
layout(location = 5) in vec4 weights;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrixDirLight;
uniform mat4 lightSpaceMatrixSpotLight;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

out vec3 normal;
out vec3 fragPos;
out vec2 texCoords;

out mat3 TBN;
out vec4 fragPosLightSpaceDirLight;
out vec4 fragPosLightSpaceSpotLight;

void main()
{

    mat4 boneTransform = mat4(1.0);
    if (boneIds[0] != -1) {
        boneTransform = mat4(0.0);
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            if (boneIds[i] == -1)
                continue;
            boneTransform += finalBonesMatrices[boneIds[i]] * weights[i];
        }
    }

    vec3 T = normalize(vec3(model * vec4(aTangent, 0.0f)));
    vec3 N = normalize(vec3(model * vec4(aNormal, 0.0f)));
    //reorthogonalize T
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);

    vec4 posL = boneTransform * vec4(aPos, 1.0);
    fragPos = vec3(model * posL);
    gl_Position = projection * view * vec4(fragPos, 1.0);
    fragPosLightSpaceDirLight = lightSpaceMatrixDirLight * vec4(fragPos, 1.0);
    fragPosLightSpaceSpotLight = lightSpaceMatrixSpotLight * vec4(fragPos, 1.0);
    normal = mat3(transpose(inverse(model))) * vec3(boneTransform * vec4(aNormal, 0));
    texCoords = atexCoords;
}
