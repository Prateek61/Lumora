#pragma once
#include "Lumora.h"
#include "ExampleAsset.h"

class LumoraLayer : public Lumora::Layer
{
public:
	LumoraLayer()
		: Lumora::Layer("LumoraLayer")
	{
		Handle = Lumora::Assets::Get<ExampleAsset>("JustExample");
	}
	~LumoraLayer() override = default;

	void OnAttach() override
	{
		LM_LOG_INFO("LumoraLayer Attached");
	}

	void OnDetach() override
	{
		LM_LOG_INFO("LumoraLayer Detached");
	}

	void OnUpdate(Lumora::TimeStep ts) override;
	void OnRender() override;

private:
	Lumora::AssetHandle<ExampleAsset> Handle;
	float m_FPS = 0.0f;
};