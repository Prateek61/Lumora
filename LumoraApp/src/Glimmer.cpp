#include "Glimmer.h"

#include <GLM/gtc/matrix_transform.hpp>
#include <imgui.h>

#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <numbers>
#include <vector>

namespace glimmer
{
	struct Particle
	{
		glm::vec2 Pos{0.0f};
		glm::vec2 Vel{0.0f};
		glm::vec4 Color{1.0f};
		float Size = 0.015f;
		float Age = 0.0f;
		float Lifetime = 3.0f;
	};

	struct TrailPoint
	{
		glm::vec2 Pos{0.0f};
		glm::vec4 Color{1.0f};
		float Size = 0.01f;
		float Age = 0.0f;
		float Lifetime = 0.5f;
	};

	struct Attractor
	{
		glm::vec2 Pos{0.0f};
		float Strength = 1.5f;
		float Radius = 0.6f;
		bool Repel = false;
	};

	struct SimParams
	{
		// Physics
		glm::vec2 Gravity{0.0f, -0.3f};
		float Damping = 0.995f;
		float MaxSpeed = 2.5f;
		bool BounceBounds = true;
		bool WrapBounds = false;
		float BoundExtent = 1.0f;
		float Restitution = 0.75f;

		// Swirl
		float SwirlStrength = 0.0f;
		float SwirlRadius = 1.2f;

		// Spawn
		int BurstCount = 40;
		float SpawnSpeed = 1.2f;
		float SpawnSpeedVariance = 0.6f;
		float ParticleLifetime = 3.0f;
		float LifetimeVariance = 1.0f;
		float ParticleSize = 0.018f;
		float ParticleSizeVariance = 0.01f;
		float HueBase = 0.55f;
		float HueRange = 1.0f;
		float Saturation = 0.85f;
		float Value = 1.0f;
		bool RainbowOverTime = true;
		bool SpawnOnDrag = true;
		float DragSpawnRate = 400.0f; // per second while dragging

		// Trails
		bool TrailsEnabled = true;
		float TrailSampleRate = 80.0f;
		float TrailLifetime = 0.6f;
		float TrailSizeFactor = 0.7f;

		// Attractors
		float AttractorDefaultStrength = 1.8f;
		float AttractorDefaultRadius = 0.7f;
		float AttractorEpsilon = 0.03f;
		bool ShowAttractorGizmos = true;

		// Backdrop
		int BackdropGridSize = 40;
		float BackdropQuadSize = 0.03f;
		float BackdropGap = 0.012f;
		float BackdropIntensity = 0.8f;
		float BackdropTimeScale = 1.0f;

		// Clear color anim
		bool ClearColorAnim = false;
		glm::vec4 BaseClearColor{0.05f, 0.05f, 0.07f, 1.0f};
	};

	struct SimRuntime
	{
		float Time = 0.0f;
		float DtScale = 1.0f;
		bool Paused = false;
		float TrailEmitAccum = 0.0f;
		float DragSpawnAccum = 0.0f;

		glm::vec2 CursorNdc{0.0f};
		bool CursorInWindow = false;
		glm::vec2 LastMouseNdc{0.0f};
		bool HasLastMouse = false;
		bool DragSpawning = false;

		uint32_t Seed = 0xCAFEBABEu;
	};

	struct Particles
	{
		static constexpr size_t MaxAlive = 2000;
		std::vector<Particle> Items;
		size_t AliveCount = 0;
	};

	struct Trails
	{
		static constexpr size_t MaxPoints = 8000;
		std::vector<TrailPoint> Items;
		// Ring index for overwrite when full
		size_t Head = 0;
		size_t Count = 0;
	};

	struct Attractors
	{
		std::vector<Attractor> Items;
	};

	struct SystemHandles
	{
		Aether::System SimInput;
		Aether::System ClearColorAnim;
		Aether::System Physics;
		Aether::System AttractorForce;
		Aether::System Swirl;
		Aether::System Lifetime;
		Aether::System TrailEmit;
		Aether::System RenderBackdrop;
		Aether::System RenderTrails;
		Aether::System RenderParticles;
		Aether::System RenderAttractors;
		Aether::System UI;
	};
}

namespace
{
	using namespace glimmer;
	constexpr float Pi = std::numbers::pi_v<float>;

	// Layer z-values. Depth test is GL_LESS with identity projection, so
	// smaller z = drawn on top. Later layers use smaller z.
	constexpr float ZBackdrop   =  0.9f;
	constexpr float ZTrails     =  0.5f;
	constexpr float ZParticles  =  0.1f;
	constexpr float ZAttractors = -0.3f;

	float RandFloat(uint32_t& seed, float a = 0.0f, float b = 1.0f)
	{
		seed = seed * 1664525u + 1013904223u;
		float t = ((seed >> 8) & 0xFFFFFFu) / float(0x01000000u);
		return a + (b - a) * t;
	}

