#version 330
in vec2 pos;
in vec2 UV;

out vec2 fUV;

void main() { 
	gl_Position = vec4(pos, 0, 1); 
	fUV = UV;
}
