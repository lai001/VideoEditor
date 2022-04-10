set_xmakever("2.6.3")

includes("../GLKit")
includes("../Core")

add_requires("glm")
add_requires("glad")
add_requires("glfw")
add_requires("rapidjson")
add_requires("spdlog")
add_requires("libsdl")

rule("SupportFFmpeg")
    on_load(function (target)
        if os.exists("../Vendor/win32/ffmpeg") == false then
            local oldir = os.cd("../Vendor/win32")
            import("net.http")
            import("utils.archive")
            if os.exists("ffmpeg.zip") == false then
                http.download("https://github.com/BtbN/FFmpeg-Builds/releases/download/autobuild-2021-02-28-12-32/ffmpeg-n4.3.2-160-gfbb9368226-win64-gpl-shared-4.3.zip", "ffmpeg.zip")
            end
            archive.extract("ffmpeg.zip")
            os.trymv("ffmpeg-n4.3.2-160-gfbb9368226-win64-gpl-shared-4.3", "ffmpeg")
            os.cd(oldir)            
        end    
        target:add("includedirs", "../Vendor/win32/ffmpeg/include")
        target:add("linkdirs", "../Vendor/win32/ffmpeg/lib")
        target:add("links", "avcodec", "avformat", "avutil", "swresample", "swscale")
    end)
    after_build(function (target)
        os.cp("../Vendor/win32/ffmpeg/bin/*.dll", target:targetdir())
    end)

rule("Supportimgui")
    on_load(function (target)
        if os.exists("../Vendor/win32/imgui") == false then
            import("devel.git")
            local oldir = os.cd("../Vendor/win32")
            git.clone("https://github.com/ocornut/imgui.git", {branch = "master", outputdir = "imgui"})
            git.checkout("v1.87", {repodir = "imgui"})
            os.cd(oldir)
        end

        target:add("includedirs", "../Vendor/win32/imgui")
        target:add("files", "../Vendor/win32/imgui/*.cpp")
        target:add("files", "../Vendor/win32/imgui/backends/imgui_impl_glfw.cpp")
        target:add("files", "../Vendor/win32/imgui/backends/imgui_impl_opengl3.cpp")
        target:add("headerfiles", "../Vendor/win32/imgui/*.h")
        target:add("headerfiles", "../Vendor/win32/imgui/backends/imgui_impl_glfw.h")
        target:add("headerfiles", "../Vendor/win32/imgui/backends/imgui_impl_opengl3.h")
        target:add("headerfiles", "../Vendor/win32/imgui/backends/imgui_impl_opengl3_loader.h")
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

target("App")
    set_kind("binary")
    set_languages("cxx11")
    add_files("*.cpp")
    add_includedirs("../")
    add_rules("mode.debug", "mode.release")
    add_rules("SupportFFmpeg")
    add_rules("Supportimgui")
    add_rules("Supportlibsdl")
    add_packages("glad")
    add_packages("glfw")
    add_packages("glm")
    add_packages("rapidjson")
    add_packages("spdlog")
    add_packages("libsdl")
    add_deps("GLKit")
    add_deps("Core")