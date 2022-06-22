#include <stdlib.h>
#include <string.h>
#include "game_of_life_GameOfLifeNativePacked.h"
#define is_alive(char, pos) ((1 << pos) & char)
#define mod(n, m) ((n)%(m))
#define safe_mod(n, m) mod((n+m), m)


struct packed_short{
    signed char pos0 : 4;
    signed char pos1 : 4;
};

JNIEXPORT void JNICALL Java_game_1of_1life_GameOfLifeNativePacked_getNGenerationNative
  (JNIEnv * env, jobject object, jint runlength, jobjectArray array) {
      const jsize xlen = (*env)->GetArrayLength(env, array);
      jobjectArray dim1 = (*env)->GetObjectArrayElement(env, array, 0); 
      const jsize ylen = (*env)->GetArrayLength(env, dim1);
      const jsize ylenpacked = ylen/4;
      if (xlen % 4 != 0 || ylen % 4 != 0) {
        printf("Array must be divisible by 4\n");
        return;
      }
      signed char** board = malloc(sizeof(char*)*xlen);
      struct packed_short** buffer = malloc(sizeof(struct packed_short*)*xlen);
      int ylenpackedhalf = ylenpacked*2;
      // printf("Here\n");
      for (int i = 0; i < xlen; i++) {
          jbooleanArray boolArrayi = (*env)->GetObjectArrayElement(env, array, i);
          jboolean isCopy = JNI_FALSE;
          //entering critical
          jboolean* boolElementsi = (*env)->GetPrimitiveArrayCritical(env, boolArrayi, &isCopy);
          if (isCopy) {
              //bad!!
          }
          //pack
          board[i] = calloc(sizeof(char), ylenpacked);
          for (int j = 0; j < ylenpacked; j++) {
            signed char c = 0;
              if (boolElementsi[j*4+3]) {
                c++;
              }
              c<<=1;
              if (boolElementsi[j*4+2]) {
                c++;
              }
              c<<=1;
              if (boolElementsi[j*4+1]) {
                c++;
              }
              c<<=1;
              if (boolElementsi[j*4]) {
                c++;
              }
              board[i][j] = c;
              // printf("val at %d, %x\n", i, c);
          }
        //   int rem = ylen%8;
        //   int total = ylen - rem;
        //   struct packed_boolean* pb = board[ylen/8];
        //   if (rem > 0) {
        //       pb->pos0 = boolElementsi[total*8];
        //   }
        //   if (rem > 1) {
        //       pb->pos1 = boolElementsi[total*8+1];
        //   }
        //   if (rem > 2) {
        //       pb->pos2 = boolElementsi[total*8+2];
        //   }
        //   if (rem > 3) {
        //       pb->pos3 = boolElementsi[total*8+3];
        //   }
        //   if (rem > 4) {
        //       pb->pos4 = boolElementsi[total*8+4];
        //   }
        //   if (rem > 5) {
        //       pb->pos5 = boolElementsi[total*8+5];
        //   }
        //   if (rem > 6) {
        //       pb->pos6 = boolElementsi[total*8+6];
        //   }
        buffer[i] = calloc(ylenpackedhalf, sizeof(struct packed_short));
        (*env)->ReleasePrimitiveArrayCritical(env, boolArrayi, boolElementsi, 0);
        // printf("Here1\n");
        //exited critical
      }
    // printf("Here2\n");
    printf("packed: %d packed half: %d\n", ylenpacked, ylenpackedhalf);
    for (int i = 0; i < runlength; i++) {
        for (int j = 0; j < xlen; j++) {
            for (int k = 0; k < ylenpacked; k++) {
                if(is_alive(board[j][k], 0)) {
                  buffer[safe_mod(j-1, xlen)][safe_mod((k*2)-1, ylenpackedhalf)].pos1++;
                  buffer[safe_mod(j-1, xlen)][(k*2)].pos0++;
                  buffer[safe_mod(j-1, xlen)][(k*2)].pos1++;

                  buffer[j][safe_mod((k*2)-1, ylenpackedhalf)].pos1++;
                  buffer[j][(k*2)].pos1++;

                  buffer[mod(j+1, xlen)][safe_mod((k*2)-1, ylenpackedhalf)].pos1++;
                  buffer[mod(j+1, xlen)][(k*2)].pos0++;
                  buffer[mod(j+1, xlen)][(k*2)].pos1++;
                }
                if(is_alive(board[j][k], 1)) {
                  buffer[safe_mod(j-1, xlen)][(k*2)].pos0++;
                  buffer[safe_mod(j-1, xlen)][(k*2)].pos1++;
                  // printf("!y is: %d y-incr: %d\n", k*4+l, mod((k+l)/2+1, ylenpackedhalf));
                  buffer[safe_mod(j-1, xlen)][mod((k*2)+1, ylenpackedhalf)].pos0++;

                  buffer[j][(k*2)].pos0++;
                  buffer[j][mod((k*2)+1, ylenpackedhalf)].pos0++;

                  buffer[mod(j+1, xlen)][(k*2)].pos0++;
                  buffer[mod(j+1, xlen)][(k*2)].pos1++;
                  buffer[mod(j+1, xlen)][mod((k*2)+1,ylenpackedhalf)].pos0++;
                }
                if(is_alive(board[j][k], 2)) {
                  buffer[safe_mod(j-1, xlen)][safe_mod((k*2)+1-1, ylenpackedhalf)].pos1++;
                  buffer[safe_mod(j-1, xlen)][(k*2)+1].pos0++;
                  buffer[safe_mod(j-1, xlen)][(k*2)+1].pos1++;

                  buffer[j][safe_mod((k*2)+1-1, ylenpackedhalf)].pos1++;
                  buffer[j][(k*2)+1].pos1++;

                  buffer[mod(j+1, xlen)][safe_mod((k*2)+1-1, ylenpackedhalf)].pos1++;
                  buffer[mod(j+1, xlen)][(k*2)+1].pos0++;
                  buffer[mod(j+1, xlen)][(k*2)+1].pos1++;
                }
                if(is_alive(board[j][k], 3)) {
                  buffer[safe_mod(j-1, xlen)][(k*2)+1].pos0++;
                  buffer[safe_mod(j-1, xlen)][(k*2)+1].pos1++;
                  // printf("!y is: %d y-incr: %d\n", k*4+l, mod((k+l)/2+1, ylenpackedhalf));
                  buffer[safe_mod(j-1, xlen)][mod((k*2)+1+1, ylenpackedhalf)].pos0++;

                  buffer[j][(k*2)+1].pos0++;
                  buffer[j][mod((k*2)+1+1, ylenpackedhalf)].pos0++;

                  buffer[mod(j+1, xlen)][(k*2)+1].pos0++;
                  buffer[mod(j+1, xlen)][(k*2)+1].pos1++;
                  buffer[mod(j+1, xlen)][mod((k*2)+1+1,ylenpackedhalf)].pos0++;
                }
            }
        }
        // printf("Here5\n");
          for (int j = 0; j < xlen; j++) {
            for (int k = 0; k < ylenpacked; k++) {
              // printf("Started");
                  // printf("\nx: %d, y: %d, count: %d\n", j, k*4, buffer[j][k*2].pos0);
                  // printf("x: %d, y: %d, count: %d\n", j, k*4+1, buffer[j][k*2].pos1);
                  // printf("x: %d, y: %d, count: %d\n", j, k*4+2, buffer[j][k*2+1].pos0);
                  // printf("x: %d, y: %d, count: %d\n", j, k*4+3, buffer[j][k*2+1].pos1);
                  

                  char c = 0;
                  if(buffer[j][k*2+1].pos1 == 3 || (buffer[j][k*2+1].pos1 == 2 && is_alive(board[j][k], 3))) {
                    c++;
                  }
                  c <<= 1;
                  if(buffer[j][k*2+1].pos0 == 3 || (buffer[j][k*2+1].pos0 == 2 && is_alive(board[j][k], 2))) {
                    c++;
                  }
                  c <<= 1;
                  if(buffer[j][k*2].pos1 == 3 || (buffer[j][k*2].pos1 == 2 && is_alive(board[j][k], 1))) {
                    c++;
                  }
                  c <<= 1;
                  if(buffer[j][k*2].pos0 == 3 || (buffer[j][k*2].pos0 == 2 && is_alive(board[j][k], 0))) {
                    c++;
                  }
                  
                  board[j][k] = c;
                  // printf("Finished");
            }
            memset(buffer[j], 0, sizeof(struct packed_short)*ylenpackedhalf);
        }
    }
    // printf("Got here!!");
    for (int i = 0; i < xlen; i++) {
      jbooleanArray boolArrayi = (*env)->GetObjectArrayElement(env, array, i);
      jboolean* boolElementsi = (*env)->GetPrimitiveArrayCritical(env, boolArrayi, 0);
      for (int j = 0; j < ylenpacked; j++) {
        for (int k = 0; k < 4; k++) {
            // printf("%d %d %d\n", j*4+k, i, j);
            boolElementsi[j*4+k] = is_alive(board[i][j], k) ? JNI_TRUE : JNI_FALSE;
        }
      }
      (*env)->ReleasePrimitiveArrayCritical(env, boolArrayi, boolElementsi, 0);
    }

  }