#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inUV;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec3 outUVW;

layout (binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 model;
	mat4 view;
} ubo;

void main() 
{
	vec4 pos = ubo.projection * ubo.view * vec4(inPos, 1.0f);
	gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);
	outUVW = vec3(inPos.x, inPos.y, -inPos.z);
	// Convert cubemap coordinates into Vulkan coordinate space
	// outUVW.xy *= -1.0;
}