	glm::vec4 HsvToRgba(float h, float s, float v, float a)
	{
		h = std::fmod(h, 1.0f);
		if (h < 0.0f) h += 1.0f;
		float c = v * s;
		float x = c * (1.0f - std::fabs(std::fmod(h * 6.0f, 2.0f) - 1.0f));
		float m = v - c;
		glm::vec3 rgb;
		int seg = int(h * 6.0f) % 6;
		switch (seg)
		{
		case 0: rgb = {c, x, 0.0f}; break;
		case 1: rgb = {x, c, 0.0f}; break;
		case 2: rgb = {0.0f, c, x}; break;
		case 3: rgb = {0.0f, x, c}; break;
		case 4: rgb = {x, 0.0f, c}; break;
		default: rgb = {c, 0.0f, x}; break;
		}
		rgb += m;
		return {rgb.r, rgb.g, rgb.b, a};
	}

	glm::vec2 ScreenToNdc(float mx, float my, float width, float height)
	{
		if (width <= 0.0f || height <= 0.0f) return {0.0f, 0.0f};
		float x = (mx / width) * 2.0f - 1.0f;
		float y = 1.0f - (my / height) * 2.0f;
		return {x, y};
	}

	void DrawQuadCentered(Lumen::Renderer2D& r, glm::vec2 center, glm::vec2 size, const glm::vec4& color, float z = 0.0f)
	{
		r.DrawQuad(glm::vec3{center.x - size.x * 0.5f, center.y - size.y * 0.5f, z}, size, color);
	}

	void SpawnParticle(Particles& particles, const SimParams& params, SimRuntime& rt,
	                   glm::vec2 pos, glm::vec2 baseVel)
	{
		if (particles.AliveCount >= Particles::MaxAlive) return;

		if (particles.Items.size() <= particles.AliveCount)
			particles.Items.resize(particles.AliveCount + 1);

		Particle& p = particles.Items[particles.AliveCount++];

		float angle = RandFloat(rt.Seed, 0.0f, 2.0f * Pi);
		float speedVar = RandFloat(rt.Seed, -params.SpawnSpeedVariance, params.SpawnSpeedVariance);
		float speed = std::max(0.0f, params.SpawnSpeed + speedVar);
		glm::vec2 dir{std::cos(angle), std::sin(angle)};

		p.Pos = pos;
		p.Vel = baseVel + dir * speed;

		float hue = params.HueBase +
			(params.RainbowOverTime ? rt.Time * 0.1f : 0.0f) +
			RandFloat(rt.Seed, -params.HueRange * 0.5f, params.HueRange * 0.5f);
		p.Color = HsvToRgba(hue, params.Saturation, params.Value, 1.0f);

		p.Size = std::max(0.001f, params.ParticleSize +
			RandFloat(rt.Seed, -params.ParticleSizeVariance, params.ParticleSizeVariance));
		p.Age = 0.0f;
		p.Lifetime = std::max(0.1f, params.ParticleLifetime +
			RandFloat(rt.Seed, -params.LifetimeVariance, params.LifetimeVariance));
	}

	void SpawnBurst(Particles& particles, const SimParams& params, SimRuntime& rt, glm::vec2 pos, int count)
	{
		for (int i = 0; i < count; ++i)
			SpawnParticle(particles, params, rt, pos, {0.0f, 0.0f});
	}

	int FindNearestAttractor(const Attractors& attractors, glm::vec2 pos, float maxDist)
	{
		int best = -1;
		float bestD2 = maxDist * maxDist;
		for (size_t i = 0; i < attractors.Items.size(); ++i)
		{
			glm::vec2 d = attractors.Items[i].Pos - pos;
			float d2 = d.x * d.x + d.y * d.y;
			if (d2 < bestD2)
			{
				bestD2 = d2;
				best = int(i);
			}
		}
		return best;
	}

	bool SystemToggle(const char* label, const Aether::System& sys)
	{
		bool enabled = sys.IsEnabled();
		bool was = enabled;
		if (ImGui::Checkbox(label, &enabled))
		{
			if (enabled) sys.Enable(); else sys.Disable();
		}
		return enabled != was;
	}

	// --------------------------------------------------------------------
	// Systems
	// --------------------------------------------------------------------

