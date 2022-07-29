echo "Compiling shaders..."
glslc shaders/defaultShader.vert -o shaders/spv/defaultVert.spv
glslc shaders/defaultShader.frag -o shaders/spv/defaultFrag.spv
glslc shaders/texturedShader.vert -o shaders/spv/texturedVert.spv
glslc shaders/texturedShader.frag -o shaders/spv/texturedFrag.spv
echo "Done compiling."