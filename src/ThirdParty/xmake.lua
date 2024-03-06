target("ThirdParty")
    set_kind("static")
    add_files("*.cpp")
    add_defines("SPIRV_REFLECT_USE_SYSTEM_SPIRV_H")

