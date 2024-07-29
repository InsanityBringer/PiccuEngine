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
flat in vec4 outplane;

out vec4 color;

void main()
{
	vec4 basecolor = texture(colortexture, outuv);
	vec4 lmcolor = texture(lightmaptexture, outuv2);
	
	float mag;
	if (room.not_in_room != 0)
	{
		float dist = dot(outpt, outplane.xyz) + outplane.w;
		
		float t = outplane.w / (outplane.w - dist);
		vec3 portal_point = outpt * t;
		
		mag = (outpt.z - portal_point.z) / room.fog_distance;
	}
	else
	{
		mag = outpt.z / room.fog_distance;
	}
	
	color = mix(vec4(basecolor.rgb * lmcolor.rgb, 1.0), vec4(room.fog_color.rgb, 1.0f), clamp(mag, 0, 1)) * vec4(outlight, outlight, outlight, 1.0);
}
