#pragma once
#include <squirrel.h>

namespace ng {
class DebugControls final {
public:
  static bool stringGetter(void *vec, int idx, const char **out_text);
  static void createTree(const char *tableKey, HSQOBJECT obj);
};
}
