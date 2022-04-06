#pragma once

#include <cassert>
#include <stdexcept>

#ifdef _DEBUG
#define zipfs_debug_assert(assertion) zipfs_always_assert(assertion)
#else
#define zipfs_debug_assert(assertion) void()
#endif

#define zipfs_always_assert(assertion) assert(assertion)


namespace zipfs {

	struct zipfs_internal_error_t : public std::logic_error {
	public:

		zipfs_internal_error_t() : std::logic_error{ "zipfs internal error." } {}
	};
}
#define zipfs_internal_assert(assertion) if (!(assertion)) throw zipfs::zipfs_internal_error_t();


namespace zipfs {

	struct zipfs_usage_error_t : public std::invalid_argument {
	public:

		zipfs_usage_error_t(const char* message) : std::invalid_argument{ message } {}
	};
}
#define zipfs_usage_assert(assertion, message) if (!(assertion)) throw zipfs::zipfs_usage_error_t(message);