	void SimInputSystem(Aether::QueryRes& res)
	{
		auto world = res.World();
		auto input = Flux::Input::Get(world);
		auto& params = world.GetResourceMut<SimParams>();
		auto& rt = world.GetResourceMut<SimRuntime>();
		auto& particles = world.GetResourceMut<Particles>();
		auto& attractors = world.GetResourceMut<Attractors>();
		const auto& window = *world.GetResource<Flux::WindowResource>().Resource;
		const auto& io = world.GetResource<Glyph::ImGuiIOState>();

		float w = float(window.GetProps().Width);
		float h = float(window.GetProps().Height);

		glm::vec2 ndc = ScreenToNdc(input.Mouse.X, input.Mouse.Y, w, h);
		rt.CursorNdc = ndc;
		rt.CursorInWindow = (input.Mouse.X >= 0 && input.Mouse.X <= w && input.Mouse.Y >= 0 && input.Mouse.Y <= h);

		float dt = res.DeltaTime().GetSeconds();

		// Keyboard — only if ImGui isn't capturing it
		if (!io.WantsCaptureKeyboard)
		{
			if (input.Keyboard.Pressed(Flux::Key::P) || input.Keyboard.Pressed(Flux::Key::Space))
				rt.Paused = !rt.Paused;
			if (input.Keyboard.Pressed(Flux::Key::R))
				particles.AliveCount = 0;
			if (input.Keyboard.Pressed(Flux::Key::C))
				attractors.Items.clear();
		}

		// Mouse — only if ImGui isn't capturing it
		if (!io.WantsCaptureMouse && rt.CursorInWindow)
		{
			if (input.Mouse.Pressed(Flux::Mouse::ButtonLeft))
			{
				SpawnBurst(particles, params, rt, ndc, params.BurstCount);
				rt.DragSpawning = params.SpawnOnDrag;
				rt.DragSpawnAccum = 0.0f;
			}
			if (input.Mouse.Released(Flux::Mouse::ButtonLeft))
				rt.DragSpawning = false;

			if (rt.DragSpawning && input.Mouse.Down(Flux::Mouse::ButtonLeft))
			{
				rt.DragSpawnAccum += params.DragSpawnRate * dt;
				int n = int(rt.DragSpawnAccum);
				rt.DragSpawnAccum -= float(n);
				glm::vec2 baseVel{0.0f};
				if (rt.HasLastMouse)
					baseVel = (ndc - rt.LastMouseNdc) / std::max(dt, 0.001f) * 0.25f;
				for (int i = 0; i < n; ++i)
					SpawnParticle(particles, params, rt, ndc, baseVel);
			}

			if (input.Mouse.Pressed(Flux::Mouse::ButtonRight))
			{
				bool repel = input.Keyboard.Down(Flux::Key::LeftShift) || input.Keyboard.Down(Flux::Key::RightShift);
				Attractor a;
				a.Pos = ndc;
				a.Strength = params.AttractorDefaultStrength;
				a.Radius = params.AttractorDefaultRadius;
				a.Repel = repel;
				attractors.Items.push_back(a);
			}

			if (input.Mouse.Pressed(Flux::Mouse::ButtonMiddle))
			{
				int idx = FindNearestAttractor(attractors, ndc, 0.25f);
				if (idx >= 0)
					attractors.Items.erase(attractors.Items.begin() + idx);
			}
		}

		rt.LastMouseNdc = ndc;
		rt.HasLastMouse = true;

		if (!rt.Paused)
			rt.Time += dt * rt.DtScale;
	}

	void ClearColorAnimSystem(Aether::QueryRes& res)
	{
		auto world = res.World();
		auto& params = world.GetResourceMut<SimParams>();
		auto& rt = world.GetResourceMut<SimRuntime>();
		auto& render_device = world.GetResource<Lumen::RenderDeviceResource>().Resource;

		if (!params.ClearColorAnim)
		{
			render_device->SetClearColor(params.BaseClearColor);
			return;
		}

		float t = rt.Time;
		float r = (std::sin(t * 0.7f) + 1.0f) * 0.15f + 0.05f;
		float g = (std::sin(t * 0.7f + 2.0f) + 1.0f) * 0.15f + 0.05f;
		float b = (std::sin(t * 0.7f + 4.0f) + 1.0f) * 0.15f + 0.05f;
		render_device->SetClearColor({r, g, b, 1.0f});
	}

	void PhysicsSystem(Aether::QueryRes& res)
	{
		auto world = res.World();
		auto& rt = world.GetResourceMut<SimRuntime>();
		if (rt.Paused) return;

		const auto& params = world.GetResource<SimParams>();
		auto& particles = world.GetResourceMut<Particles>();

		float dt = res.DeltaTime().GetSeconds() * rt.DtScale;
		float maxSpeedSq = params.MaxSpeed * params.MaxSpeed;

		for (size_t i = 0; i < particles.AliveCount; ++i)
		{
			Particle& p = particles.Items[i];
			p.Vel += params.Gravity * dt;
			p.Vel *= std::pow(params.Damping, dt * 60.0f); // frame-rate normalized damping

			float speedSq = p.Vel.x * p.Vel.x + p.Vel.y * p.Vel.y;
			if (speedSq > maxSpeedSq)
			{
				float s = params.MaxSpeed / std::sqrt(speedSq);
				p.Vel *= s;
			}

			p.Pos += p.Vel * dt;

			if (params.WrapBounds)
			{
				float e = params.BoundExtent;
				if (p.Pos.x < -e) p.Pos.x += 2.0f * e;
				if (p.Pos.x > e)  p.Pos.x -= 2.0f * e;
				if (p.Pos.y < -e) p.Pos.y += 2.0f * e;
				if (p.Pos.y > e)  p.Pos.y -= 2.0f * e;
			}
			else if (params.BounceBounds)
			{
				float e = params.BoundExtent;
				if (p.Pos.x < -e) { p.Pos.x = -e; p.Vel.x = -p.Vel.x * params.Restitution; }
				if (p.Pos.x > e)  { p.Pos.x = e;  p.Vel.x = -p.Vel.x * params.Restitution; }
				if (p.Pos.y < -e) { p.Pos.y = -e; p.Vel.y = -p.Vel.y * params.Restitution; }
				if (p.Pos.y > e)  { p.Pos.y = e;  p.Vel.y = -p.Vel.y * params.Restitution; }
			}
		}
	}

