@vert
#version 330

in vec3 pos;
uniform mat4 MVP;

void main() {
    gl_PointSize = 20;
	gl_Position = MVP * vec4(pos, 1);
}
@@

@frag
#version 330

out vec4 fColor;
uniform vec4 color;

void main() {
	fColor = color;
}
@@
