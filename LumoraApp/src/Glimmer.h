#pragma once

#include "Lumora.h"

using namespace Lumora;

class Glimmer : public Core::Plugin
{
public:
	Glimmer(bool useImGui = true)
		: m_UseImGui(useImGui) {}

	void Build(Core::Application& app) override;

	void Cleanup(Core::Application& app) override;

	void AddDependencies(Core::DependencyList& dependencies) override;

	const char* GetName() const override
	{
		return "Glimmer";
	}

private:
	bool m_UseImGui;
};
