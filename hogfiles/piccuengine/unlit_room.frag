#version 330 core

uniform sampler2D colortexture;
uniform sampler2D lightmaptexture;

in vec2 outuv;
in vec2 outuv2;
in float outlight;
in float outalpha;

out vec4 color;

void main()
{
	vec4 basecolor = texture(colortexture, outuv);
	color = vec4(basecolor.rgb * outlight, basecolor.a * outalpha);
}
