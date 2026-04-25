#pragma once

#include "Lumora.h"

using namespace Lumora;

struct Data
{
	std::string Message;
};
VISITABLE_STRUCT(Data, Message);

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
};