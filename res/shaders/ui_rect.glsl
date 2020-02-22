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

uniform int type; // 0 for color, 1 for image
uniform vec4 color;
uniform sampler2D img;

void main() {
	if(type == 0)
		fColor = color;
	else if(type == 1)
		fColor = texture(img, fUV);
}
@@