	void AttractorForceSystem(Aether::QueryRes& res)
	{
		auto world = res.World();
		auto& rt = world.GetResourceMut<SimRuntime>();
		if (rt.Paused) return;

		const auto& params = world.GetResource<SimParams>();
		const auto& attractors = world.GetResource<Attractors>();
		auto& particles = world.GetResourceMut<Particles>();

		if (attractors.Items.empty()) return;

		float dt = res.DeltaTime().GetSeconds() * rt.DtScale;
		float eps2 = params.AttractorEpsilon * params.AttractorEpsilon;

		for (size_t i = 0; i < particles.AliveCount; ++i)
		{
			Particle& p = particles.Items[i];
			for (const auto& a : attractors.Items)
			{
				glm::vec2 d = a.Pos - p.Pos;
				float d2 = d.x * d.x + d.y * d.y;
				float dist = std::sqrt(d2 + eps2);
				if (dist > a.Radius) continue;

				float falloff = 1.0f - (dist / a.Radius);
				float mag = a.Strength * falloff * falloff / (dist + params.AttractorEpsilon);
				if (a.Repel) mag = -mag;
				p.Vel += (d / dist) * mag * dt;
			}
		}
	}

	void SwirlSystem(Aether::QueryRes& res)
	{
		auto world = res.World();
		auto& rt = world.GetResourceMut<SimRuntime>();
		if (rt.Paused) return;

		const auto& params = world.GetResource<SimParams>();
		if (std::fabs(params.SwirlStrength) < 1e-5f) return;

		auto& particles = world.GetResourceMut<Particles>();
		float dt = res.DeltaTime().GetSeconds() * rt.DtScale;
		float r2 = params.SwirlRadius * params.SwirlRadius;

		for (size_t i = 0; i < particles.AliveCount; ++i)
		{
			Particle& p = particles.Items[i];
			float d2 = p.Pos.x * p.Pos.x + p.Pos.y * p.Pos.y;
			if (d2 > r2) continue;
			float falloff = 1.0f - std::sqrt(d2) / params.SwirlRadius;
			// Tangent (perpendicular to position)
			glm::vec2 tang{-p.Pos.y, p.Pos.x};
			p.Vel += tang * (params.SwirlStrength * falloff * dt);
		}
	}

	void LifetimeSystem(Aether::QueryRes& res)
	{
		auto world = res.World();
		auto& rt = world.GetResourceMut<SimRuntime>();
		if (rt.Paused) return;

		auto& particles = world.GetResourceMut<Particles>();
		float dt = res.DeltaTime().GetSeconds() * rt.DtScale;

		for (size_t i = 0; i < particles.AliveCount;)
		{
			Particle& p = particles.Items[i];
			p.Age += dt;
			if (p.Age >= p.Lifetime)
			{
				// Swap-and-pop
				particles.Items[i] = particles.Items[particles.AliveCount - 1];
				--particles.AliveCount;
			}
			else
			{
				++i;
			}
		}
	}

	void TrailEmitSystem(Aether::QueryRes& res)
	{
		auto world = res.World();
		auto& rt = world.GetResourceMut<SimRuntime>();
		if (rt.Paused) return;

		const auto& params = world.GetResource<SimParams>();
		auto& trails = world.GetResourceMut<Trails>();
		const auto& particles = world.GetResource<Particles>();

		float dt = res.DeltaTime().GetSeconds() * rt.DtScale;

		// Age existing trail points
		for (size_t i = 0; i < trails.Count; ++i)
			trails.Items[i].Age += dt;

		// Compact dead points (rare, so in-place)
		size_t write = 0;
		for (size_t i = 0; i < trails.Count; ++i)
		{
			if (trails.Items[i].Age < trails.Items[i].Lifetime)
			{
				if (write != i) trails.Items[write] = trails.Items[i];
				++write;
			}
		}
		trails.Count = write;

		if (!params.TrailsEnabled) return;
		if (particles.AliveCount == 0) return;

		// Sample rate accumulator
		rt.TrailEmitAccum += params.TrailSampleRate * dt;
		int samplesPerParticle = int(rt.TrailEmitAccum);
		if (samplesPerParticle <= 0) return;
		rt.TrailEmitAccum -= float(samplesPerParticle);
		samplesPerParticle = std::min(samplesPerParticle, 3);

		if (trails.Items.size() < Trails::MaxPoints)
			trails.Items.resize(Trails::MaxPoints);

		for (size_t i = 0; i < particles.AliveCount; ++i)
		{
			const Particle& p = particles.Items[i];
			for (int s = 0; s < samplesPerParticle; ++s)
			{
				TrailPoint tp;
				tp.Pos = p.Pos;
				tp.Color = p.Color;
				tp.Size = p.Size * params.TrailSizeFactor;
				tp.Age = 0.0f;
				tp.Lifetime = params.TrailLifetime;

				if (trails.Count < Trails::MaxPoints)
				{
					trails.Items[trails.Count++] = tp;
				}
				else
				{
					trails.Items[trails.Head] = tp;
					trails.Head = (trails.Head + 1) % Trails::MaxPoints;
				}
			}
		}
	}

