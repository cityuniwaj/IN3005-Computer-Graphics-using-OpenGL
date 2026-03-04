#version 400 core

// Inputs from vertex shader
in vec3 vEyeNorm;
in vec4 vEyePosition;
in vec2 vTexCoord;

// Output colour
out vec4 vOutputColour;

// Light and material structs
struct LightInfo
{
	vec4 position;
	vec3 La;
	vec3 Ld;
	vec3 Ls;
};

struct MaterialInfo
{
	vec3 Ma;
	vec3 Md;
	vec3 Ms;
	float shininess;
};

uniform LightInfo light1;
uniform MaterialInfo material1;
uniform sampler2D sampler0;
uniform bool bUseTexture;

// Phong shading model — calculated per fragment
vec3 PhongModel(vec4 eyePosition, vec3 eyeNorm)
{
	vec3 s = normalize(vec3(light1.position - eyePosition));
	vec3 v = normalize(-eyePosition.xyz);
	vec3 r = reflect(-s, eyeNorm);
	vec3 n = eyeNorm;

	vec3 ambient = light1.La * material1.Ma;

	float sDotN = max(dot(s, n), 0.0f);
	vec3 diffuse = light1.Ld * material1.Md * sDotN;

	vec3 specular = vec3(0.0f);
	float eps = 0.000001f;
	if (sDotN > 0.0f)
		specular = light1.Ls * material1.Ms * pow(max(dot(r, v), 0.0f), material1.shininess + eps);

	return ambient + diffuse + specular;
}

void main()
{
	// Compute lighting per fragment
	vec3 colour = PhongModel(vEyePosition, normalize(vEyeNorm));

	if (bUseTexture)
		vOutputColour = texture(sampler0, vTexCoord) * vec4(colour, 1.0f);
	else
		vOutputColour = vec4(colour, 1.0f);
}