#pragma once

#include "Lumora.h"

using namespace Lumora;

class Spark : public Core::Plugin
{
public:
	Spark() = default;
	
	void Build(Core::Application& app) override;

	void Cleanup(Core::Application& app) override;

	void AddDependencies(Core::DependencyList& dependencies) override;

	const char* GetName() const override
	{
		return "Spark";
	}

private:
	Aether::System m_UpdateSystem;
	Aether::System m_RenderSystem;
};