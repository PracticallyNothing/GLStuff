@vert
#version 330

in vec3 pos;
in vec2 uv;

out vec2 fUV;

uniform mat4 MVP;

void main() {
	gl_Position = MVP * vec4(pos, 1);
	fUV = uv;
}
@@

@frag
#version 330

in vec2 fUV;
out vec4 fColor;

uniform sampler2D diffuse;

void main() {
	fColor = texture(diffuse, fUV);
}
@@
