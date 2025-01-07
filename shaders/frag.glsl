#version 450

layout(binding = 0) uniform sampler2DArray inTex[];

layout(location = 0) in vec2 inTexCoord;
layout(location = 1) flat in vec4 inWhiteColor;
layout(location = 2) flat in float inWhiteDepth;
layout(location = 3) flat in vec4 inBlackColor;
layout(location = 4) flat in float inBlackDepth;
layout(location = 5) flat in uvec2 inTexInfo;

layout(location = 0) out vec4 outColor;

void
main()
{
	outColor = texture(inTex[inTexInfo[0]], vec3(inTexCoord, inTexInfo[1]));

	if(outColor == vec4(0.0, 0.0, 0.0, 1.0))
	{
		outColor = inBlackColor;
		gl_FragDepth = inBlackDepth;
	}
	else
	{
		outColor *= inWhiteColor;
		gl_FragDepth = inWhiteDepth;
	}

	if(outColor.w == 0.0)
	{
		discard;
	}
}
