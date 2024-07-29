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
layout(location = 4) in vec2 uv;
layout(location = 5) in vec2 uv2;

out vec2 outuv;
out vec2 outuv2;
out vec3 outpt;
out float outlight;
flat out vec4 outplane;

void main()
{
	vec4 temp = commons.modelview * vec4(position, 1.0);
	gl_Position = commons.projection * vec4(temp.xy, -temp.z, temp.w);
	outuv = uv;
	outuv2 = uv2;
	outpt = temp.xyz;
	outlight = room.brightness;
	
	//fog plane nonsense
	//This will take the room's fog plane and translate it into view space, so that the position doesn't need to be extracted from the modelview matrix.	
	vec4 fogplane = transpose(inverse(commons.modelview)) * room.fog_plane;
	float normmag = length(fogplane.xyz);
	outplane = vec4(normalize(fogplane.xyz), fogplane.w / normmag);
}
