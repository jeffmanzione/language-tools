#include "language-tools/intern.h"

IMPL_INTERN_POOL(GlobalStringInternPool, char);

static GlobalStringInternPool global_intern_pool_;

// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
#define FNV_32_PRIME (0x01000193)
#define FNV_1A_32_OFFSET (0x811C9DC5)

// Is true if any bytes in the provided 32-bit integer are equivalent to 0x00.
#define HAS_NULL(x)                                      \
  (((x & 0x000000FF) == 0) || ((x & 0x0000FF00) == 0) || \
   ((x & 0x00FF0000) == 0) || ((x & 0xFF000000) == 0))

uint32_t hash_string_(const char *ptr, uint32_t size) {
  unsigned char *s = (unsigned char *)ptr;
  uint32_t hval = FNV_1A_32_OFFSET;
  for (uint32_t i = 0; i < size; ++i) {
    hval *= FNV_32_PRIME;
    hval ^= (uint32_t)*s++;
  }
  return hval;
}

int32_t compare_strings_(const char *ptr1, uint32_t size1, const char *ptr2,
                         uint32_t size2) {
  if (ptr1 == ptr2) {
    return 0;
  }
  if (NULL == ptr1) {
    return -1;
  }
  if (NULL == ptr2) {
    return 1;
  }
  return memcmp(ptr1, ptr2, size1 > size2 ? size1 : size2);
}

void global_string_intern_pool_init() {
  GlobalStringInternPool_init(&global_intern_pool_, /*threadsafe=*/false,
                              hash_string_, compare_strings_);
}

void global_string_intern_pool_finalize() {
  GlobalStringInternPool_finalize(&global_intern_pool_);
}

const char *global_intern(const char text[]) {
  return GlobalStringInternPool_intern(&global_intern_pool_, text,
                                       strlen(text) + 1);
}

const char *global_intern_range(const char text[], int start, int len) {
  char *cpy = strndup(text + start, len);
  const char *interned =
      GlobalStringInternPool_intern(&global_intern_pool_, cpy, len + 1);
  free(cpy);
  return interned;
}