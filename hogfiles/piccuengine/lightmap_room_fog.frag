#version 330 core

uniform sampler2D colortexture;
uniform sampler2D lightmaptexture;

layout(std140) uniform RoomBlock
{
	vec4 fog_color;
	float fog_distance;
	float fog_modifier;
	float brightness;
} room;

in vec2 outuv;
in vec2 outuv2;
in vec3 outpt;
in float outlight;

out vec4 color;

void main()
{
	vec4 basecolor = texture(colortexture, outuv);
	vec4 lmcolor = texture(lightmaptexture, outuv2);
	color = mix(vec4(basecolor.rgb * lmcolor.rgb, 1.0), vec4(room.fog_color.rgb, 1.0f), clamp(outpt.z / room.fog_distance, 0, 1)) * vec4(outlight, outlight, outlight, 1.0);
}
