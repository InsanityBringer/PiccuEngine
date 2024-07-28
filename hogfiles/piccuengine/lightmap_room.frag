#version 330 core

uniform sampler2D colortexture;
uniform sampler2D lightmaptexture;

in vec2 outuv;
in vec2 outuv2;
in float outlight;

out vec4 color;

void main()
{
	vec4 basecolor = texture(colortexture, outuv);
	vec4 lmcolor = texture(lightmaptexture, outuv2);
	color = vec4(basecolor.rgb * lmcolor.rgb * outlight, 1.0);
}
