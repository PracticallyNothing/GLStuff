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

out vec4 fcolor;

uniform vec3 color;

void main() {
	fcolor = vec4(0, 1, 0, 1); 
}
@@
