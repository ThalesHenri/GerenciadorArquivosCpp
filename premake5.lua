workspace "GerenciadorArquivos"
    architecture "x64"
    startproject "GerenciadorArquivos"

    configurations { "Debug", "Release" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

project "GerenciadorArquivos"
    kind "ConsoleApp" -- ConsoleApp permite ver logs do terminal (ótimo para dar printf/cout de debug)
    language "C++"
    cppdialect "C++17"
    targetdir "bin/%{cfg.buildcfg}"
    objdir "bin-int/%{cfg.buildcfg}"

    -- Mapeia todos os códigos do projeto e os arquivos do ImGui dentro de 'externals'
    files {
        "include/**.hpp",
        "src/**.cpp",
        "externals/imgui/imgui*.cpp",
        "externals/imgui/backends/imgui_impl_glfw.cpp",
        "externals/imgui/backends/imgui_impl_opengl3.cpp"
    }

    -- Caminhos para o compilador achar os #include
    includedirs {
        "include",
        "externals/imgui",
        "externals/imgui/backends"
    }

    -- Configurações exclusivas para Windows
    filter "system:windows"
        systemversion "latest"
        defines { "GLFW_INCLUDE_NONE" }
        links { "glfw3", "opengl32", "gdi32", "shell32" }

    -- Configurações exclusivas para Linux
    filter "system:linux"
        defines { "GLFW_INCLUDE_NONE" }
        links { "glfw", "GL", "X11", "pthread", "dl" }

    filter {}
