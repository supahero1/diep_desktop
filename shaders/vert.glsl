#version 450

layout(push_constant) uniform Constants
{
	float width;
	float height;
}
consts;

layout(location = 0) in vec2 inVertexPosition;
layout(location = 1) in vec2 inTexCoords;

layout(location = 2) in vec2 inPosition;
layout(location = 3) in vec2 inDimensions;
layout(location = 4) in vec4 inWhiteColor;
layout(location = 5) in float inWhiteDepth;
layout(location = 6) in vec4 inBlackColor;
layout(location = 7) in float inBlackDepth;
layout(location = 8) in uvec2 inTexInfo;
layout(location = 9) in float inRotation;
layout(location = 10) in vec2 inTexScale;
layout(location = 11) in vec2 inTexOffset;

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) flat out vec4 outWhiteColor;
layout(location = 2) flat out float outWhiteDepth;
layout(location = 3) flat out vec4 outBlackColor;
layout(location = 4) flat out float outBlackDepth;
layout(location = 5) flat out uvec2 outTexInfo;

void
main()
{
	mat3 scale;
	scale[0].xyz = vec3(2.0 / consts.width, 0.0, 0.0);
	scale[1].xyz = vec3(0.0, 2.0 / consts.height, 0.0);
	scale[2].xyz = vec3(0.0, 0.0, 1.0);

	float c = cos(inRotation);
	float s = sin(inRotation);

	mat3 rotation;
	rotation[0].xyz = vec3(c, -s, 0.0);
	rotation[1].xyz = vec3(s, c, 0.0);
	rotation[2].xyz = vec3(0.0, 0.0, 1.0);

	vec3 rotated = rotation * vec3(inVertexPosition * inDimensions, 0.0) + vec3(inPosition.xy, 0.0);

	gl_Position = vec4((scale * rotated).xy, 0.0, 1.0);

	outTexCoord = ((inTexCoords - vec2(0.5)) * inTexScale * 0.5) + inTexOffset;
	outWhiteColor = inWhiteColor;
	outWhiteDepth = inWhiteDepth;
	outBlackColor = inBlackColor;
	outBlackDepth = inBlackDepth;
	outTexInfo = inTexInfo;
}
