#pragma once
#include "api.h"

#ifdef DBG_ASSERT
#include <assert.h>
constexpr bool DBG_ASSERT_V = true;
#else
constexpr bool DBG_ASSERT_V = false;
#define assert(x) {}
#endif // DBASSERT

#ifdef DBG_GL
constexpr bool DBG_GL_V = true;
#else
constexpr bool DBG_GL_V = false;
#endif // DBG_GL

namespace glDebug {
  void GLAPIENTRY glDebugOutput(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam);
}
