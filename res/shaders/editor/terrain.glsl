@vert
#version 330

in vec3 pos;
in vec2 uv;

out vec2 fUV;

uniform mat4 MVP;

void main() {
	gl_Position = MVP * vec4(pos, 1);
	fUV = uv;
}
@@

@frag
#version 330

in vec2 fUV;

out vec4 fColor;

void main() {
	float thickness = 0.10;

	if((fUV.x > -1 + thickness && fUV.x < 1 - thickness) && 
	   (fUV.y > -1 + thickness && fUV.y < 1 - thickness)) {
		fColor = vec4(0.3, 0.3, 0.3, 1);
	} else {
		fColor = vec4(0.5, 0.5, 0.5, 1);
	}
}
@@
