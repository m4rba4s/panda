#include "board/crypto/rsa.h"

const RSAPublicKey debug_rsa_key = {
  .len = RSANUMWORDS,
  .n0inv = 0,
  .n = {0},
  .rr = {0},
  .exponent = 65537,
};

const RSAPublicKey release_rsa_key = {
  .len = RSANUMWORDS,
  .n0inv = 0,
  .n = {0},
  .rr = {0},
  .exponent = 65537,
};
