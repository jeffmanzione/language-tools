#ifndef COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_INTERN_H_
#define COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_INTERN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "intern/intern.h"

DEFINE_INTERN_POOL(GlobalStringInternPool, char);

void global_string_intern_pool_init();
void global_string_intern_pool_finalize();

const char *global_intern(const char text[]);
const char *global_intern_range(const char text[], int start, int len);

#ifdef __cplusplus
}
#endif

#endif /* COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_INTERN_H_ */