#version 330

in vec3 pos;
in vec2 uv;
in vec3 norm;

out vec3 fPos;
out vec2 fUV;
out vec3 fNorm;

uniform mat4 persp;
uniform mat4 view;
uniform mat4 model;

void main() {
	gl_Position = persp * view * model * vec4(pos, 1);
	fPos = (model * vec4(pos,1)).xyz;
	fUV = uv;
	// TODO: Replace this operation with uniform
	fNorm = mat3(transpose(inverse(model))) * norm;
}

