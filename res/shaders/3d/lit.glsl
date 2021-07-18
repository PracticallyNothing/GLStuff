@vert
#version 330

in vec3 pos;
in vec2 uv;
in vec3 norm;

out vec3 fPos;
out vec2 fUV;
out vec3 fNorm;

uniform mat4 MVP;

void main() {
    gl_PointSize = 20;
	gl_Position = MVP * vec4(pos, 1);

	fPos = (MVP * vec4(pos, 1)).xyz;
	fUV = uv;
	fNorm = normalize(MVP * vec4(norm, 1)).xyz;
}
@@

@frag
#version 330

in vec3 fPos;
in vec2 fUV;
in vec3 fNorm;

out vec4 fColor;

struct Material {
	vec3 DiffuseColor;
	
	bool HasDiffuseTexture;
	bool HasNormalTexture;

	sampler2D DiffuseTexture;
	sampler2D NormalTexture;
};

// NOTE: These are point lights.
struct Light {
	vec3 Position;
	vec3 Radius;
	vec3 Color;
};

uniform bool lightEnabled = 1;
uniform Material Material;

uniform vec3 ambientLight = vec3(0.05, 0.05, 0.05);
uniform int NumEnabledLights = 0;
uniform Light Lights[8];

void main() {
	if(!lightEnabled) {
		fColor = color;
		return;
	}

	vec3 basePixel;
	if(Material.HasDiffuseTexture)
		basePixel = texture(Material.DiffuseTexture);
	else
		basePixel = Material.DiffuseColor;
	
	for(int i = 0; i < NumEnabledLights; i++) {
		vec3 v = Lights[i].Position - fPos;
		float amountLit = max(0.0, dot(fNorm, v));
	}
}
@@
