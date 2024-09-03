#version 330 core

uniform sampler2D colortexture;
uniform sampler2D lightmaptexture;

struct specular
{
	vec4 bright_center;
	vec4 color;
};

layout(std140) uniform SpecularBlock
{
	int num_specular;
	int exponent;
	float strength;
	specular speculars[4];
} specular_data;

in vec2 outuv;
in vec2 outuv2;
in vec3 outpos;
in vec3 outnormal;
flat in vec3[4] outlightpos;

out vec4 color;

void main()
{
	const float[4] weights = float[4](1.0, 0.66, 0.33, 0.25);
	vec4 basecolor = texture(colortexture, outuv);
	vec4 lmcolor = texture(lightmaptexture, outuv2);
	color = vec4(basecolor.rgb * lmcolor.rgb, 1.0);
	
	vec3 pos = normalize(outpos);
	vec3 normal = normalize(outnormal);
	for (int i = 0; i < specular_data.num_specular; i++)
	{
		vec3 lightvec = normalize(outlightpos[i] + outpos);
		vec3 reflectlight = reflect(-lightvec, normal);
		
		color += vec4(pow(max(dot(reflectlight, pos), 0.0), specular_data.exponent) * specular_data.speculars[i].color.xyz, 0.0) * lmcolor * specular_data.strength * basecolor.a * weights[i];
	}
}
