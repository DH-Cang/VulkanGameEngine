mkdir "shaders\bin"
glslc.exe shaders\src\simple_shader.vert -o shaders\bin\simple_shader.vert.spv
glslc.exe shaders\src\simple_shader.frag -o shaders\bin\simple_shader.frag.spv
glslc.exe shaders\src\point_light.vert -o shaders\bin\point_light.vert.spv
glslc.exe shaders\src\point_light.frag -o shaders\bin\point_light.frag.spv