	void RenderBackdropSystem(Aether::QueryRes& res)
	{
		auto world = res.World();
		auto& renderer = world.GetResourceMut<Lumen::Renderer2D>();
		const auto& params = world.GetResource<SimParams>();
		const auto& rt = world.GetResource<SimRuntime>();

		float time = rt.Time * params.BackdropTimeScale;
		int grid = std::max(1, params.BackdropGridSize);
		float quadSize = params.BackdropQuadSize;
		float step = quadSize + params.BackdropGap;
		float intensity = params.BackdropIntensity;

		for (int y = 0; y < grid; ++y)
		{
			for (int x = 0; x < grid; ++x)
			{
				float xpos = (x - grid / 2.0f) * step;
				float ypos = (y - grid / 2.0f) * step;

				float dist = std::sqrt(xpos * xpos + ypos * ypos);
				float angle = std::atan2(ypos, xpos);

				float wave = std::sin(dist * 8.0f - time * 3.0f) * 0.5f + 0.5f;
				float spiral = std::sin(angle * 3.0f + dist * 4.0f - time * 2.0f) * 0.5f + 0.5f;
				float pulse = std::sin(dist * 6.0f - time * 4.0f) * 0.3f + 0.7f;
				float size = quadSize * pulse;
				float offset = (quadSize - size) * 0.5f;

				float r = std::sin(wave * Pi + time * 0.7f) * 0.4f + 0.6f;
				float g = std::sin(spiral * Pi + time * 0.5f + 2.0f) * 0.4f + 0.5f;
				float b = std::sin((wave + spiral) * Pi + time * 0.3f + 4.0f) * 0.4f + 0.6f;

				float fade = 1.0f - std::clamp(dist, 0.0f, 1.0f);
				fade *= fade;

				float alpha = fade * (0.35f + wave * 0.35f) * intensity;
				if (alpha > 0.02f)
				{
					renderer.DrawQuad(glm::vec3{xpos + offset, ypos + offset, ZBackdrop},
					                  {size, size}, {r, g, b, alpha});
				}
			}
		}
	}

	void RenderTrailsSystem(Aether::QueryRes& res)
	{
		auto world = res.World();
		auto& renderer = world.GetResourceMut<Lumen::Renderer2D>();
		const auto& trails = world.GetResource<Trails>();

		for (size_t i = 0; i < trails.Count; ++i)
		{
			const TrailPoint& tp = trails.Items[i];
			float t = tp.Age / std::max(tp.Lifetime, 1e-4f);
			if (t >= 1.0f) continue;
			float alpha = (1.0f - t);
			alpha = alpha * alpha * tp.Color.a;
			float size = tp.Size * (1.0f - 0.5f * t);
			glm::vec4 color{tp.Color.r, tp.Color.g, tp.Color.b, alpha};
			DrawQuadCentered(renderer, tp.Pos, {size, size}, color, ZTrails);
		}
	}

	void RenderParticlesSystem(Aether::QueryRes& res)
	{
		auto world = res.World();
		auto& renderer = world.GetResourceMut<Lumen::Renderer2D>();
		const auto& particles = world.GetResource<Particles>();
		const auto& rt = world.GetResource<SimRuntime>();

		for (size_t i = 0; i < particles.AliveCount; ++i)
		{
			const Particle& p = particles.Items[i];
			float t = p.Age / std::max(p.Lifetime, 1e-4f);
			// Pulse size + fade out with age
			float pulse = 0.85f + 0.15f * std::sin(rt.Time * 6.0f + float(i) * 0.3f);
			float alpha = std::clamp(1.0f - t * t, 0.0f, 1.0f);
			float size = p.Size * pulse * (0.5f + 0.5f * (1.0f - t));
			glm::vec4 color{p.Color.r, p.Color.g, p.Color.b, alpha};
			DrawQuadCentered(renderer, p.Pos, {size, size}, color, ZParticles);
		}
	}

