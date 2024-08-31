//the preprocessor definitions, including #version, are automatically applied
//at compile time for the generic shader

layout(std140) uniform TerrainFogBlock
{
	vec4 color;
	float start_dist;
	float end_dist;
} fog;

#if defined(USE_TEXTURING)
uniform sampler2D colortexture;
#if defined(USE_LIGHTMAP)
uniform sampler2D lightmaptexture;
#endif
#endif

in vec4 outcolor;
#if defined(USE_TEXTURING)
in vec3 outuv;
#if defined(USE_LIGHTMAP)
in vec3 outuv2;
#endif
#endif
#if defined(USE_FOG)
in vec3 outpt;
#endif

out vec4 color;

void main()
{
	#if defined(USE_SPECULAR)
		color = vec4(outcolor.rgb, texture(colortexture, outuv.xy / outuv.z).a * outcolor.a);
	#elif defined(USE_TEXTURING) && defined(USE_LIGHTMAP)
		color = texture(colortexture, outuv.xy / outuv.z) * texture(lightmaptexture, outuv2.xy / outuv2.z) * outcolor;
	#elif defined(USE_TEXTURING)
		color = texture(colortexture, outuv.xy / outuv.z) * outcolor;
	#else
		color = outcolor;
	#endif
	
	#if defined(USE_FOG)
		float mag = clamp((-outpt.z - fog.start_dist) / (fog.end_dist - fog.start_dist), 0, 1);
		color = vec4(mix(color.rgb, fog.color.rgb, mag), color.a);
	#endif
}
