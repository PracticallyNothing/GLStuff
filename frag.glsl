#version 330
uniform sampler2D fbo_color;

in vec2 fUV;
out vec4 color;

void main() { 
	ivec2 size = textureSize(fbo_color, 0);
	float xPixelSize = 2.0 / size.x;

	vec2 fUV_left = fUV - vec2(10*xPixelSize, 0);
	color = mix(
		texture(fbo_color, fUV_left, 0), 
		texture(fbo_color, fUV), 
		0.5
	);
}
