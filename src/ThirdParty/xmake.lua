includes("SpirvReflection")

target("ThirdParty")
    set_kind("static")
    add_files("*.cpp")
    add_deps("SpirvReflection")

