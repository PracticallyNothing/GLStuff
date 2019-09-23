#version 330

uniform mat4 persp;
uniform mat4 view;
uniform mat4 model;

in vec3 pos;

void main() { 
	gl_Position = persp * model * vec4(pos, 1);
}
