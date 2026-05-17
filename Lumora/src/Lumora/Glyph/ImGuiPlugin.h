#pragma once

#include "Lumora/Core/Plugin.h"
#include "Lumora/Core/SmartPointers.h"
#include "Lumora/Glyph/ImGuiBackend.h"
#include "Lumora/Rune/Reflect.h"

struct ImGuiContext;

namespace Lumora::Glyph
{
	struct ImGuiIOState
	{
		bool WantsCaptureMouse = false;
		bool WantsCaptureKeyboard = false;
		bool WantsTextInput = false;
	};

	struct ImGuiSettings
	{
		bool DockingEnabled = true;
		bool ViewportsEnabled = true;
		bool ShowDemoWindow = false;
		bool DockSpaceOverMainViewport = true;

		std::string IniFilename = "";
	};

	class ImGuiPlugin final : public Core::Plugin
	{
	public:
		ImGuiPlugin(const ImGuiSettings& settings = {});

		void Build(Core::Application& app) override;
		void Finish(Core::Application& app) override;
		void Cleanup(Core::Application& app) override;
		void AddDependencies(Core::DependencyList& dependencies) override;

		const char* GetName() const override { return "ImGuiPlugin"; }

	private:
		ImGuiContext* m_ImGuiContext = nullptr;
		Scope<ImGuiBackend> m_ImGuiBackend;

		ImGuiSettings m_InitialSettings;
	};
}

LM_REFLECTABLE(Lumora::Glyph::ImGuiSettings, DockingEnabled, ViewportsEnabled, ShowDemoWindow, DockSpaceOverMainViewport, IniFilename);