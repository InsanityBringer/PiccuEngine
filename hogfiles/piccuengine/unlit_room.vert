#version 330 core

layout(std140) uniform CommonBlock
{
	mat4 projection;
	mat4 modelview;
} commons;

layout(std140) uniform RoomBlock
{
	vec4 fog_color;
	float fog_distance;
	float brightness;
	int not_in_room;
	vec4 fog_plane;
} room;

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 4) in vec2 uv;

out vec2 outuv;
out float outlight;
out float outalpha;

void main()
{
	vec4 temp = commons.modelview * vec4(position, 1.0);
	gl_Position = commons.projection * temp;
	outuv = uv;
	outlight = room.brightness;
	outalpha = color.a;
}
