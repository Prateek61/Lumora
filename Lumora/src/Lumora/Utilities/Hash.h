#pragma once

#include <cstdint>

namespace Lumora::Hash
{
	constexpr uint64_t FNV1a(const char* str, size_t length)
	{
		if (str == nullptr || length == 0)
			return 0;

		uint64_t h = 14695981039346656037ULL; // FNV offset basis
		for (size_t i = 0; i < length; ++i)
		{
			// FNV-1a hashes octets. Plain char would sign-extend anything above 0x7F.
			h ^= static_cast<uint64_t>(static_cast<unsigned char>(str[i]));
			h *= 1099511628211ULL; // FNV prime
		}

		return h ? h : 1; // Ensure non-zero hash
	}

	template <size_t N>
	constexpr uint64_t FNV1a(const char(&str)[N])
	{
		static_assert(N > 1, "String literal must not be empty");

		uint64_t h = 14695981039346656037ULL; // FNV offset basis
		for (size_t i = 0; i < N - 1; ++i) // Exclude null terminator
		{
			// FNV-1a hashes octets. Plain char would sign-extend anything above 0x7F.
			h ^= static_cast<uint64_t>(static_cast<unsigned char>(str[i]));
			h *= 1099511628211ULL; // FNV prime
		}

		return h ? h : 1; // Ensure non-zero hash
	}
}