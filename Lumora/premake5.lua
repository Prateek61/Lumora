project "Lumora"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    targetdir ("%{wks.location}/bin/" .. outdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin/bin-int/" .. outdir .. "/%{prj.name}")

    pchheader "lmpch.h"
    pchsource "src/lmpch.cpp"