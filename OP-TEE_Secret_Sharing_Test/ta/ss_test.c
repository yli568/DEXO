/*
 * Copyright (c) 2016, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <ss_test_ta.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <time.h>

#include "d_string.h"
#include "shamir.h"
/*
 * Called when the instance of the TA is created. This is the first call in
 * the TA.
 */
TEE_Result TA_CreateEntryPoint(void) { return TEE_SUCCESS; }

/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void TA_DestroyEntryPoint(void) {}

/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA. In this function you will normally do the global initialization for the
 * TA.
 */
TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
                                    TEE_Param __maybe_unused params[4],
                                    void __maybe_unused **sess_ctx) {
  uint32_t exp_param_types =
      TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE,
                      TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);
  if (param_types != exp_param_types) return TEE_ERROR_BAD_PARAMETERS;

  (void)&params;
  (void)&sess_ctx;

  return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void TA_CloseSessionEntryPoint(void __maybe_unused *sess_ctx) {
  (void)&sess_ctx;
}

static int prime = 257;

int modular_exponentiation(int base, int exp, int mod) {
  if (exp == 0) {
    return 1;
  } else if (exp % 2 == 0) {
    int mysqrt = modular_exponentiation(base, exp / 2, mod);
    return (mysqrt * mysqrt) % mod;
  } else {
    return (base * modular_exponentiation(base, exp - 1, mod)) % mod;
  }
}

int *split_number(int number, int n, int t) {
  int *shares = malloc(sizeof(int) * n);

  int *coef = malloc(sizeof(int) * t);
  int x;
  int i;

  coef[0] = number;

  for (i = 1; i < t; ++i) {
    /* Generate random coefficients -- use arc4random if available */
#ifdef HAVE_ARC4RANDOM
    coef[i] = arc4random_uniform(prime);
#else
    coef[i] = rand() % (prime);
#endif
  }

  for (x = 0; x < n; ++x) {
    int y = coef[0];

    /* Calculate the shares */
    for (i = 1; i < t; ++i) {
      int temp = modular_exponentiation(x + 1, i, prime);

      y = (y + (coef[i] * temp % prime)) % prime;
    }

    /* Sometimes we're getting negative numbers, and need to fix that */
    y = (y + prime) % prime;

    shares[x] = y;
  }

  free(coef);

  return shares;
}

char **split_string(char *secret, int n, int t) {
  int len = strlen(secret);

  char **shares = malloc(sizeof(char *) * n);
  int i;

  for (i = 0; i < n; ++i) {
    /* need two characters to encode each character */
    /* Need 4 character overhead for share # and quorum # */
    /* Additional 2 characters are for compatibility with:

            http://www.christophedavid.org/w/c/w.php/Calculators/ShamirSecretSharing
    */
    shares[i] = (char *)malloc(2 * len + 6 + 1);

    sprintf(shares[i], "%02X%02XAA", (i + 1), t);
  }

  /* Now, handle the secret */

  for (i = 0; i < len; ++i) {
    int letter = secret[i];  // - '0';

    if (letter < 0) {
      letter = 256 + letter;
    }

    int *chunks = split_number(letter, n, t);
    int j;

    for (j = 0; j < n; ++j) {
      if (chunks[j] == 256) {
        sprintf(shares[j] + 6 + i * 2, "G0"); /* Fake code */
      } else {
        sprintf(shares[j] + 6 + i * 2, "%02X", chunks[j]);
      }
    }

    free(chunks);
  }

  return shares;
}

void free_string_shares(char **shares, int n) {
  int i;

  for (i = 0; i < n; ++i) {
    free(shares[i]);
  }

  free(shares);
}

char *generate_share_strings(char *secret, int n, int t) {
  char **result = split_string(secret, n, t);

  int len = strlen(secret);
  int key_len = 6 + 2 * len + 1;
  int i;

  char *shares = malloc(key_len * n + 1);

  for (i = 0; i < n; ++i) {
    sprintf(shares + i * key_len, "%s\n", result[i]);
  }

  free_string_shares(result, n);

  return shares;
}

static TEE_Result ss_test(uint32_t param_types, TEE_Param params[4], int n,
                          int l) {
  uint32_t exp_param_types =
      TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE,
                      TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);

  if (param_types != exp_param_types) return TEE_ERROR_BAD_PARAMETERS;
  int t = (n * 2 + 2) / 3;
  char *str = (char *)malloc(l + 1);
  memset(str, '0', l);
  str[l] = '\0';
  char *shares = generate_share_strings(str, n, t);
  // use shares. pass
  free(shares);
  free(str);
  return TEE_SUCCESS;
}

static TEE_Result ss_test_half(uint32_t param_types, TEE_Param params[4], int n,
                               int l) {
  uint32_t exp_param_types =
      TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE,
                      TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);

  if (param_types != exp_param_types) return TEE_ERROR_BAD_PARAMETERS;
  int t = (n + 1) / 2;

  char *str = (char *)malloc(l + 1);
  memset(str, '0', l);
  str[l] = '\0';

  char *shares = generate_share_strings(str, n, t);
  // use shares. pass
  free(shares);
  free(str);
  return TEE_SUCCESS;
}

/*
 * Called when a TA is invoked. sess_ctx hold that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the paramters
 * comes from normal world.
 */
TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
                                      uint32_t cmd_id, uint32_t param_types,
                                      TEE_Param params[4]) {
  (void)&sess_ctx; /* Unused parameter */

  switch (cmd_id) {
    case TA_SS_TEST_5:
      return ss_test(param_types, params, 5, 1000);
    case TA_SS_TEST_10:
      return ss_test(param_types, params, 10, 1000);
    case TA_SS_TEST_20:
      return ss_test(param_types, params, 20, 1000);
    case TA_SS_TEST_30:
      return ss_test(param_types, params, 30, 1000);
    case TA_SS_TEST_40:
      return ss_test(param_types, params, 40, 1000);
    case TA_SS_TEST_50:
      return ss_test(param_types, params, 50, 1000);
    case TA_SS_TEST_5_half:
      return ss_test_half(param_types, params, 5, 1000);
    case TA_SS_TEST_10_half:
      return ss_test_half(param_types, params, 10, 1000);
    case TA_SS_TEST_20_half:
      return ss_test_half(param_types, params, 20, 1000);
    case TA_SS_TEST_30_half:
      return ss_test_half(param_types, params, 30, 1000);
    case TA_SS_TEST_40_half:
      return ss_test_half(param_types, params, 40, 1000);
    case TA_SS_TEST_50_half:
      return ss_test_half(param_types, params, 50, 1000);
    case TA_SS_TEST_5_100bytes:
      return ss_test(param_types, params, 5, 100);
    case TA_SS_TEST_10_100bytes:
      return ss_test(param_types, params, 10, 100);
    case TA_SS_TEST_20_100bytes:
      return ss_test(param_types, params, 20, 100);
    case TA_SS_TEST_30_100bytes:
      return ss_test(param_types, params, 30, 100);
    case TA_SS_TEST_40_100bytes:
      return ss_test(param_types, params, 40, 100);
    case TA_SS_TEST_50_100bytes:
      return ss_test(param_types, params, 50, 100);
    case TA_SS_TEST_5_half_100bytes:
      return ss_test_half(param_types, params, 5, 100);
    case TA_SS_TEST_10_half_100bytes:
      return ss_test_half(param_types, params, 10, 100);
    case TA_SS_TEST_20_half_100bytes:
      return ss_test_half(param_types, params, 20, 100);
    case TA_SS_TEST_30_half_100bytes:
      return ss_test_half(param_types, params, 30, 100);
    case TA_SS_TEST_40_half_100bytes:
      return ss_test_half(param_types, params, 40, 100);
    case TA_SS_TEST_50_half_100bytes:
      return ss_test_half(param_types, params, 50, 100);
    case TA_SS_TEST_5_1000bytes:
      return ss_test(param_types, params, 5, 1000);
    case TA_SS_TEST_10_1000bytes:
      return ss_test(param_types, params, 10, 1000);
    case TA_SS_TEST_20_1000bytes:
      return ss_test(param_types, params, 20, 1000);
    case TA_SS_TEST_30_1000bytes:
      return ss_test(param_types, params, 30, 1000);
    case TA_SS_TEST_40_1000bytes:
      return ss_test(param_types, params, 40, 1000);
    case TA_SS_TEST_50_1000bytes:
      return ss_test(param_types, params, 50, 1000);
    case TA_SS_TEST_5_half_1000bytes:
      return ss_test_half(param_types, params, 5, 1000);
    case TA_SS_TEST_10_half_1000bytes:
      return ss_test_half(param_types, params, 10, 1000);
    case TA_SS_TEST_20_half_1000bytes:
      return ss_test_half(param_types, params, 20, 1000);
    case TA_SS_TEST_30_half_1000bytes:
      return ss_test_half(param_types, params, 30, 1000);
    case TA_SS_TEST_40_half_1000bytes:
      return ss_test_half(param_types, params, 40, 1000);
    case TA_SS_TEST_50_half_1000bytes:
      return ss_test_half(param_types, params, 50, 1000);
    case TA_SS_TEST_5_10000bytes:
      return ss_test(param_types, params, 5, 10000);
    case TA_SS_TEST_10_10000bytes:
      return ss_test(param_types, params, 10, 10000);
    case TA_SS_TEST_20_10000bytes:
      return ss_test(param_types, params, 20, 10000);
    case TA_SS_TEST_30_10000bytes:
      return ss_test(param_types, params, 30, 10000);
    case TA_SS_TEST_40_10000bytes:
      return ss_test(param_types, params, 40, 10000);
    case TA_SS_TEST_50_10000bytes:
      return ss_test(param_types, params, 50, 10000);
    case TA_SS_TEST_5_half_10000bytes:
      return ss_test_half(param_types, params, 5, 10000);
    case TA_SS_TEST_10_half_10000bytes:
      return ss_test_half(param_types, params, 10, 10000);
    case TA_SS_TEST_20_half_10000bytes:
      return ss_test_half(param_types, params, 20, 10000);
    case TA_SS_TEST_30_half_10000bytes:
      return ss_test_half(param_types, params, 30, 10000);
    case TA_SS_TEST_40_half_10000bytes:
      return ss_test_half(param_types, params, 40, 10000);
    case TA_SS_TEST_50_half_10000bytes:
      return ss_test_half(param_types, params, 50, 10000);
    default:
      return TEE_ERROR_BAD_PARAMETERS;
  }
}
