#include "Lumora.h"

#include "Lumora/Core/Props.h"

using namespace Lumora;

int main()
{
	Log::Init();

	LuaSerializer serializer;

	auto props = serializer.DeserializeFromFile<ApplicationProps>("../Assets/Config.lua");

	auto context = RendererContext::Create();

	auto Window = Lumora::Window(props.WindowProps);
	bool running = true;

	Window::EventCallbackFn callback = [&](Event& e)
	{
		EventDispatcher dispatcher(e);
		EventHandler close_handler = [&](WindowCloseEvent&)
		{
			running = false;
			return true;
		};
		dispatcher.Dispatch<WindowCloseEvent>(close_handler);
		dispatcher.Dispatch<WindowResizeEvent>([&context](WindowResizeEvent& e)
		{
			context->Resize(e.GetWidth(), e.GetHeight());
			return true;
		});

		LM_TRACE("Event: {}", e.ToString());
	};
	Window.SetEventCallback(callback);

	
	context->Init(Window);

	while(running)
	{
		Window.OnUpdate();

		context->BeginFrame();
		context->EndFrame();
	}

	context->Shutdown();
}
