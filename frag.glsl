#version 330

uniform sampler2D fbo_color;
uniform sampler2DShadow fbo_depth;

in vec2 fUV;
out vec4 color;

void main() { 
	color = texture(fbo_depth, fUV);
}
