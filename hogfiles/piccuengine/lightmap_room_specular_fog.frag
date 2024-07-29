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
in vec3 outpos;
in vec3 outnormal;
flat in vec3[4] outlightpos;
in float outlight;
flat in vec4 outplane;

out vec4 color;

void main()
{
	const float[4] weights = float[4](1.0, 0.66, 0.33, 0.25);
	vec4 basecolor = texture(colortexture, outuv);
	vec4 lmcolor = texture(lightmaptexture, outuv2);
	vec4 tempcolor = vec4(basecolor.rgb * lmcolor.rgb, 1.0);
	
	vec3 pos = normalize(-outpos);
	vec3 normal = normalize(outnormal);
	for (int i = 0; i < specular_data.num_specular; i++)
	{
		vec3 lightvec = normalize(outlightpos[i] - outpos);
		vec3 reflectlight = reflect(-lightvec, normal);
		
		tempcolor += vec4(pow(max(dot(reflectlight, pos), 0.0), specular_data.exponent) * specular_data.speculars[i].color.xyz, 0.0) * lmcolor * specular_data.strength * basecolor.a * weights[i];
	}
	
	float mag = 0;
	if (room.not_in_room != 0)
	{
		float dist = dot(outpt, outplane.xyz) + outplane.w;
		if (dist > 0)
		{
			float t = outplane.w / (outplane.w - dist);
			vec3 portal_point = outpt * t;
			
			mag = max(0, (outpt.z - portal_point.z));
		}
	}
	else
	{
		mag = outpos.z / room.fog_distance;
	}
	color = mix(tempcolor, vec4(room.fog_color.xyz, tempcolor.z), clamp(mag, 0, 1)) * vec4(outlight, outlight, outlight, 1.0); 
}
