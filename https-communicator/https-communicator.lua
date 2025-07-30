project "https-communicator"
    location "."
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++latest"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files 
    {
        "src/**.cpp",           
        "include/**.h",     
        "include/**.inl",
        "global/**.h",
        "global/**.cpp",
    }
    
    includedirs 
    {
        "%{IncludeDir.ASIO}",
        "global",            
        "include",           
        "src",
    }
    
    if _OPTIONS["with-openssl"] then
        includedirs "%{IncludeDir.OpenSSL}"
        libdirs "%{LibDir.OpenSSL}"
        defines
        {
            "OPENSSL_NO_AUTO_INIT", 
            "OPENSSL_NO_AUTO_CLEANUP",
            "OPENSSL_USE_STATIC_LIBS",
        }
        links 
        { 
            "libssl_static",
            "libcrypto_static.lib",
            "ws2_32",
            "crypt32",
            "user32",
            "gdi32",
        }
    end
    
    if _OPTIONS["with-gtest"] then
        includedirs "%{IncludeDir.TEST}"
        libdirs "%{LibDir.TEST}"
        links 
        {
            "gtest_main",
            "gtest"
        }
    end

    defines "ASIO_STANDALONE"
    
    pchheader "headers.h"
    pchsource "global/headers.cpp"

    -- Toolset and compiler settings
    filter "toolset:msc"
        toolset "msc-v143"
        buildoptions { "/std:c++23" } 
        
    filter "toolset:gcc or toolset:clang"
        buildoptions { "-std=c++23" }

    -- Configuration settings
    filter "configurations:Debug"
        defines "DEBUG"
        symbols "On"
        optimize "Off"
        runtime  "Release"--"Debug"

    filter "configurations:Release"
        symbols "Off"
        optimize "On"
        defines "NDEBUG"
        runtime "Release"

    -- Windows specific settings
    filter "system:windows"
        systemversion "latest"
        defines "PLATFORM_WINDOWS"
    
    -- Visual Studio specific settings
    filter "action:vs*"
        defines "_CRT_SECURE_NO_WARNINGS"
        staticruntime "on"

    -- Linux/GCC/Clang settings
    filter "system:linux or toolset:gcc or toolset:clang"
        buildoptions { "-include pch.h" }
