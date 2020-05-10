#pragma once
#include <stdint.h>

namespace ng {
class BTEACrypto {
public:
  static void encrypt(uint32_t *v, int n, const uint32_t *k);
  static void decrypt(uint32_t *v, int n, const uint32_t *k);

private:
  static void btea(uint32_t *v, int n, const uint32_t *k);
};
}