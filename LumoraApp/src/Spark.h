#pragma once

#include "Lumora.h"
#include "Text.h"

using namespace Lumora;

struct Data
{
	std::string Message;
};
LM_REFLECTABLE(Data, Message);

class Spark : public Core::Plugin
{
public:
	Spark() = default;
	
	void Build(Core::Application& app) override;
	void Finish(Core::Application& app) override;
	void Cleanup(Core::Application& app) override;

	void AddDependencies(Core::DependencyList& dependencies) override;

	const char* GetName() const override
	{
		return "Spark";
	}

private:
	Data m_Data;
	Atlas::AssetHandle<Text> m_Text;
};