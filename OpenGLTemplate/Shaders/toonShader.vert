#version 400 core

uniform struct Matrices {
    mat4 projMatrix;
    mat4 modelViewMatrix;
    mat3 normalMatrix;
} matrices;

struct LightInfo {
    vec4 position;
    vec3 La;
    vec3 Ld;
    vec3 Ls;
};
uniform LightInfo light1;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inCoord;
layout (location = 2) in vec3 inNormal;

out vec2  vTexCoord;
out vec3  vEyeNorm;
out vec4  vEyePosition;

void main()
{
    vEyePosition = matrices.modelViewMatrix * vec4(inPosition, 1.0f);
    vEyeNorm     = normalize(matrices.normalMatrix * inNormal);
    vTexCoord    = inCoord;
    gl_Position  = matrices.projMatrix * vEyePosition;
}
