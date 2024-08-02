#version 330 core

layout(std140) uniform CommonBlock
{
	mat4 projection;
	mat4 modelview;
} commons;

layout(location = 0) in vec3 position;
layout(location = 4) in vec2 uv;
layout(location = 5) in vec2 uv2;

out vec2 outuv;
out vec2 outuv2;

void main()
{
	vec4 temp = commons.modelview * vec4(position, 1.0);
	gl_Position = commons.projection * vec4(temp.xyz, temp.w);
	outuv = uv;
	outuv2 = uv2;
}
