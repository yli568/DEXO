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

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>

/* For the UUID (found in local file(s)) */
#include <ss_test_ta.h>

int call_ta(uint32_t cmd) {
  TEEC_Result res = TEEC_ERROR_GENERIC;
  TEEC_Context ctx = {};
  TEEC_Session sess = {};
  TEEC_UUID uuid = TA_SS_TEST_UUID;
  uint32_t err_origin = 0;
  TEEC_Operation op = {};
  // Public key exponent in big endian order
  uint32_t data = 0;
  // Modulus in big endian order
  uint32_t data_signature = 0;
  uint32_t RT_signature = 0;

  /* Initialize a context connecting us to the TEE */
  res = TEEC_InitializeContext(NULL, &ctx);
  if (res != TEEC_SUCCESS)
    errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

  /*
   * Open a session to the "attestation" PTA.
   */
  res = TEEC_OpenSession(&ctx, &sess, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL,
                         &err_origin);
  if (res != TEEC_SUCCESS)
    errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x", res,
         err_origin);

  /*
   * Execute a function in the TA by invoking it.
   */

  /* Clear the TEEC_Operation struct */
  memset(&op, 0, sizeof(op));

  /*
   * Prepare the argument.
   * TEEC_TempMemoryReference
   */
  op.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_NONE, TEEC_NONE);

  // TA_ATTESTATION_GET_DATA_SIGNATURE
  res = TEEC_InvokeCommand(&sess, cmd, &op, &err_origin);
  if (res != TEEC_SUCCESS)
    errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x", res,
         err_origin);

  // data = op.params[0].value.a;
  // data_signature = op.params[0].value.b;
  // RT_signature = op.params[1].value.b;
  // printf("Data: %u\n", data);
  // printf("Data Signature: %u\n", data_signature);
  // printf("RT Signature: %u\n", RT_signature);
  TEEC_CloseSession(&sess);
  TEEC_FinalizeContext(&ctx);
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("provide a number (size(B) of data)\n");
    return 1;
  }

  // int num = 10;
  int num = atoi(argv[1]);
  int cmd_ids[] = {TA_SS_TEST_5,       TA_SS_TEST_10,      TA_SS_TEST_20,
                   TA_SS_TEST_30,      TA_SS_TEST_40,      TA_SS_TEST_50,
                   TA_SS_TEST_5_half,  TA_SS_TEST_10_half, TA_SS_TEST_20_half,
                   TA_SS_TEST_30_half, TA_SS_TEST_40_half, TA_SS_TEST_50_half};
  clock_t start, end;
  printf("For %d B of data\n", num);
  switch (num) {
    case 10:
      for (int i = 0; i < sizeof(cmd_ids) / sizeof(cmd_ids[0]); i++) {
        start = clock();
        double time_taken;
        printf("Start for cmd_id %d\n", cmd_ids[i]);
        for (int j = 0; j < 1; j++) {
          switch (cmd_ids[i]) {
            case TA_SS_TEST_5:
              call_ta(TA_SS_TEST_5);
              break;
            case TA_SS_TEST_10:
              call_ta(TA_SS_TEST_10);
              break;
            case TA_SS_TEST_20:
              call_ta(TA_SS_TEST_20);
              break;
            case TA_SS_TEST_30:
              call_ta(TA_SS_TEST_30);
              break;
            case TA_SS_TEST_40:
              call_ta(TA_SS_TEST_40);
              break;
            case TA_SS_TEST_50:
              call_ta(TA_SS_TEST_50);
              break;
            case TA_SS_TEST_5_half:
              call_ta(TA_SS_TEST_5_half);
              break;
            case TA_SS_TEST_10_half:
              call_ta(TA_SS_TEST_10_half);
              break;
            case TA_SS_TEST_20_half:
              call_ta(TA_SS_TEST_20_half);
              break;
            case TA_SS_TEST_30_half:
              call_ta(TA_SS_TEST_30_half);
              break;
            case TA_SS_TEST_40_half:
              call_ta(TA_SS_TEST_40_half);
              break;
            case TA_SS_TEST_50_half:
              call_ta(TA_SS_TEST_50_half);
              break;
            default:
              break;
          }
        }

        end = clock();
        time_taken = ((double)end - start) / CLOCKS_PER_SEC *
                     1000.0;  // Convert to milliseconds
        printf("Time taken for cmd_id %d(1): %f ms\n", cmd_ids[i], time_taken);
      }
      // for (int i = 0; i < sizeof(cmd_ids) / sizeof(cmd_ids[0]); i++) {
      //   start = clock();
      //   double time_taken;
      //   printf("Start for cmd_id %d\n", cmd_ids[i]);
      //   for (int j = 0; j < 10; j++) {
      //     switch (cmd_ids[i]) {
      //       case TA_SS_TEST_5:
      //         call_ta(TA_SS_TEST_5);
      //         break;
      //       case TA_SS_TEST_10:
      //         call_ta(TA_SS_TEST_10);
      //         break;
      //       case TA_SS_TEST_20:
      //         call_ta(TA_SS_TEST_20);
      //         break;
      //       case TA_SS_TEST_30:
      //         call_ta(TA_SS_TEST_30);
      //         break;
      //       case TA_SS_TEST_40:
      //         call_ta(TA_SS_TEST_40);
      //         break;
      //       case TA_SS_TEST_50:
      //         call_ta(TA_SS_TEST_50);
      //         break;
      //       case TA_SS_TEST_5_half:
      //         call_ta(TA_SS_TEST_5_half);
      //         break;
      //       case TA_SS_TEST_10_half:
      //         call_ta(TA_SS_TEST_10_half);
      //         break;
      //       case TA_SS_TEST_20_half:
      //         call_ta(TA_SS_TEST_20_half);
      //         break;
      //       case TA_SS_TEST_30_half:
      //         call_ta(TA_SS_TEST_30_half);
      //         break;
      //       case TA_SS_TEST_40_half:
      //         call_ta(TA_SS_TEST_40_half);
      //         break;
      //       case TA_SS_TEST_50_half:
      //         call_ta(TA_SS_TEST_50_half);
      //         break;
      //       default:
      //         break;
      //     }
      //   }

      //   end = clock();
      //   time_taken = ((double)end - start) / CLOCKS_PER_SEC *
      //                1000.0;  // Convert to milliseconds
      //   printf("Time taken for cmd_id %d(10): %f ms\n", cmd_ids[i], time_taken);
      // }
      break;
    case 100:
      for (int i = 0; i < sizeof(cmd_ids) / sizeof(cmd_ids[0]); i++) {
        start = clock();
        double time_taken;
        printf("Start for cmd_id %d\n", cmd_ids[i]);
        for (int j = 0; j < 1; j++) {
          switch (cmd_ids[i]) {
            case TA_SS_TEST_5_100bytes:
              call_ta(TA_SS_TEST_5_100bytes);
              break;
            case TA_SS_TEST_10_100bytes:
              call_ta(TA_SS_TEST_10_100bytes);
              break;
            case TA_SS_TEST_20_100bytes:
              call_ta(TA_SS_TEST_20_100bytes);
              break;
            case TA_SS_TEST_30_100bytes:
              call_ta(TA_SS_TEST_30_100bytes);
              break;
            case TA_SS_TEST_40_100bytes:
              call_ta(TA_SS_TEST_40_100bytes);
              break;
            case TA_SS_TEST_50_100bytes:
              call_ta(TA_SS_TEST_50_100bytes);
              break;
            case TA_SS_TEST_5_half_100bytes:
              call_ta(TA_SS_TEST_5_half_100bytes);
              break;
            case TA_SS_TEST_10_half_100bytes:
              call_ta(TA_SS_TEST_10_half_100bytes);
              break;
            case TA_SS_TEST_20_half_100bytes:
              call_ta(TA_SS_TEST_20_half_100bytes);
              break;
            case TA_SS_TEST_30_half_100bytes:
              call_ta(TA_SS_TEST_30_half_100bytes);
              break;
            case TA_SS_TEST_40_half_100bytes:
              call_ta(TA_SS_TEST_40_half_100bytes);
              break;
            case TA_SS_TEST_50_half_100bytes:
              call_ta(TA_SS_TEST_50_half_100bytes);
              break;
            default:
              break;
          }
        }

        end = clock();
        time_taken = ((double)end - start) / CLOCKS_PER_SEC *
                     1000.0;  // Convert to milliseconds
        printf("Time taken for cmd_id %d(1): %f ms\n", cmd_ids[i], time_taken);
      }
      for (int i = 0; i < sizeof(cmd_ids) / sizeof(cmd_ids[0]); i++) {
        start = clock();
        double time_taken;
        printf("Start for cmd_id %d\n", cmd_ids[i]);
        for (int j = 0; j < 10; j++) {
          switch (cmd_ids[i]) {
            case TA_SS_TEST_5_100bytes:
              call_ta(TA_SS_TEST_5_100bytes);
              break;
            case TA_SS_TEST_10_100bytes:
              call_ta(TA_SS_TEST_10_100bytes);
              break;
            case TA_SS_TEST_20_100bytes:
              call_ta(TA_SS_TEST_20_100bytes);
              break;
            case TA_SS_TEST_30_100bytes:
              call_ta(TA_SS_TEST_30_100bytes);
              break;
            case TA_SS_TEST_40_100bytes:
              call_ta(TA_SS_TEST_40_100bytes);
              break;
            case TA_SS_TEST_50_100bytes:
              call_ta(TA_SS_TEST_50_100bytes);
              break;
            case TA_SS_TEST_5_half_100bytes:
              call_ta(TA_SS_TEST_5_half_100bytes);
              break;
            case TA_SS_TEST_10_half_100bytes:
              call_ta(TA_SS_TEST_10_half_100bytes);
              break;
            case TA_SS_TEST_20_half_100bytes:
              call_ta(TA_SS_TEST_20_half_100bytes);
              break;
            case TA_SS_TEST_30_half_100bytes:
              call_ta(TA_SS_TEST_30_half_100bytes);
              break;
            case TA_SS_TEST_40_half_100bytes:
              call_ta(TA_SS_TEST_40_half_100bytes);
              break;
            case TA_SS_TEST_50_half_100bytes:
              call_ta(TA_SS_TEST_50_half_100bytes);
              break;
            default:
              break;
          }
        }

        end = clock();
        time_taken = ((double)end - start) / CLOCKS_PER_SEC *
                     1000.0;  // Convert to milliseconds
        printf("Time taken for cmd_id %d(10): %f ms\n", cmd_ids[i], time_taken);
      }
      break;
    case 1000:
      for (int i = 0; i < sizeof(cmd_ids) / sizeof(cmd_ids[0]); i++) {
        start = clock();
        double time_taken;
        printf("Start for cmd_id %d\n", cmd_ids[i]);
        for (int j = 0; j < 1; j++) {
          switch (cmd_ids[i]) {
            case TA_SS_TEST_5_1000bytes:
              call_ta(TA_SS_TEST_5_1000bytes);
              break;
            case TA_SS_TEST_10_1000bytes:
              call_ta(TA_SS_TEST_10_1000bytes);
              break;
            case TA_SS_TEST_20_1000bytes:
              call_ta(TA_SS_TEST_20_1000bytes);
              break;
            case TA_SS_TEST_30_1000bytes:
              call_ta(TA_SS_TEST_30_1000bytes);
              break;
            case TA_SS_TEST_40_1000bytes:
              call_ta(TA_SS_TEST_40_1000bytes);
              break;
            case TA_SS_TEST_50_1000bytes:
              call_ta(TA_SS_TEST_50_1000bytes);
              break;
            case TA_SS_TEST_5_half_1000bytes:
              call_ta(TA_SS_TEST_5_half_1000bytes);
              break;
            case TA_SS_TEST_10_half_1000bytes:
              call_ta(TA_SS_TEST_10_half_1000bytes);
              break;
            case TA_SS_TEST_20_half_1000bytes:
              call_ta(TA_SS_TEST_20_half_1000bytes);
              break;
            case TA_SS_TEST_30_half_1000bytes:
              call_ta(TA_SS_TEST_30_half_1000bytes);
              break;
            case TA_SS_TEST_40_half_1000bytes:
              call_ta(TA_SS_TEST_40_half_1000bytes);
              break;
            case TA_SS_TEST_50_half_1000bytes:
              call_ta(TA_SS_TEST_50_half_1000bytes);
              break;
            default:
              break;
          }
        }

        end = clock();
        time_taken = ((double)end - start) / CLOCKS_PER_SEC *
                     1000.0;  // Convert to milliseconds
        printf("Time taken for cmd_id %d(1): %f ms\n", cmd_ids[i], time_taken);
      }
      // for (int i = 0; i < sizeof(cmd_ids) / sizeof(cmd_ids[0]); i++) {
      //   start = clock();
      //   double time_taken;
      //   printf("Start for cmd_id %d\n", cmd_ids[i]);
      //   for (int j = 0; j < 10; j++) {
      //     switch (cmd_ids[i]) {
      //       case TA_SS_TEST_5_1000bytes:
      //         call_ta(TA_SS_TEST_5_1000bytes);
      //         break;
      //       case TA_SS_TEST_10_1000bytes:
      //         call_ta(TA_SS_TEST_10_1000bytes);
      //         break;
      //       case TA_SS_TEST_20_1000bytes:
      //         call_ta(TA_SS_TEST_20_1000bytes);
      //         break;
      //       case TA_SS_TEST_30_1000bytes:
      //         call_ta(TA_SS_TEST_30_1000bytes);
      //         break;
      //       case TA_SS_TEST_40_1000bytes:
      //         call_ta(TA_SS_TEST_40_1000bytes);
      //         break;
      //       case TA_SS_TEST_50_1000bytes:
      //         call_ta(TA_SS_TEST_50_1000bytes);
      //         break;
      //       case TA_SS_TEST_5_half_1000bytes:
      //         call_ta(TA_SS_TEST_5_half_1000bytes);
      //         break;
      //       case TA_SS_TEST_10_half_1000bytes:
      //         call_ta(TA_SS_TEST_10_half_1000bytes);
      //         break;
      //       case TA_SS_TEST_20_half_1000bytes:
      //         call_ta(TA_SS_TEST_20_half_1000bytes);
      //         break;
      //       case TA_SS_TEST_30_half_1000bytes:
      //         call_ta(TA_SS_TEST_30_half_1000bytes);
      //         break;
      //       case TA_SS_TEST_40_half_1000bytes:
      //         call_ta(TA_SS_TEST_40_half_1000bytes);
      //         break;
      //       case TA_SS_TEST_50_half_1000bytes:
      //         call_ta(TA_SS_TEST_50_half_1000bytes);
      //         break;
      //       default:
      //         break;
      //     }
      //   }

      //   end = clock();
      //   time_taken = ((double)end - start) / CLOCKS_PER_SEC *
      //                1000.0;  // Convert to milliseconds
      //   printf("Time taken for cmd_id %d(10): %f ms\n", cmd_ids[i], time_taken);
      // }
      break;
    case 10000:
      for (int i = 0; i < sizeof(cmd_ids) / sizeof(cmd_ids[0]); i++) {
        start = clock();
        double time_taken;
        printf("Start for cmd_id %d\n", cmd_ids[i]);
        for (int j = 0; j < 1; j++) {
          switch (cmd_ids[i]) {
            case TA_SS_TEST_5_10000bytes:
              call_ta(TA_SS_TEST_5_10000bytes);
              break;
            case TA_SS_TEST_10_10000bytes:
              call_ta(TA_SS_TEST_10_10000bytes);
              break;
            case TA_SS_TEST_20_10000bytes:
              call_ta(TA_SS_TEST_20_10000bytes);
              break;
            case TA_SS_TEST_30_10000bytes:
              call_ta(TA_SS_TEST_30_10000bytes);
              break;
            case TA_SS_TEST_40_10000bytes:
              call_ta(TA_SS_TEST_40_10000bytes);
              break;
            case TA_SS_TEST_50_10000bytes:
              call_ta(TA_SS_TEST_50_10000bytes);
              break;
            case TA_SS_TEST_5_half_10000bytes:
              call_ta(TA_SS_TEST_5_half_10000bytes);
              break;
            case TA_SS_TEST_10_half_10000bytes:
              call_ta(TA_SS_TEST_10_half_10000bytes);
              break;
            case TA_SS_TEST_20_half_10000bytes:
              call_ta(TA_SS_TEST_20_half_10000bytes);
              break;
            case TA_SS_TEST_30_half_10000bytes:
              call_ta(TA_SS_TEST_30_half_10000bytes);
              break;
            case TA_SS_TEST_40_half_10000bytes:
              call_ta(TA_SS_TEST_40_half_10000bytes);
              break;
            case TA_SS_TEST_50_half_10000bytes:
              call_ta(TA_SS_TEST_50_half_10000bytes);
              break;
            default:
              break;
          }
        }

        end = clock();
        time_taken = ((double)end - start) / CLOCKS_PER_SEC *
                     1000.0;  // Convert to milliseconds
        printf("Time taken for cmd_id %d(1): %f ms\n", cmd_ids[i], time_taken);
      }
      // for (int i = 0; i < sizeof(cmd_ids) / sizeof(cmd_ids[0]); i++) {
      //   start = clock();
      //   double time_taken;
      //   printf("Start for cmd_id %d\n", cmd_ids[i]);
      //   for (int j = 0; j < 10; j++) {
      //     switch (cmd_ids[i]) {
      //       case TA_SS_TEST_5_10000bytes:
      //         call_ta(TA_SS_TEST_5_10000bytes);
      //         break;
      //       case TA_SS_TEST_10_10000bytes:
      //         call_ta(TA_SS_TEST_10_10000bytes);
      //         break;
      //       case TA_SS_TEST_20_10000bytes:
      //         call_ta(TA_SS_TEST_20_10000bytes);
      //         break;
      //       case TA_SS_TEST_30_10000bytes:
      //         call_ta(TA_SS_TEST_30_10000bytes);
      //         break;
      //       case TA_SS_TEST_40_10000bytes:
      //         call_ta(TA_SS_TEST_40_10000bytes);
      //         break;
      //       case TA_SS_TEST_50_10000bytes:
      //         call_ta(TA_SS_TEST_50_10000bytes);
      //         break;
      //       case TA_SS_TEST_5_half_10000bytes:
      //         call_ta(TA_SS_TEST_5_half_10000bytes);
      //         break;
      //       case TA_SS_TEST_10_half_10000bytes:
      //         call_ta(TA_SS_TEST_10_half_10000bytes);
      //         break;
      //       case TA_SS_TEST_20_half_10000bytes:
      //         call_ta(TA_SS_TEST_20_half_10000bytes);
      //         break;
      //       case TA_SS_TEST_30_half_10000bytes:
      //         call_ta(TA_SS_TEST_30_half_10000bytes);
      //         break;
      //       case TA_SS_TEST_40_half_10000bytes:
      //         call_ta(TA_SS_TEST_40_half_10000bytes);
      //         break;
      //       case TA_SS_TEST_50_half_10000bytes:
      //         call_ta(TA_SS_TEST_50_half_10000bytes);
      //         break;
      //       default:
      //         break;
      //     }
      //   }

      //   end = clock();
      //   time_taken = ((double)end - start) / CLOCKS_PER_SEC *
      //                1000.0;  // Convert to milliseconds
      //   printf("Time taken for cmd_id %d(10): %f ms\n", cmd_ids[i], time_taken);
      // }
      break;
    default:
      break;
  }

  return 0;
}
// if (argc > 1) {
//   int number = atoi(argv[1]);
//   // printf("Converted number: %d\n", number);

//   switch (number) {
//     case 5:
//       call_ta(TA_SS_TEST_5);
//       break;
//     case 10:
//       call_ta(TA_SS_TEST_10);
//       break;
//     case 20:
//       call_ta(TA_SS_TEST_20);
//       break;
//     case 30:
//       call_ta(TA_SS_TEST_30);
//       break;
//     case 40:
//       call_ta(TA_SS_TEST_40);
//       break;
//     case 50:
//       call_ta(TA_SS_TEST_50);
//       break;

//     default:
//       break;
//   }
// } else {
//   printf("Please provide a number as an argument.\n");
// }
// return 0;
// }