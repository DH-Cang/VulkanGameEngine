includes("EngineSystems", "Vk", "EngineCore", "Platform", "ThirdParty");

target("VulkanGameEngine")
    set_default(true) -- set this target as default build
    set_kind("binary")
    add_deps("EngineCore", "EngineSystems", "shader")
    add_files("*.cpp")

    set_rundir("$(projectdir)")