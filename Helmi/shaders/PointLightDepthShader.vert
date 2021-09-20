#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 4) in ivec4 boneIds;
layout(location = 5) in vec4 weights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];


uniform mat4 model;

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

    gl_Position = model * boneTransform*vec4(aPos, 1.0);
}
