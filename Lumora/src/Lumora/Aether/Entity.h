#pragma once

#include "Lumora/Aether/Base.h"

namespace Lumora::Aether
{
	struct Entity
	{
		Entity() = default;
		explicit Entity(const flecs::entity& e): m_Entity(e) {}
		explicit Entity(flecs::entity&& e)
		    : m_Entity(std::move(e)) {}

		// Accessors
		template <typename T>
		const T& Get() const { return m_Entity.get<T>(); }
		template <typename T>
		T& GetMut() const { return m_Entity.get_mut<T>(); }
		template <typename T>
		const T* TryGet() const { return m_Entity.try_get<T>(); }
		template <typename T>
		T* TryGetMut() { return m_Entity.try_get_mut<T>(); }

		// Modifiers
		template <typename T, typename... Args>
		Entity& Set(Args&&... args) { m_Entity.set<T>(std::forward<Args>(args)...); return *this; }
		template <typename T>
		Entity& Set(T&& val) { m_Entity.set(val); return *this; }
		template <typename T, typename... Args>
		Entity& Emplace(Args&&... args) { m_Entity.emplace<T>(std::forward<Args>(args)...); return *this; }
		template <typename T>
		Entity& Add() { m_Entity.add<T>(); return *this; }
		template <typename T>
		Entity& Remove() { m_Entity.remove<T>(); return *this; }
		Entity& ChildOf(const Entity& parent) { m_Entity.child_of(parent.Raw()); return *this; }
		Entity& Clear() { m_Entity.clear(); return *this; }
		void Destruct() { m_Entity.destruct(); }

		// Utilities
		bool IsValid() const { return m_Entity.is_valid(); }
		template <typename T>
		bool Has() const { return m_Entity.has<T>(); }
		
		flecs::entity& Raw() { return m_Entity; }
		flecs::entity Raw() const { return m_Entity; }
	private:
		flecs::entity m_Entity;
	};
}