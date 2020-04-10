@vert
#version 330

in vec2 pos;
in vec4 colorUV;

out vec4 fColorUV;

uniform mat4 view;
uniform mat4 proj;

void main() {
	gl_Position = proj * view * vec4(pos, 0, 1);
	fColorUV = colorUV;
}
@@

@frag
#version 330

in vec4 fColorUV;

out vec4 fColor;

uniform int type; // 0 for color, 1 for image
uniform sampler2D img;

void main() {
	if(type == 0)
		fColor = fColorUV;
	else if(type == 1)
		fColor = texture(img, fColorUV.xy);
}
@@
