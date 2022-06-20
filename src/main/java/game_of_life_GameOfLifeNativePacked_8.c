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

signed char** pack_8(JNIEnv * env, jobjectArray array, int* xlen_store, int* ylen_store) {
  const jsize xlen = (*env)->GetArrayLength(env, array);
  jobjectArray dim1 = (*env)->GetObjectArrayElement(env, array, 0); 
  const jsize ylen = (*env)->GetArrayLength(env, dim1);
  const jsize ylenpacked = ylen/8;
  *xlen_store = xlen;
  *ylen_store = ylen;
  if (ylen % 8) {
    return NULL;
  }
  signed char** board = malloc(sizeof(char*)*xlen);
  for (int i = 0; i < xlen; i++) {
      jbooleanArray boolArrayi = (*env)->GetObjectArrayElement(env, array, i);
      jboolean isCopy = JNI_FALSE;
      //entering critical
      jboolean* boolElementsi = (*env)->GetPrimitiveArrayCritical(env, boolArrayi, &isCopy);
      if (isCopy) {
          return NULL;
      }
      //pack
      board[i] = calloc(sizeof(char), ylenpacked);
      for (int j = 0; j < ylenpacked; j++) {
        signed char c = 0;
          if (boolElementsi[j*8+7]) {
            c++;
          }
          c<<=1;
          if (boolElementsi[j*8+6]) {
            c++;
          }
          c<<=1;
          if (boolElementsi[j*8+5]) {
            c++;
          }
          c<<=1;
          if (boolElementsi[j*8+4]) {
            c++;
          }
          c<<=1;
          if (boolElementsi[j*8+3]) {
            c++;
          }
          c<<=1;
          if (boolElementsi[j*8+2]) {
            c++;
          }
          c<<=1;
          if (boolElementsi[j*8+1]) {
            c++;
          }
          c<<=1;
          if (boolElementsi[j*8]) {
            c++;
          }
          board[i][j] = c;
      }
    (*env)->ReleasePrimitiveArrayCritical(env, boolArrayi, boolElementsi, 0);
  }
  return board;
}

