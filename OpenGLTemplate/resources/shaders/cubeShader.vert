#version 400 core

// Structure for matrices
uniform struct Matrices
{
	mat4 projMatrix;
	mat4 modelViewMatrix;
	mat3 normalMatrix;
} matrices;

// Layout of vertex attributes in VBO
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inCoord;
layout (location = 2) in vec3 inNormal;

// Outputs to fragment shader
out vec3 vEyeNorm;       // Normal in eye space
out vec4 vEyePosition;   // Position in eye space
out vec2 vTexCoord;      // Texture coordinate

void main()
{
	// Transform position to clip space
	gl_Position = matrices.projMatrix * matrices.modelViewMatrix * vec4(inPosition, 1.0f);

	// Pass eye-space values to the fragment shader for per-fragment lighting
	vEyeNorm     = normalize(matrices.normalMatrix * inNormal);
	vEyePosition = matrices.modelViewMatrix * vec4(inPosition, 1.0f);

	// Pass through texture coordinate
	vTexCoord = inCoord;
}