#include "Lumora.h"

#include "Lumora/Core/Props.h"

using namespace Lumora;

int main()
{
	Log::Init();

	LuaSerializer serializer;

	auto props = serializer.DeserializeFromFile<ApplicationProps>("../Assets/Config.lua");

	auto Window = Lumora::Window(props.WindowProps);
	bool running = true;
	Window::EventCallbackFn callback = [&running](Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>([&running](WindowCloseEvent& e)
		{
			running = false;
			return true;
		});

		LM_TRACE("Event: {}", e.ToString());
	};
	Window.SetEventCallback(callback);

	while(running)
	{
		Window.OnUpdate();
	}
}
