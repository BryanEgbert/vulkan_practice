echo "Compiling shaders..."
glslc shaders/defaultShader.vert -o shaders/spv/defaultVert.spv
glslc shaders/defaultShader.frag -o shaders/spv/defaultFrag.spv
glslc shaders/texturedShader.vert -o shaders/spv/texturedVert.spv
glslc shaders/texturedShader.frag -o shaders/spv/texturedFrag.spv
glslc shaders/cubemapShader.vert -o shaders/spv/cubemapVert.spv
glslc shaders/cubemapShader.frag -o shaders/spv/cubemapFrag.spv
echo "Done compiling."