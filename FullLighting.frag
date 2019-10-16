#version 330

// Thank you,
// https://learnopengl.com/Lighting/Light-casters

in vec3 fPos;
in vec2 fUV;
in vec3 fNorm;

out vec4 color;

uniform vec3 viewPos;

/*
uniform Material {
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
	float SpecularExponent;

	bool HasAmbientTex = false;
	bool HasDiffuseTex = false;
	bool HasSpecularTex = false;

	sampler2D AmbientTex;
	sampler2D DiffuseTex;
	sampler2D SpecularTex;
} mat;

uniform Lights {
	bool SunEnabled = false;
	Sun Sun;

	int NumPointLights = 0;
	PointLight Points [8];

	int NumSpotlights  = 0;
	Spotlight Spots  [8];
} lights;
 */

uniform vec3 mat_ambient = vec3(1, 0, 0);
uniform vec3 mat_diffuse = vec3(0, 1, 0);
uniform vec3 mat_specular = vec3(0, 0, 1);
uniform float mat_specularExponent = 32;

uniform bool mat_tex_hasAmbient = false;
uniform bool mat_tex_hasDiffuse = false;
uniform bool mat_tex_hasSpecular = false;
uniform sampler2D mat_tex_ambient;
uniform sampler2D mat_tex_diffuse;
uniform sampler2D mat_tex_specular;

struct Sun {
	vec3 Direction;

	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
};
uniform bool sun_enabled = false;
uniform Sun sun;

struct PointLight {
	vec3 Position;

	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;

	float ConstantAttenuation;
	float LinearAttenuation;
	float QuadraticAttenuation;
};
uniform int pointLights_numEnabled = 0;
uniform PointLight pointLights[8];

struct Spotlight {
	vec3 Position;
	vec3 Direction;

	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;

	float CutoffAngle_Inner;
	float CutoffAngle_Outer;
};
uniform int spotlights_numEnabled = 0;
uniform Spotlight spotlights[8];

float CalcSpecular(vec3 lightDir) {
	vec3 viewDir = normalize(viewPos - fPos);
	vec3 reflectDir = reflect(-lightDir, fNorm);
	return pow(max(dot(viewDir, reflectDir), 0), mat_specularExponent);
}

void main() {
	vec3 ambient = vec3(0);
	vec3 diffuse = vec3(0);
	vec3 specular = vec3(0);

	if(sun_enabled) {
		float ambientStr = 0.01;
		float diffuseStr = max(dot(-sun.Direction, fNorm), 0);
		float specularStr = 0.5 * CalcSpecular(sun.Direction);

		ambient  += sun.Ambient * mat_ambient * ambientStr;
		diffuse  += sun.Diffuse * mat_diffuse * diffuseStr;
		specular += sun.Specular * mat_specular * specularStr;
	}

	for(int i = 0; i < pointLights_numEnabled; i++) {
		PointLight l = pointLights[i];
		vec3 lDir = normalize(fPos - l.Position);

		float dist = length(lDir);
		float atten = 1.0 / (l.ConstantAttenuation +
		                     l.LinearAttenuation * dist +
		                     l.QuadraticAttenuation * pow(dist, 2));

		float ambientStr = 0.01;
		float diffuseStr = max(dot(-lDir, fNorm), 0);
		float specularStr = 0.5 * CalcSpecular(lDir);

		ambient += l.Ambient * mat_ambient * ambientStr * atten;
		diffuse += l.Diffuse * mat_diffuse * diffuseStr * atten;
		specular += l.Specular * mat_specular * specularStr * atten;
	}

	for(int i = 0; i < spotlights_numEnabled; i++) {
		Spotlight l = spotlights[i];

		vec3 FragToLight = l.Position - fPos;
		float theta = dot(normalize(FragToLight), normalize(-l.Direction));

		if(theta > l.CutoffAngle_Outer) {
			float epsilon = l.CutoffAngle_Inner - l.CutoffAngle_Outer;
			float intensity = clamp((theta - l.CutoffAngle_Outer) / epsilon, 0.0, 1.0);
			float specularStr = CalcSpecular(l.Direction);

			diffuse += l.Diffuse * mat_diffuse * intensity;
			specular += l.Specular * mat_specular * specularStr * intensity;
		} else {
			float ambientStr = 0.01;
			ambient += l.Ambient * mat_ambient * ambientStr;
		}
	}

	color = vec4(ambient + diffuse + specular, 1);
}
