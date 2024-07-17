#version 330 core

layout(std140) uniform CommonBlock
{
	mat4 projection;
	mat4 modelview;
} commons;

struct specular
{
	vec4 bright_center;
	vec4 color;
};

layout(std140) uniform SpecularBlock
{
	int num_specular;
	int exponent;
	specular speculars[4];
} specular_data;


layout(location = 0) in vec3 position;
layout(location = 2) in vec3 normal;
layout(location = 4) in vec2 uv;
layout(location = 5) in vec2 uv2;

out vec2 outuv;
out vec2 outuv2;
out vec3 outpos;
out vec3 outnormal;
flat out vec3[4] outlightpos;

void main()
{
	vec4 temp = commons.modelview * vec4(position, 1.0);
	gl_Position = commons.projection * vec4(temp.xy, -temp.z, temp.w);
	outuv = uv;
	outuv2 = uv2;
	outpos = -temp.xyz;
	outnormal = mat3(commons.modelview) * normal;
	
	//Need to transform the light positions too..
	for (int i = 0; i < specular_data.num_specular; i++)
	{
		outlightpos[i] = (commons.modelview * specular_data.speculars[i].bright_center).xyz;
	}
}
 