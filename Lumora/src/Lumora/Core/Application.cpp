#include "LMPCH.h"
#include "Application.h"

#include "Lumora/Core/Props.h"

namespace Lumora
{
	class Application
	{
	public:
		Application(ApplicationProps& props);
		virtual ~Application();
	};
}