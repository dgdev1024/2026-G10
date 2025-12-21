--
-- @file    premake5.lua
-- @author  Dennis W. Griffin <dgdev1024@gmail.com>
-- @date    2025-11-10
--
-- @brief   The Premake5 build script for the G10 Workspace
--

-- Options ---------------------------------------------------------------------

newoption {
    trigger     = "build-static",
    description = "Build static libraries instead of shared libraries"
}

newoption {
    trigger     = "unused-is-error",
    description = "Treat unused variable/function/parameter warnings as errors"
}

-- Workspace -------------------------------------------------------------------

workspace "2026-G10"
    
    -- Language and Standard
    language "C++"
    cppdialect "C++23"

    -- Extra Warnings; Treat Warnings as Errors
    warnings "Extra"
    fatalwarnings { "All" }

    -- Ignore warnings involving unused parameters, functions, variables, etc.
    if _OPTIONS["unused-is-error"] == nil then
        filter { "toolset:gcc or clang" }
            buildoptions { "-Wno-unused-parameter", "-Wno-unused-function", 
                "-Wno-unused-variable", "-Wno-unused-but-set-variable" }
        filter { "toolset:msc" }
            buildoptions { "/wd4100", "/wd4505", "/wd4189", "/wd4181" }
        filter {}
    end

    -- Build Configurations
    configurations { "debug", "release", "distribute" }
    startproject "g10mu"

    -- Configuration Settings
    filter { "configurations:debug" }
        defines { "G10_DEBUG", "DEBUG" }
        symbols "On"
    filter { "configurations:release" }
        defines { "G10_RELEASE", "NDEBUG" }
        optimize "On"
    filter { "configurations:distribute" }
        defines { "G10_DISTRIBUTE", "NDEBUG" }
        optimize "Full"
        symbols "Off"
    filter { "system:linux" }
        defines { "G10_LINUX" }
    filter { "system:windows" }
        defines { "G10_WINDOWS" }
    filter {}

-- Project: `g10` - G10 CPU Emulator Core Library ------------------------------

project "g10"
    if _OPTIONS["build-static"] then
        kind "StaticLib"
        pic "On"
        defines { "G10_BUILD_STATIC" }
    else
        kind "SharedLib"
        pic "On"
        defines { "G10_BUILDING_SHARED" }
    end

    location "./build"
    targetdir "./build/bin/%{cfg.system}-%{cfg.buildcfg}"
    objdir "./build/obj/%{cfg.system}-%{cfg.buildcfg}/%{prj.name}"
    files { "./projects/g10/**.hpp", "./projects/g10/**.cpp" }
    includedirs { "./projects" }
    
-- Project: `g10asm` - G10 Assembler Tool --------------------------------------

project "g10asm"
    kind "ConsoleApp"

    location "./build"
    targetdir "./build/bin/%{cfg.system}-%{cfg.buildcfg}"
    objdir "./build/obj/%{cfg.system}-%{cfg.buildcfg}/%{prj.name}"
    files { "./projects/g10asm/**.hpp", "./projects/g10asm/**.cpp" }
    includedirs { "./projects", "./projects/g10" }
    links { "g10" }
    