#pragma once

#include "Lumora/Utilities/TimeStep.h"
#include "Lumora/Event/Event.h"

namespace Lumora
{
	class Layer
	{
	public:
		Layer(std::string name = "Layer")
			: m_DebugName(std::move(name))
		{
		}
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(TimeStep ts) {}
		virtual void OnRender() {}
		virtual void OnEvent(Event& e) {}
		virtual void OnImGuiRender(TimeStep ts) {}

		const std::string& GetName() const { return m_DebugName; }
	private:
		std::string m_DebugName;
	};
}