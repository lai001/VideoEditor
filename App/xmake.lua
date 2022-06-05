set_xmakever("2.6.3")

includes("../../Foundation/Foundation")
includes("../../KSRenderEngine/KSRenderEngine")
includes("../../KSImage/KSImage")
includes("../VideoEditor")

add_requires("glm")
add_requires("spdlog")
add_requires("libsdl")
add_requires("tclap")

rule("App.deps")
    on_load(function (target)
        if os.exists("../Vendor/win32/imgui") == false then
            import("devel.git")
            local oldir = os.cd("../Vendor/win32")
            git.clone("https://github.com/ocornut/imgui.git", {branch = "master", outputdir = "imgui"})
            git.checkout("v1.87", {repodir = "imgui"})
            os.cd(oldir)
        end
    end)

rule("Supportlibsdl")
    after_build(function (target)
        local libsdl_pkg = target:pkgs()["libsdl"]
        local libfiles = libsdl_pkg:get("libfiles")
        for index, file in pairs(libfiles) do
            if path.extension(file) == ".dll" then
                os.cp(file, path.join(target:targetdir(), path.filename(file)))
            end
        end
    end)

rule("App.Copy")
    after_build(function (target)
        local resourceDir = path.join(target:targetdir(), "Resource")
        local shaderDir = path.join(resourceDir, "Shader")
        if os.exists(resourceDir) == false then
            os.mkdir(resourceDir)
        end
        if os.exists(shaderDir) == false then
            os.mkdir(shaderDir)
        end
        os.cp("Shader/*.hlsl", shaderDir)
    end)

target("App")
    set_kind("binary")
    set_languages("c++17")
    add_files("main.cpp")
    add_files("Platform/*.cpp")
    add_includedirs("Platform")
    add_headerfiles("Platform/*.hpp")
    add_rules("mode.debug", "mode.release")
    add_rules("Supportlibsdl")
    add_rules("App.Copy")
    add_packages("glm")
    add_packages("spdlog")
    add_packages("libsdl")
    add_deps("Foundation")
    add_deps("VideoEditor")
    add_deps("KSRenderEngine")
    add_deps("KSImage")
    add_deps("ImGui")

target("ImGui")
    set_kind("static")
    set_languages("c++17")
    add_rules("mode.debug", "mode.release", "App.deps")
    add_includedirs("../Vendor/win32/imgui", { public = true })
    add_files("../Vendor/win32/imgui/*.cpp")
    add_files("../Vendor/win32/imgui/backends/imgui_impl_dx11.cpp")
    add_files("../Vendor/win32/imgui/backends/imgui_impl_win32.cpp")
    add_headerfiles("../Vendor/win32/imgui/*.h")
    add_headerfiles("../Vendor/win32/imgui/backends/imgui_impl_win32.h")
    add_headerfiles("../Vendor/win32/imgui/backends/imgui_impl_dx11.h")

target("VideoEditorCmd")
    set_kind("binary")
    set_languages("c++17")
    add_files("VideoEditorCmd.cpp")
    add_files("Platform/*.cpp")
    add_includedirs("Platform")
    add_headerfiles("Platform/*.hpp")
    add_rules("mode.debug", "mode.release")
    add_packages("spdlog")
    add_packages("tclap")
    add_deps("VideoEditor")
    add_deps("Foundation")