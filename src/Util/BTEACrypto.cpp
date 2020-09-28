#include "engge/Util/BTEACrypto.hpp"

namespace ng {
#define DELTA 0x9e3779b9
#define MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (key[(p&3)^e] ^ z)))

void BTEACrypto::encrypt(uint32_t *v, int n, const uint32_t *key) {
  btea(v, n, key);
}

void BTEACrypto::decrypt(uint32_t *v, int n, const uint32_t *key) {
  btea(v, -n, key);
}

// This method comes from https://en.wikipedia.org/wiki/XXTEA
void BTEACrypto::btea(uint32_t *v, int n, const uint32_t *key) {
  uint32_t y, z, sum;
  unsigned p, rounds, e;
  if (n > 1) {          /* Coding Part */
    rounds = 6 + 52 / n;
    sum = 0;
    z = v[n - 1];
    do {
      sum += DELTA;
      e = (sum >> 2) & 3;
      for (p = 0; p < static_cast<uint32_t>(n - 1); p++) {
        y = v[p + 1];
        z = v[p] += MX;
      }
      y = v[0];
      z = v[n - 1] += MX;
    } while (--rounds);
  } else if (n < -1) {  /* Decoding Part */
    n = -n;
    rounds = 6 + 52 / n;
    sum = rounds * DELTA;
    y = v[0];
    do {
      e = (sum >> 2) & 3;
      for (p = n - 1; p > 0; p--) {
        z = v[p - 1];
        y = v[p] -= MX;
      }
      z = v[n - 1];
      y = v[0] -= MX;
      sum -= DELTA;
    } while (--rounds);
  }
}
}