	void RenderAttractorsSystem(Aether::QueryRes& res)
	{
		auto world = res.World();
		const auto& params = world.GetResource<SimParams>();
		if (!params.ShowAttractorGizmos) return;

		auto& renderer = world.GetResourceMut<Lumen::Renderer2D>();
		const auto& attractors = world.GetResource<Attractors>();
		const auto& rt = world.GetResource<SimRuntime>();

		for (const auto& a : attractors.Items)
		{
			glm::vec4 core = a.Repel
				? glm::vec4{1.0f, 0.3f, 0.3f, 0.9f}
				: glm::vec4{0.3f, 0.8f, 1.0f, 0.9f};

			// Core dot
			float pulse = 0.8f + 0.2f * std::sin(rt.Time * 4.0f);
			DrawQuadCentered(renderer, a.Pos, {0.025f * pulse, 0.025f * pulse}, core, ZAttractors);

			// Three halo rings (approximated with concentric faded quads)
			for (int ring = 1; ring <= 3; ++ring)
			{
				float t = ring / 3.0f;
				float r = a.Radius * t * (0.9f + 0.1f * std::sin(rt.Time * 2.0f + ring));
				glm::vec4 halo{core.r, core.g, core.b, 0.08f * (1.0f - t)};
				DrawQuadCentered(renderer, a.Pos, {r * 2.0f, 0.004f}, halo, ZAttractors);
				DrawQuadCentered(renderer, a.Pos, {0.004f, r * 2.0f}, halo, ZAttractors);
			}
		}
	}

	void UISystem(Aether::QueryRes& res)
	{
		auto world = res.World();
		auto& params = world.GetResourceMut<SimParams>();
		auto& rt = world.GetResourceMut<SimRuntime>();
		auto& particles = world.GetResourceMut<Particles>();
		auto& trails = world.GetResourceMut<Trails>();
		auto& attractors = world.GetResourceMut<Attractors>();
		const auto& sys = world.GetResource<SystemHandles>();
		const auto& stats = world.GetResource<Lumen::Renderer2D::Stats>();

		// --- Stats -----------------------------------------------------
		ImGui::Begin("Glimmer — Stats");
		ImGui::Text("Draw Calls: %" PRIu32, stats.DrawCalls);
		ImGui::Text("Quads:      %" PRIu32, stats.QuadCount);
		ImGui::Separator();
		ImGui::Text("Time:       %.2f s", rt.Time);
		ImGui::Text("Particles:  %zu / %zu", particles.AliveCount, Particles::MaxAlive);
		ImGui::Text("Trails:     %zu / %zu", trails.Count, Trails::MaxPoints);
		ImGui::Text("Attractors: %zu", attractors.Items.size());
		ImGui::Text("Cursor NDC: %.2f, %.2f", rt.CursorNdc.x, rt.CursorNdc.y);
		ImGui::Text("Status:     %s", rt.Paused ? "PAUSED" : "Running");
		ImGui::End();

		// --- Systems ---------------------------------------------------
		ImGui::Begin("Glimmer — Systems");
		ImGui::TextDisabled("Toggle individual systems");
		ImGui::Separator();
		SystemToggle("SimInput",          sys.SimInput);
		SystemToggle("ClearColorAnim",    sys.ClearColorAnim);
		SystemToggle("Physics",           sys.Physics);
		SystemToggle("AttractorForce",    sys.AttractorForce);
		SystemToggle("Swirl",             sys.Swirl);
		SystemToggle("Lifetime",          sys.Lifetime);
		SystemToggle("TrailEmit",         sys.TrailEmit);
		ImGui::Separator();
		SystemToggle("RenderBackdrop",    sys.RenderBackdrop);
		SystemToggle("RenderTrails",      sys.RenderTrails);
		SystemToggle("RenderParticles",   sys.RenderParticles);
		SystemToggle("RenderAttractors",  sys.RenderAttractors);
		ImGui::End();

		// --- Controls --------------------------------------------------
		ImGui::Begin("Glimmer — Controls");

		if (ImGui::Button(rt.Paused ? "Resume" : "Pause")) rt.Paused = !rt.Paused;
		ImGui::SameLine();
		if (ImGui::Button("Clear Particles")) particles.AliveCount = 0;
		ImGui::SameLine();
		if (ImGui::Button("Clear Attractors")) attractors.Items.clear();
		if (ImGui::Button("Clear Trails")) { trails.Count = 0; trails.Head = 0; }
		ImGui::SameLine();
		if (ImGui::Button("Burst at Center"))
			SpawnBurst(particles, params, rt, {0.0f, 0.0f}, params.BurstCount * 3);

		ImGui::SliderFloat("Time Scale", &rt.DtScale, 0.0f, 3.0f, "%.2fx");

		if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::DragFloat2("Gravity", &params.Gravity.x, 0.01f, -5.0f, 5.0f);
			ImGui::SliderFloat("Damping", &params.Damping, 0.8f, 1.0f);
			ImGui::SliderFloat("Max Speed", &params.MaxSpeed, 0.1f, 10.0f);
			ImGui::Checkbox("Bounce Bounds", &params.BounceBounds);
			ImGui::SameLine();
			ImGui::Checkbox("Wrap Bounds", &params.WrapBounds);
			ImGui::SliderFloat("Bound Extent", &params.BoundExtent, 0.1f, 2.0f);
			ImGui::SliderFloat("Restitution", &params.Restitution, 0.0f, 1.0f);
		}

