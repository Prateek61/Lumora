#pragma once

#include "Lumora/Aether/Base.h"
#include "Lumora/Utilities/TimeStep.h"

namespace Lumora::Aether
{
	template <typename T>
	struct Field
	{
		Field(flecs::field<T>&& field);
		T& operator[](size_t index) const;
		T& operator*() const;

	private:
		flecs::field<T> m_Field;
	};

	/// Wrapper around flecs::iter
	class QueryRes
	{
	public:
		QueryRes(flecs::iter& iter);

		QueryRes(const QueryRes&) = delete;
		QueryRes& operator=(const QueryRes&) = delete;

		// Accessors
		World World() const;
		TimeStep DeltaTime() const;
		TimeStep DeltaSystemTime() const;

		// Iteration

		bool Next() const;
		template <typename T>
		Field<T> Field(int8_t index) const;
		auto begin() const { return m_Iter.begin(); }
		auto end() const { return m_Iter.end(); }
		size_t Count() const { return m_Iter.count(); }

	private:
		flecs::iter& m_Iter;
	};
}

// Inline/Template Implementations

#include "Lumora/Aether/World.h"

namespace Lumora::Aether
{
	template <typename T>
	Field<T>::Field(flecs::field<T>&& field) : m_Field(std::move(field)) {}

	template <typename T>
	T& Field<T>::operator[](size_t index) const
	{
		return m_Field[index];
	}

	template <typename T>
	T& Field<T>::operator*() const
	{
		return *m_Field;
	}


	inline QueryRes::QueryRes(flecs::iter& iter) : m_Iter(iter) {}

	inline World QueryRes::World() const
	{
		return Aether::World{m_Iter.world()};
	}

	inline TimeStep QueryRes::DeltaTime() const
	{
		return {m_Iter.delta_time()};
	}

	inline TimeStep QueryRes::DeltaSystemTime() const
	{
		return {m_Iter.delta_system_time()};
	}

	inline bool QueryRes::Next() const
	{
		return m_Iter.next();
	}

	template <typename T>
	Field<T> QueryRes::Field(int8_t index) const
	{
		return Aether::Field<T>{
			m_Iter.field<T>(index)
		};
	}
}
