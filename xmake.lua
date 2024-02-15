set_project("vulkan")

set_arch("x64")
set_warnings("all")
set_languages("c++20")

add_rules("mode.debug","mode.releasedbg", "mode.release", "mode.minsizerel")

add_requires("vulkansdk", "glfw", "glm")

target("main")
    set_default(true)
    set_kind("binary")
    add_files("src/*.cpp")
    add_packages("vulkansdk", "glfw", "glm")