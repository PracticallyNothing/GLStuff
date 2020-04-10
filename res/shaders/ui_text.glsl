@vert
#version 330

in vec2 pos;
in vec2 uv;

out vec2 fUV;

uniform mat4 view;
uniform mat4 proj;

void main() {
	gl_Position = proj * view * vec4(pos, 0, 1);
	fUV = uv;
}
@@

@frag
#version 330

in vec2 fUV;

out vec4 fColor;

uniform vec4 fg;
uniform vec4 bg;
uniform sampler2D font;

void main() {
	fColor = mix(bg, fg, texture(font, fUV).a);
}
@@
