#pragma once

#include <cstdint>

namespace zipfs {

	enum class OVERWRITE : uint32_t {
		NEVER, ALWAYS,
		IF_DATE_OLDER, IF_SIZE_MISMATCH, IF_DATE_OLDER_AND_SIZE_MISMATCH
	};

	enum class ORPHAN : uint32_t {
		KEEP, DELETE_ //.>underscore cos winnt.h ...
	};

	enum class QUERY_RESULT : uint32_t {
		NONE					= 0x00000000,
		FILE_WRITE				= 0x00000001,
		FILE_OVERWRITE			= 0x00000002,
		FILE_DONT_OVERWRITE		= 0x00000004,
		FILE_ORPHAN_KEEP		= 0x00000008,
		FILE_ORPHAN_DELETE		= 0x00000010,
		FILE_IS_BUT_NOT_FILE	= 0x00000020,
		DIR_ADD					= 0x00000040,
		DIR_ALREADY_EXISTS		= 0x00000080,
		DIR_ORPHAN_KEEP			= 0x00000100,
		DIR_ORPHAN_DELETE		= 0x00000200,
		DIR_IS_BUT_NOT_DIR		= 0x00000400,
		DISCARD					= 0x00000800,

		W						= FILE_WRITE | FILE_OVERWRITE | FILE_ORPHAN_DELETE | DIR_ADD | DIR_ORPHAN_DELETE, /*modifications*/
		D						= DISCARD, /*discard*/
		C						= FILE_IS_BUT_NOT_FILE | DIR_IS_BUT_NOT_DIR, /*conflicts*/
		ALL						= 0xffffffff
	};

	inline bool operator&(const QUERY_RESULT qr, uint32_t what) {
		return ((uint32_t)qr & what) != 0;
	}

	inline uint32_t operator|(const QUERY_RESULT qrl, const QUERY_RESULT qrr) {
		return (uint32_t)qrl | (uint32_t)qrr;
	}

	inline uint32_t operator|(uint32_t qrl, const QUERY_RESULT qrr) {
		return qrl | (uint32_t)qrr;
	}
}