workspace "Https-Communicator"
    architecture "x64"
    startproject "ProjName"

    configurations { "Debug", "Release" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder
IncludeDir = {}
IncludeDir["ASIO"] = "../vendor/ASIO/include"
IncludeDir["OpenSSL"] = "../vendor/OPENSSL/include"
IncludeDir["TEST"] = "../vendor/TEST/include"


LibDir = {}
LibDir["OpenSSL"] = "../vendor/OPENSSL/lib"
LibDir["TEST"] = "../vendor/TEST/lib"


newoption 
{
    trigger = "with-openssl",
    description = "Include the OpenSSL Library"
}

newoption 
{
    trigger = "with-gtest",
    description = "Include the Google Testing Library"
}



group "https-communicator"
    include "https-communicator/https-communicator.lua"