		if (ImGui::CollapsingHeader("Swirl"))
		{
			ImGui::SliderFloat("Swirl Strength", &params.SwirlStrength, -10.0f, 10.0f);
			ImGui::SliderFloat("Swirl Radius", &params.SwirlRadius, 0.1f, 2.0f);
		}

		if (ImGui::CollapsingHeader("Spawn", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SliderInt("Burst Count", &params.BurstCount, 1, 200);
			ImGui::SliderFloat("Spawn Speed", &params.SpawnSpeed, 0.0f, 5.0f);
			ImGui::SliderFloat("Speed Variance", &params.SpawnSpeedVariance, 0.0f, 3.0f);
			ImGui::SliderFloat("Lifetime", &params.ParticleLifetime, 0.1f, 10.0f);
			ImGui::SliderFloat("Lifetime Variance", &params.LifetimeVariance, 0.0f, 5.0f);
			ImGui::SliderFloat("Particle Size", &params.ParticleSize, 0.001f, 0.08f);
			ImGui::SliderFloat("Size Variance", &params.ParticleSizeVariance, 0.0f, 0.05f);
			ImGui::SliderFloat("Hue Base", &params.HueBase, 0.0f, 1.0f);
			ImGui::SliderFloat("Hue Range", &params.HueRange, 0.0f, 1.0f);
			ImGui::SliderFloat("Saturation", &params.Saturation, 0.0f, 1.0f);
			ImGui::SliderFloat("Value", &params.Value, 0.0f, 1.0f);
			ImGui::Checkbox("Rainbow Over Time", &params.RainbowOverTime);
			ImGui::Checkbox("Spawn On Drag", &params.SpawnOnDrag);
			ImGui::SliderFloat("Drag Spawn Rate", &params.DragSpawnRate, 0.0f, 2000.0f, "%.0f / s");
		}

		if (ImGui::CollapsingHeader("Trails"))
		{
			ImGui::Checkbox("Trails Enabled", &params.TrailsEnabled);
			ImGui::SliderFloat("Sample Rate", &params.TrailSampleRate, 0.0f, 300.0f);
			ImGui::SliderFloat("Trail Lifetime", &params.TrailLifetime, 0.05f, 3.0f);
			ImGui::SliderFloat("Trail Size Factor", &params.TrailSizeFactor, 0.0f, 2.0f);
		}

		if (ImGui::CollapsingHeader("Attractors"))
		{
			ImGui::SliderFloat("Default Strength", &params.AttractorDefaultStrength, 0.0f, 10.0f);
			ImGui::SliderFloat("Default Radius", &params.AttractorDefaultRadius, 0.05f, 2.0f);
			ImGui::SliderFloat("Epsilon", &params.AttractorEpsilon, 0.001f, 0.2f);
			ImGui::Checkbox("Show Gizmos", &params.ShowAttractorGizmos);

			int idx = 0;
			for (auto it = attractors.Items.begin(); it != attractors.Items.end(); )
			{
				ImGui::PushID(idx);
				ImGui::Text("[%d] %s", idx, it->Repel ? "Repel" : "Attract");
				ImGui::SameLine();
				if (ImGui::SmallButton("X")) { it = attractors.Items.erase(it); ImGui::PopID(); ++idx; continue; }
				ImGui::SameLine();
				if (ImGui::SmallButton(it->Repel ? "-> Attract" : "-> Repel"))
					it->Repel = !it->Repel;
				ImGui::SliderFloat("Strength", &it->Strength, 0.0f, 10.0f);
				ImGui::SliderFloat("Radius", &it->Radius, 0.05f, 2.0f);
				ImGui::DragFloat2("Position", &it->Pos.x, 0.005f, -1.5f, 1.5f);
				ImGui::Separator();
				ImGui::PopID();
				++it;
				++idx;
			}
		}

		if (ImGui::CollapsingHeader("Backdrop"))
		{
			ImGui::SliderInt("Grid Size", &params.BackdropGridSize, 2, 80);
			ImGui::SliderFloat("Quad Size", &params.BackdropQuadSize, 0.005f, 0.1f);
			ImGui::SliderFloat("Gap", &params.BackdropGap, 0.0f, 0.05f);
			ImGui::SliderFloat("Intensity", &params.BackdropIntensity, 0.0f, 2.0f);
			ImGui::SliderFloat("Time Scale", &params.BackdropTimeScale, 0.0f, 4.0f);
		}

		if (ImGui::CollapsingHeader("Clear Color"))
		{
			ImGui::Checkbox("Animate", &params.ClearColorAnim);
			ImGui::ColorEdit4("Base Color", &params.BaseClearColor.x);
		}

		ImGui::End();

		// --- Help ------------------------------------------------------
		ImGui::Begin("Glimmer — Help");
		ImGui::TextWrapped("LMB: burst spawn particles (hold to stream)");
		ImGui::TextWrapped("RMB: place attractor");
		ImGui::TextWrapped("Shift+RMB: place repeller");
		ImGui::TextWrapped("MMB: remove nearest attractor");
		ImGui::Separator();
		ImGui::TextWrapped("P / Space: pause");
		ImGui::TextWrapped("R: clear particles");
		ImGui::TextWrapped("C: clear attractors");
		ImGui::End();
	}
}

