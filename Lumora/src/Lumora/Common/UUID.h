#pragma once

#include <cstdint>

namespace Lumora
{
	class UUID
	{
	public:
		UUID()
			: m_UUID(0)
		{
		}

		UUID(uint64_t uuid)
			: m_UUID(uuid)
		{
		}

		UUID(const UUID&) = default;

		operator uint64_t() const { return m_UUID; }

		static UUID Generate();

	private:
		uint64_t m_UUID;
	};
}

namespace std
{
	template <typename T>
	struct hash;

	template <>
	struct hash<Lumora::UUID>
	{
		size_t operator()(const Lumora::UUID& uuid) const noexcept
		{
			return uuid;
		}
	};
}
