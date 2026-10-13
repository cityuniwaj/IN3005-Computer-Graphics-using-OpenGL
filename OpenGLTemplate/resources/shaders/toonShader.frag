#version 400 core

in vec2  vTexCoord;
in vec3  vEyeNorm;
in vec4  vEyePosition;

out vec4 vOutputColour;

uniform sampler2D sampler0;
uniform bool      bUseTexture;

struct LightInfo {
    vec4 position;
    vec3 La;
    vec3 Ld;
    vec3 Ls;
};
uniform LightInfo light1;

void main()
{
    // Diffuse intensity
    vec3  s         = normalize(vec3(light1.position - vEyePosition));
    float intensity = max(dot(s, vEyeNorm), 0.0f);

    // Quantise into 4 bands (cel shading)
    float toon;
    if      (intensity > 0.85f) toon = 1.0f;
    else if (intensity > 0.55f) toon = 0.7f;
    else if (intensity > 0.25f) toon = 0.4f;
    else                        toon = 0.15f;

    vec3 colour = light1.La * 0.2f + light1.Ld * toon;

    vec4 texColour = bUseTexture ? texture(sampler0, vTexCoord) : vec4(1.0f);
    vOutputColour  = texColour * vec4(colour, 1.0f);
}
