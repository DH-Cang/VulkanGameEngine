includes("EngineSystems", "Vk");

target("VulkanGameEngine")
    set_default(true) -- set this target as default build
    set_kind("binary")
    add_deps("EngineSystems", "Vk", "shader")
    add_files("*.cpp")

    set_rundir("$(projectdir)")