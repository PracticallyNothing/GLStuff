@vert
#version 330

in vec3 pos;
out vec3 fPos;
out vec3 fNorm;
uniform mat4 VP;
uniform mat4 model;
uniform float time;

uniform sampler2D waveHeight;

// Thank you,
// https://thebookofshaders.com/10/
float random (in vec2 st, in float scale) {
    return fract(sin(dot(floor(st.xy * scale), vec2(12.9898,78.233)))* 43758.5453123);
}

void main() {
	float speed  = 2;
	float spread = 3;
	float height = 0.3;

	vec4 p = model * vec4(pos,1);

	float x = p.x/spread;
	float t = time*speed;
	float h = height;

	const float pi = 3.1415926;

	float yMod = 0;

	if(mod(x-t, 2*pi) <= pi) {
		if(cos(x-t) >= 0) {
			yMod = abs(sin(x-t)*h);
		} else {
			yMod = abs(pow(sin(x-t), 21)*h);
		}
	}

	p.y += yMod * 2 * texture(waveHeight, vec2(p.z/2, x*0.8-t)/100).r;

	gl_Position = VP * p;
	fPos = p.xyz;
	fNorm = normalize(vec3(random(vec2(x, p.z), 10), 1, random(vec2(x, p.z), 10)));
}
@@

@frag
#version 330

in vec3 fPos;
in vec3 fNorm;
out vec4 fColor;

uniform float time;
uniform sampler2D noise;
uniform sampler2D foam;
uniform sampler2D waveHeight;

uniform vec3 sunDir = normalize(vec3(-1, -1, -1));

// Thank you,
// https://thebookofshaders.com/10/
float random (in vec2 st, in float scale) {
    return fract(sin(dot(floor(st.xy * scale), vec2(12.9898,78.233)))* 43758.5453123);
}

void main() {
	const float speed  = 2;
	const float spread = 3;
	const float height = 0.3;

	float x = fPos.x/spread;
	float t = time*speed;
	float h = height;

	const float pi = 3.1415926;

	const vec4 baseColor = vec4(0.1, 0.25, 1, 1);

	//if(mod((fPos.x+spread)/spread-t, 2*pi) <= pi || mod((fPos.x-spread)/spread-t, 2*pi) <= pi) {
		fColor = baseColor 
		         + max(vec4(0,0,0,1), 
				       texture(foam, vec2(fPos.x-t/5, fPos.z)/20) 
					   * (pow(fPos.y,0.2) - 1.5 + height - sin((fPos.x - 12) / spread - time * speed)));
		fColor += baseColor * 0.5 * (texture(foam, vec2(fPos.x-t/5, fPos.z-sin(t))/20));
	//} else {
		//fColor = baseColor;
	//}

	//fColor += 0.1 * vec4(1,1,1,1) * dot(-sunDir, fNorm);

	//fColor = vec4(fNorm,1);
}
@@