JNIEXPORT void JNICALL Java_game_1of_1life_GameOfLifeNativePacked_getNGenerationNative
  (JNIEnv * env, jobject object, jint runlength, jobjectArray array) {
    int xlen = -1;
    int ylen = -1;
    signed char** board = pack_8(env, array, &xlen, &ylen);
    if (!board) {
      if (ylen % 8) {
        printf("Array must have a y size divisable by 8\n");
        return;
      } else {
        printf("Indeterminate error, terminating\n");
      }
    }
    int ylenpacked = ylen/8;
    int ylenpackedhalf = ylen/2;
    struct packed_short** buffer = malloc(sizeof(struct packed_short*)*xlen);
    for (int i = 0; i < xlen; i++) {
      buffer[i] = calloc(ylenpackedhalf, sizeof(struct packed_short));
    }
    // printf("Here2\n");
    printf("packed: %d packed half: %d\n", ylenpacked, ylenpackedhalf);
    for (int i = 0; i < runlength; i++) {
        for (int j = 0; j < xlen; j++) {
            for (int k = 0; k < ylenpacked; k++) {
                // for (int l = 0; l < 8; l++) {
                  // printf("%d\n",(1 << l) & board[j][k]);
                    if(is_alive(board[j][k], 0)) {
                      buffer[safe_mod(j-1, xlen)][safe_mod((k*4)-1, ylenpackedhalf)].pos1++;
                      buffer[safe_mod(j-1, xlen)][(k*4)].pos0++;
                      buffer[safe_mod(j-1, xlen)][(k*4)].pos1++;

                      buffer[j][safe_mod((k*4)-1, ylenpackedhalf)].pos1++;
                      buffer[j][(k*4)].pos1++;

                      buffer[mod(j+1, xlen)][safe_mod((k*4)-1, ylenpackedhalf)].pos1++;
                      buffer[mod(j+1, xlen)][(k*4)].pos0++;
                      buffer[mod(j+1, xlen)][(k*4)].pos1++;
                    }
                    if(is_alive(board[j][k], 1)) {
                      buffer[safe_mod(j-1, xlen)][(k*4)].pos0++;
                      buffer[safe_mod(j-1, xlen)][(k*4)].pos1++;
                      // printf("!y is: %d y-incr: %d\n", k*4+l, mod((k+l)/2+1, ylenpackedhalf));
                      buffer[safe_mod(j-1, xlen)][mod((k*4)+1, ylenpackedhalf)].pos0++;

                      buffer[j][(k*4)].pos0++;
                      buffer[j][mod((k*4)+1, ylenpackedhalf)].pos0++;

                      buffer[mod(j+1, xlen)][(k*4)].pos0++;
                      buffer[mod(j+1, xlen)][(k*4)].pos1++;
                      buffer[mod(j+1, xlen)][mod((k*4)+1,ylenpackedhalf)].pos0++;
                    }
                    if(is_alive(board[j][k], 2)) {
                      buffer[safe_mod(j-1, xlen)][safe_mod((k*4)+1-1, ylenpackedhalf)].pos1++;
                      buffer[safe_mod(j-1, xlen)][(k*4)+1].pos0++;
                      buffer[safe_mod(j-1, xlen)][(k*4)+1].pos1++;

                      buffer[j][safe_mod((k*4)+1-1, ylenpackedhalf)].pos1++;
                      buffer[j][(k*4)+1].pos1++;

                      buffer[mod(j+1, xlen)][safe_mod((k*4)+1-1, ylenpackedhalf)].pos1++;
                      buffer[mod(j+1, xlen)][(k*4)+1].pos0++;
                      buffer[mod(j+1, xlen)][(k*4)+1].pos1++;
                    }
                    if(is_alive(board[j][k], 3)) {
                      buffer[safe_mod(j-1, xlen)][(k*4)+1].pos0++;
                      buffer[safe_mod(j-1, xlen)][(k*4)+1].pos1++;
                      // printf("!y is: %d y-incr: %d\n", k*4+l, mod((k+l)/2+1, ylenpackedhalf));
                      buffer[safe_mod(j-1, xlen)][mod((k*4)+1+1, ylenpackedhalf)].pos0++;

                      buffer[j][(k*4)+1].pos0++;
                      buffer[j][mod((k*4)+1+1, ylenpackedhalf)].pos0++;

                      buffer[mod(j+1, xlen)][(k*4)+1].pos0++;
                      buffer[mod(j+1, xlen)][(k*4)+1].pos1++;
                      buffer[mod(j+1, xlen)][mod((k*4)+1+1,ylenpackedhalf)].pos0++;
                    }
                     if(is_alive(board[j][k], 4)) {
                      buffer[safe_mod(j-1, xlen)][safe_mod((k*4)+2-1, ylenpackedhalf)].pos1++;
                      buffer[safe_mod(j-1, xlen)][(k*4)+2].pos0++;
                      buffer[safe_mod(j-1, xlen)][(k*4)+2].pos1++;

                      buffer[j][safe_mod((k*4)+2-1, ylenpackedhalf)].pos1++;
                      buffer[j][(k*4)+2].pos1++;

                      buffer[mod(j+1, xlen)][safe_mod((k*4)+2-1, ylenpackedhalf)].pos1++;
                      buffer[mod(j+1, xlen)][(k*4)+2].pos0++;
                      buffer[mod(j+1, xlen)][(k*4)+2].pos1++;
                    }
                    if(is_alive(board[j][k], 5)) {
                      buffer[safe_mod(j-1, xlen)][(k*4)+2].pos0++;
                      buffer[safe_mod(j-1, xlen)][(k*4)+2].pos1++;
                      // printf("!y is: %d y-incr: %d\n", k*4+l, mod((k+l)/2+1, ylenpackedhalf));
                      buffer[safe_mod(j-1, xlen)][mod((k*4)+2+1, ylenpackedhalf)].pos0++;

                      buffer[j][(k*4)+2].pos0++;
                      buffer[j][mod((k*4)+2+1, ylenpackedhalf)].pos0++;

                      buffer[mod(j+1, xlen)][(k*4)+2].pos0++;
                      buffer[mod(j+1, xlen)][(k*4)+2].pos1++;
                      buffer[mod(j+1, xlen)][mod((k*4)+2+1,ylenpackedhalf)].pos0++;
                    }
                     if(is_alive(board[j][k], 6)) {
                      buffer[safe_mod(j-1, xlen)][safe_mod((k*4)+3-1, ylenpackedhalf)].pos1++;
                      buffer[safe_mod(j-1, xlen)][(k*4)+3].pos0++;
                      buffer[safe_mod(j-1, xlen)][(k*4)+3].pos1++;

                      buffer[j][safe_mod((k*4)+3-1, ylenpackedhalf)].pos1++;
                      buffer[j][(k*4)+3].pos1++;

                      buffer[mod(j+1, xlen)][safe_mod((k*4)+3-1, ylenpackedhalf)].pos1++;
                      buffer[mod(j+1, xlen)][(k*4)+3].pos0++;
                      buffer[mod(j+1, xlen)][(k*4)+3].pos1++;
                    }
                    if(is_alive(board[j][k], 7)) {
                      buffer[safe_mod(j-1, xlen)][(k*4)+3].pos0++;
                      buffer[safe_mod(j-1, xlen)][(k*4)+3].pos1++;
                      // printf("!y is: %d y-incr: %d\n", k*4+l, mod((k+l)/2+1, ylenpackedhalf));
                      buffer[safe_mod(j-1, xlen)][mod((k*4)+3+1, ylenpackedhalf)].pos0++;

                      buffer[j][(k*4)+3].pos0++;
                      buffer[j][mod((k*4)+3+1, ylenpackedhalf)].pos0++;

                      buffer[mod(j+1, xlen)][(k*4)+3].pos0++;
                      buffer[mod(j+1, xlen)][(k*4)+3].pos1++;
                      buffer[mod(j+1, xlen)][mod((k*4)+3+1,ylenpackedhalf)].pos0++;
                    }
                    // printf("Here3\n");
                // }
                // printf("Here4\n");
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
                  c+=(buffer[j][k*4+3].pos1 == 3 || (buffer[j][k*4+3].pos1 == 2 && is_alive(board[j][k], 7)));
                  c <<= 1;
                  c+=(buffer[j][k*4+3].pos0 == 3 || (buffer[j][k*4+3].pos0 == 2 && is_alive(board[j][k], 6)));
                  c <<= 1;
                  c+=(buffer[j][k*4+2].pos1 == 3 || (buffer[j][k*4+2].pos1 == 2 && is_alive(board[j][k], 5)));
                  c <<= 1;
                  c+=(buffer[j][k*4+2].pos0 == 3 || (buffer[j][k*4+2].pos0 == 2 && is_alive(board[j][k], 4)));
                  c <<= 1;
                  c+=(buffer[j][k*4+1].pos1 == 3 || (buffer[j][k*4+1].pos1 == 2 && is_alive(board[j][k], 3)));
                  c <<= 1;
                  c+=(buffer[j][k*4+1].pos0 == 3 || (buffer[j][k*4+1].pos0 == 2 && is_alive(board[j][k], 2)));
                  c <<= 1;
                  c+=(buffer[j][k*4].pos1 == 3 || (buffer[j][k*4].pos1 == 2 && is_alive(board[j][k], 1)));
                  c <<= 1;
                  c+=(buffer[j][k*4].pos0 == 3 || (buffer[j][k*4].pos0 == 2 && is_alive(board[j][k], 0)));
                  
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
        for (int k = 0; k < 8; k++) {
            boolElementsi[j*8+k] = is_alive(board[i][j], k) ? JNI_TRUE : JNI_FALSE;
        }
      }
      (*env)->ReleasePrimitiveArrayCritical(env, boolArrayi, boolElementsi, 0);
    }

  }