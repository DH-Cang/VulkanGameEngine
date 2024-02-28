set_project("CDH_VulkanGameEngine")
set_arch("x64")
set_warnings("all")
set_languages("c++20")
set_exceptions("cxx") -- prevent error message: exceptions disabled

add_rules("mode.debug", "mode.release")
set_defaultmode("release") -- set default compile mode

add_requires("glfw", "glm", "vulkansdk")
add_packages("vulkansdk", "glfw", "glm")

includes("src", "shaders")