void Glimmer::Build(Core::Application& app)
{
	LM_PROFILE_FUNCTION();

	auto& world = app.GetWorld();

	// Resources
	world.SetResource<SimParams>({});
	world.SetResource<SimRuntime>({});
	world.SetResource<Particles>({});
	world.SetResource<Trails>({});
	world.SetResource<Attractors>({});

	SystemHandles handles;

	// ----- Input phase -------------------------------------------------
	{
		auto b = world.System("Glimmer::SimInput");
		b.SetPhase<Aether::Phases::OnUpdate>()
		 .Read<Flux::KeyboardState>().Read<Flux::MouseState>()
		 .Read<Flux::WindowResource>().Read<Glyph::ImGuiIOState>()
		 .Write<SimParams>().Write<SimRuntime>()
		 .Write<Particles>().Write<Attractors>();
		handles.SimInput = b.Run(SimInputSystem);
	}

	// ----- Update phase ------------------------------------------------
	{
		auto b = world.System("Glimmer::ClearColorAnim");
		b.SetPhase<Aether::Phases::OnUpdate>()
		 .Read<Lumen::RenderDeviceResource>()
		 .Read<SimParams>().Read<SimRuntime>();
		handles.ClearColorAnim = b.Run(ClearColorAnimSystem);
	}
	{
		auto b = world.System("Glimmer::Physics");
		b.SetPhase<Aether::Phases::OnUpdate>()
		 .Read<SimParams>().Write<SimRuntime>().Write<Particles>();
		handles.Physics = b.Run(PhysicsSystem);
	}
	{
		auto b = world.System("Glimmer::AttractorForce");
		b.SetPhase<Aether::Phases::OnUpdate>()
		 .Read<SimParams>().Read<Attractors>()
		 .Write<SimRuntime>().Write<Particles>();
		handles.AttractorForce = b.Run(AttractorForceSystem);
	}
	{
		auto b = world.System("Glimmer::Swirl");
		b.SetPhase<Aether::Phases::OnUpdate>()
		 .Read<SimParams>().Write<SimRuntime>().Write<Particles>();
		handles.Swirl = b.Run(SwirlSystem);
	}

	// ----- PostUpdate phase --------------------------------------------
	{
		auto b = world.System("Glimmer::Lifetime");
		b.SetPhase<Aether::Phases::PostUpdate>()
		 .Write<SimRuntime>().Write<Particles>();
		handles.Lifetime = b.Run(LifetimeSystem);
	}
	{
		auto b = world.System("Glimmer::TrailEmit");
		b.SetPhase<Aether::Phases::PostUpdate>()
		 .Read<SimParams>().Read<Particles>()
		 .Write<SimRuntime>().Write<Trails>();
		handles.TrailEmit = b.Run(TrailEmitSystem);
	}

	// ----- Render phase ------------------------------------------------
	{
		auto b = world.System("Glimmer::RenderBackdrop");
		b.SetPhase<Aether::Phases::OnRender>()
		 .Read<SimParams>().Read<SimRuntime>().Write<Lumen::Renderer2D>();
		handles.RenderBackdrop = b.Run(RenderBackdropSystem);
	}
	{
		auto b = world.System("Glimmer::RenderTrails");
		b.SetPhase<Aether::Phases::OnRender>()
		 .Read<Trails>().Write<Lumen::Renderer2D>();
		handles.RenderTrails = b.Run(RenderTrailsSystem);
	}
	{
		auto b = world.System("Glimmer::RenderParticles");
		b.SetPhase<Aether::Phases::OnRender>()
		 .Read<Particles>().Read<SimRuntime>().Write<Lumen::Renderer2D>();
		handles.RenderParticles = b.Run(RenderParticlesSystem);
	}
	{
		auto b = world.System("Glimmer::RenderAttractors");
		b.SetPhase<Aether::Phases::OnRender>()
		 .Read<SimParams>().Read<Attractors>().Read<SimRuntime>()
		 .Write<Lumen::Renderer2D>();
		handles.RenderAttractors = b.Run(RenderAttractorsSystem);
	}

	// ----- UI phase ----------------------------------------------------
	{
		auto b = world.System("Glimmer::UI");
		b.SetPhase<Aether::Phases::OnUI>()
		 .Read<Lumen::Renderer2D::Stats>()
		 .Write<SimParams>().Write<SimRuntime>()
		 .Write<Particles>().Write<Trails>().Write<Attractors>();
		handles.UI = b.Run(UISystem);
	}

	world.SetResource<SystemHandles>(std::move(handles));
}

void Glimmer::Cleanup(Core::Application&)
{
}

void Glimmer::AddDependencies(Core::DependencyList& dependencies)
{
	dependencies.Require<Flux::WindowPlugin>();
	dependencies.Require<Lumen::RendererPlugin>();
	dependencies.Require<Lumen::Renderer2DPlugin>();
	dependencies.Require<Glyph::ImGuiPlugin>();
}
