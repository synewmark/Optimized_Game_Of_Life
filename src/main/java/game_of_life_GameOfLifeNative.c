#include <stdlib.h>
#include <string.h>
#include "game_of_life_GameOfLifeNative.h"

JNIEXPORT void JNICALL Java_game_1of_1life_GameOfLifeNative_getNGenerationNative (JNIEnv * env, jobject obj, jint runlength, jobjectArray array) {

      jsize xlen = (*env)->GetArrayLength(env, array);
      jobjectArray dim1 = (*env)->GetObjectArrayElement(env, array, 0); 
      jsize ylen = (*env)->GetArrayLength(env, dim1);
      jboolean **native_array = malloc(sizeof(jboolean*)*xlen);
      char** buffer = malloc(sizeof(char*)*xlen);
    printf("%d\n", sizeof(jboolean));
      for (int i = 0; i < xlen; i++) {
          jbooleanArray boolArrayi = (*env)->GetObjectArrayElement(env, array, i);
          jboolean isCopy = JNI_FALSE;
          jboolean* boolElementsi = (*env)->GetPrimitiveArrayCritical(env, boolArrayi, &isCopy);
          if (isCopy) {
              //bad!!
          }
          native_array[i] = boolElementsi;
        //   for (int j = 0; j < ylen; j++) {
        //         printf("%d", native_array[i][j]);
        //   }
        buffer[i] = calloc(ylen, sizeof(char));
      }
      for (int i = 0; i < runlength; i++) {
          for (int j = 0; j < xlen; j++) {
              for (int k = 0; k < ylen; k++) {
                  if (native_array[j][k]) {
                    buffer[(j + 1) % xlen][k]++;
                    buffer[j][(k + 1) % ylen]++;
                    buffer[(j + 1) % xlen][(k + 1) % ylen]++;

                    buffer[(j - 1 + xlen) % xlen][k]++;
                    buffer[j][(k - 1 + ylen) % ylen]++;
                    buffer[(j - 1 + xlen) % xlen][(k - 1 + ylen) % ylen]++;

                    buffer[(j - 1 + xlen) % xlen][(k + 1) % ylen]++;

                    buffer[(j + 1) % xlen][(k - 1 + ylen) % ylen]++;
                  }
              }
          }
          for (int j = 0; j < xlen; j++) {
				for (int k = 0; k < ylen; k++) {
					native_array[j][k] = (buffer[j][k] == 3 || (buffer[j][k] == 2 && native_array[j][k]));
				}
				memset(buffer[j], 0, sizeof(char)*ylen);
		    }
      }
  }

