#pragma once

#include "Lumora.h"

using namespace Lm;

class Glimmer : public Core::Plugin
{
public:
	Glimmer() = default;
	
	void Build(Core::Application& app) override;

	void Cleanup(Core::Application& app) override;

	const char* GetName() const override
	{
		return "Glimmer";
	}

private:
	Lumen::Renderer2D m_Renderer2D;
	Aether::System m_UpdateSystem;
	Aether::System m_RenderSystem;
};