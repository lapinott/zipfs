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
		FILE_WRITE, FILE_OVERWRITE, FILE_DONT_OVERWRITE,
		FILE_ORPHAN_KEEP, FILE_ORPHAN_DELETE,
		DIR_ADD, DIR_ALREADY_EXISTS,
		DIR_ORPHAN_KEEP, DIR_ORPHAN_DELETE
	};

	enum class QUERY_RESULTS_GET : uint32_t {
		FW		= 0x00000001,	//FILE_WRITE
		FO		= 0x00000002,	//FILE_OVERWRITE
		FDO		= 0x00000004,	//FILE_DONT_OVERWRITE
		FOK		= 0x00000008,	//FILE_ORPHAN_KEEP
		FOD		= 0x00000010,	//FILE_ORPHAN_DELETE
		DA		= 0x00000020,	//DIR_ADD
		DAE		= 0x00000040,	//DIR_ALREADY_EXISTS
		DOK		= 0x00000080,	//DIR_ORPHAN_KEEP
		DOD		= 0x00000100,	//DIR_ORPHAN_DELETE

		W		= FW | FO | FOD | DA | DOD,
		ALL		= FW | FO | FDO | FOK | FOD | DA | DAE | DOK | DOD
	};

	inline bool operator&(const QUERY_RESULT qr, const QUERY_RESULTS_GET what) {
		return ((uint32_t)qr & (uint32_t)what) != 0;
	}
}