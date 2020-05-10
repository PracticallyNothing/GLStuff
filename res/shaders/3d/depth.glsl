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

out vec4 fColor;

uniform vec2 screenSize = vec2(1920, 1080);

void main() {
	fColor = vec4(
		gl_FragCoord.x / screenSize.x,
		gl_FragCoord.y / screenSize.y,
		gl_FragCoord.z,
		1
	);
}
@@
