#include "triangleGLTFLoader.hpp"

namespace triangle
{
	void GLTFLoader::loadImages(tinygltf::Model& input)
	{
		images.resize(input.images.size());
		for (size_t i = 0; i < input.images.size(); ++i)
		{
			tinygltf::Image& glTFImage = input.images[i];

			unsigned char* buffer = nullptr;
			vk::DeviceSize bufferSize = 0;
			bool deleteBuffer = false;

			if (glTFImage.component == 3)
			{
				bufferSize = glTFImage.width * glTFImage.height * 4;
				buffer = new unsigned char[bufferSize];
				unsigned char* rgba = buffer;
				unsigned char* rgb = &glTFImage.image[0];

				for(size_t i = 0; i < glTFImage.width * glTFImage.height; ++i)
				{
					memcpy(rgba, rgb, sizeof(unsigned char) * 3);
					rgba += 4;
					rgb += 3;
				}

				deleteBuffer = true;
			}
			else
			{
				buffer = &glTFImage.image[0];
				bufferSize = glTFImage.image.size();
			}

			// images[i].texture
		}
	}
}