#pragma once

#include "Lumora.h"

using namespace Lm;

class Glimmer : public Core::Plugin
{
public:
	Glimmer() = default;
	
	void Build(Core::Application& app) override;

	const char* GetName() const override
	{
		return "Glimmer";
	}
};