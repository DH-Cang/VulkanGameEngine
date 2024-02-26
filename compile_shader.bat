mkdir "shaders\bin"
glslc.exe shaders\src\simple_shader.vert -o shaders\bin\simple_shader.vert.spv
glslc.exe shaders\src\simple_shader.frag -o shaders\bin\simple_shader.frag.spv