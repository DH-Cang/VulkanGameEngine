target("EngineSystems")
    set_kind("static")
    add_files("*.cpp")
    add_includedirs("$(projectdir)/src")
    add_deps("Vk", "EngineCore")

