workspace "Https-Communicator"
    architecture "x64"
    startproject "ProjName"

    configurations { "Debug", "Release" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder
IncludeDir = {}
IncludeDir["ASIO"] = "../vendor/ASIO/include"
IncludeDir["OpenSSL"] = "../vendor/OPENSSL/include"
IncludeDir["TEST"] = "../test/include"


LibDir = {}
LibDir["OpenSSL"] = "../vendor/OPENSSL/lib"
LibDir["TEST"] = "../test/lib"




group "https-communicator"
    include "https-communicator/https-communicator.lua"