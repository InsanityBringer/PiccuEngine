#version 330 core

uniform sampler2D colortexture;
uniform sampler2D lightmaptexture;

layout(std140) uniform RoomBlock
{
	vec4 fog_color;
	float fog_distance;
	float brightness;
	int not_in_room;
	vec4 fog_plane;
} room;

in vec2 outuv;
in vec2 outuv2;
in vec3 outpt;
in float outlight;
in float outalpha;
flat in vec4 outplane;

out vec4 color;

void main()
{
	vec4 basecolor = texture(colortexture, outuv);
	
	float mag = 0;
	if (room.not_in_room != 0)
	{
		//alternate way of doing this suggested by Jeff Graw
		float dist = dot(outpt, outplane.xyz) + outplane.w;
		float t = outplane.w / (outplane.w - dist);
		vec3 portal_point = outpt * t; 
		mag = step(0, dist) * max(0, -(outpt.z - portal_point.z));
	}
	else
	{
		mag = -outpt.z;
	}
	
	color = mix(vec4(basecolor.rgb, 1.0), vec4(room.fog_color.rgb, 1.0f), clamp(mag / room.fog_distance, 0, 1)) * vec4(outlight, outlight, outlight, 1.0);
	color.a *= outalpha;
}
