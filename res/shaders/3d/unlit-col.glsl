@vert
#version 330

in vec3 pos;
uniform mat4 MVP;

void main() {
	gl_Position = MVP * vec4(pos, 1);
}
@@

@frag
#version 330

in vec2 fUV;
out vec4 color;

uniform sampler2D diffuse;

void main() {
	color = texture(diffuse, fUV);
}
@@
