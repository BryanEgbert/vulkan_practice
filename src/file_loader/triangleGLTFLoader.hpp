#pragma once

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE

#include "../triangleTypes.hpp";
#include <tinygltf/tiny_gltf.h>

namespace triangle
{
	class GLTFLoader
	{
	public:
		void loadImages(tinygltf::Model& input);
	private:
		std::vector<GLTF::Image> images;
		std::vector<GLTF::Texture> textures;
		std::vector<GLTF::Material> materials;
		std::vector<GLTF::Node> nodes;
	};
}