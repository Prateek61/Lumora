#pragma once

#include "Lumora/Common/Base.h"
#include "Lumora/Scripting/LuaSerializer.h"

namespace Lumora
{
	struct ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			LM_CORE_ASSERT(index < Count, "Command line Args Index out of range")
				return Args[index];
		}
	};

	struct WindowProps
	{
		std::string Title = "Lumora!!";
		uint32_t Height = 1280;
		uint32_t Width = 720;
	};

	struct ApplicationProps
	{
		std::string Name = "Lumora Application";
		std::filesystem::path AssetsDirectory;
		ApplicationCommandLineArgs CommandLineArgs;
		WindowProps WindowProps;

		bool Run = true; // Whether to run the application loop, for debugging or testing purposes
	};
}

VISITABLE_STRUCT(Lumora::WindowProps, Title, Width, Height);
VISITABLE_STRUCT(Lumora::ApplicationProps, Name, AssetsDirectory, WindowProps, Run);