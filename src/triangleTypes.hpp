#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include <vector>
#include <iostream>

namespace triangle
{
	using Index = uint32_t;

	enum TextureType
	{
		NONE,
		TEXTURE_TYPE_2D,
		TEXTURE_TYPE_CUBEMAP
	};

	struct MVP
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
		// glm::vec3 normal;
		glm::vec3 uv;

		// Initialized in triangleModel.cpp
		static std::vector<vk::VertexInputBindingDescription> getBindingDesciptions();
		static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions();
	};

	struct Mesh
	{
		std::vector<Vertex> vertices;
		std::vector<Index> indices;
		MVP mvp;

		Mesh(std::vector<Vertex> &a_Vertices, std::vector<Index> &a_Indices) : vertices{a_Vertices}, indices{a_Indices} {};
	};

	struct Transform
	{
		glm::vec3 position = glm::vec3(0.f, 0.f, 0.f);
		glm::vec3 rotation = glm::vec3(0.f, 0.f, 0.f);
		glm::vec3 scale = glm::vec3(0.f, 0.f, 0.f);
	};

	struct Material
	{
		vk::PipelineLayout pipelineLayout = VK_NULL_HANDLE;
		vk::Pipeline solidPipeline = VK_NULL_HANDLE;
		vk::Pipeline wireframePipeline = VK_NULL_HANDLE;
	};

	struct Cubemap
	{};

	struct RenderModel
	{
		Mesh &mesh;
		Transform &transform;
		Material &material;

		RenderModel(Mesh& a_Mesh, Transform& a_Transform, Material& a_Material) 
			: mesh{a_Mesh}, transform{a_Transform}, material{a_Material} {};
	};

	struct MeshPushConstant
	{
		alignas(16) glm::vec3 offset;
		alignas(16) glm::vec3 color;
	};

	struct GLTF
	{
		struct Primitive
		{
			uint32_t firstIndex;
			uint32_t indexCount;
			int32_t materialIndex;
		};

		struct Mesh
		{
			std::vector<Primitive> primitives;
		};

		struct Node
		{
			Node* parent;
			std::vector<Node> children;
			Mesh mesh;
			glm::mat4 matrix;
		};

		struct Material
		{
			glm::vec4 baseColorFactor = glm::vec4(1.f);
			uint32_t baseColorTextureIndex;
		};

		struct Image 
		{
			vk::DescriptorSet descriptorSet;
		};

		struct Texture
		{
			int32_t imageIndex;
		};
	};
}