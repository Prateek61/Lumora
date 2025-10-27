#pragma once
#include "Lumora.h"
#include "ExampleAsset.h"

class LumoraLayer : public Lumora::Layer
{
public:
	LumoraLayer()
		: Lm::Layer("LumoraLayer")
	{
		Handle = Lumora::Assets::Get<ExampleAsset>("JustExample");
	}
	~LumoraLayer() override = default;

	void OnAttach() override
	{
		LM_LOG_INFO("LumoraLayer Attached");
		Lm::Renderer2D::Init();
	}

	void OnDetach() override
	{
		LM_LOG_INFO("LumoraLayer Detached");
	}

	void OnUpdate(Lm::TimeStep ts) override;
	void OnRender() override;
	void OnEvent(Lumora::Event& e) override;

	void OnImGuiRender(Lumora::TimeStep ts) override;

private:
	Lm::AssetHandle<ExampleAsset> Handle;
	float m_FPS = 0.0f;
	uint32_t m_Color = 0xff336699;
	float m_CumTime = 0.0f;

	bool m_ShowImGuiMenu = false;
	bool m_ShowLumoraWindow = false;
};