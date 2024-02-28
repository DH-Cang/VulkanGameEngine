add_requires("glslang", {configs = {binaryonly = true}})

target("shader")
    set_kind("object")
    add_rules("utils.glsl2spv", {outputdir = "$(buildir)/ShaderBin"})
    add_files("*.vert", "*.frag")
    add_packages("glslang")