local flecsEnablePic = true

project "flecs"
	kind "StaticLib"

    language "C"
	cdialect "C99"

	targetdir ("bin/" .. outdir .. "/%{prj.name}")
	objdir ("bin/int/" .. outdir .. "/%{prj.name}")

	files
	{
		"flecs/distr/flecs.h",
        "flecs/distr/flecs.c"
	}

	includedirs
	{
		"flecs/distr"
	}

    defines { "flecs_STATIC" }

	filter "system:windows"
		links
		{
			"wsock32",
			"ws2_32",
			"dbghelp"
		}

	filter "system:linux"
		links
		{
			"pthread"
		}
        
	-- if flecsEnablePic then
	-- 	filter "system:not windows"
	-- 		pic "On"
	-- end

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"
		optimize "Debug"

	filter "configurations:Release"
		runtime "Release"
		optimize "On"
		symbols "On"

	filter "configurations:Dist"
		runtime "Release"
		optimize "Full"

	filter {}

