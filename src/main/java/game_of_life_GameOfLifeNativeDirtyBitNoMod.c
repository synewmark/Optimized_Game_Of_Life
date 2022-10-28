#include <stdlib.h>
#include <string.h>
#include "game_of_life_GameOfLifeNativeDirtyNoMod.h"
#define is_alive(char, pos) ((1 << (pos)) & (char))
#define set_alive(char, pos) (char |= (1 << (pos)))
#define mod(n, m) ((n)%(m))
#define safe_mod(n, m) mod((n+m), m)


struct packed_short{
    unsigned char pos0 : 4;
    unsigned char pos1 : 4;
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
      // printf("board[%d] = %x\n", i, board[i]);
      for (int j = 0; j < ylenpacked; j++) {
        signed char c = 0;
        for (int k = 7; k >= 0; k--) {
          c<<=1;
          c+=(boolElementsi[j*8+k]);
        }
        board[i][j] = c;
      }
    (*env)->ReleasePrimitiveArrayCritical(env, boolArrayi, boolElementsi, 0);
    //exiting critical
  }
  return board;
}

void unpack_8(JNIEnv* env, jobjectArray array, signed char** board, int xlen, int ylenpacked){
  for (int i = 0; i < xlen; i++) {
      jbooleanArray boolArrayi = (*env)->GetObjectArrayElement(env, array, i);
      jboolean* boolElementsi = (*env)->GetPrimitiveArrayCritical(env, boolArrayi, 0);
      for (int j = 0; j < ylenpacked; j++) {
        if (board[i][j]) {
          // printf("not blank!");
        }
        for (int k = 0; k < 8; k++) {
            boolElementsi[j*8+k] = is_alive(board[i][j], k) ? JNI_TRUE : JNI_FALSE;
        }
      }
      // printf("board[%d] = %x\n", i, board[i]);
      free(board[i]);
      (*env)->ReleasePrimitiveArrayCritical(env, boolArrayi, boolElementsi, 0);
    }
}



JNIEXPORT void JNICALL Java_game_1of_1life_GameOfLifeNativeDirtyNoMod_getNGenerationNative
  (JNIEnv * env, jobject object, jint runlength, jobjectArray array) {
    unsigned long long skip_count = 0;
    unsigned long long non_skip_count = 0;
    int num = 0;
    int xlen = -1;
    int ylen = -1;
    signed char** board = pack_8(env, array, &xlen, &ylen);
    if (!board) {
      if (ylen % 8) {
        printf("Array must have a y size divisable by 8\n");
        return;
      } else {
        printf("Indeterminate error, terminating\n");
        return;
      }
    }
    int ylenpacked = ylen/8;
    int ylenpackedhalf = ylen/2;
    int ylenpackeddouble = ylenpacked/8 + (ylenpacked%8 != 0);
    struct packed_short** buffer = malloc(sizeof(struct packed_short*)*(xlen+2));
    struct packed_short** buffer2 = malloc(sizeof(struct packed_short*)*(xlen+2));
    buffer = buffer+1;
    buffer2 = buffer2+1;
    signed char** dirty_bit1 = malloc(sizeof(char*)*(xlen+2));
    signed char** dirty_bit2 = malloc(sizeof(char*)*(xlen+2));
    dirty_bit1 = dirty_bit1+1;
    dirty_bit2 = dirty_bit2+1;
    for (int i = 0; i < xlen; i++) {
      buffer[i] = calloc(ylenpackedhalf+2, sizeof(struct packed_short));
      if (!buffer[i]) {
        printf("Errno: %d\n", errno);
      }
      buffer[i] = buffer[i]+1;
      buffer2[i] = calloc(ylenpackedhalf+2, sizeof(struct packed_short));
      if (!buffer2[i]) {
        printf("Errno: %d\n", errno);
      }  
      buffer2[i] = buffer2[i]+1;
      dirty_bit2[i] = calloc(ylenpackeddouble+2, sizeof(char));
      if (!dirty_bit2[i]) {
        printf("Errno: %d\n", errno);
      }  
      dirty_bit2[i] = dirty_bit2[i]+1;
      dirty_bit1[i] = malloc(sizeof(char)*(ylenpackeddouble+2));
      if (!dirty_bit1[i]) {
        printf("Errno: %d\n", errno);
      }
      dirty_bit1[i] = dirty_bit1[i]+1;
      memset(dirty_bit1[i], 0xFF, sizeof(char)*ylenpackeddouble);
      // printf("%d\n", i);
    }
    buffer[-1] = buffer[xlen-1];
    buffer2[-1] = buffer2[xlen-1];
    dirty_bit1[-1] = dirty_bit1[xlen-1];
    dirty_bit2[-1] = dirty_bit2[xlen-1];

    buffer[xlen] = buffer[0];
    buffer2[xlen] = buffer2[0];
    dirty_bit1[xlen] = dirty_bit1[0];
    dirty_bit2[xlen] = dirty_bit2[0];
    // printf("%d, %d\n", ylenpackedhalf, ylenpackeddouble);
    // printf("packed: %d packed half: %d packed double: %d\n", ylenpacked, ylenpackedhalf, ylenpackeddouble);
    for (int j = 0; j < xlen; j++) {
      for (int k = 0; k < ylenpacked; k++) {
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
            // }
          }
      }
    }
    for (int j = 0; j < xlen; j++) {
        memcpy(buffer2[j], buffer[j], ylenpackedhalf*sizeof(struct packed_short));
    }
    for (int i = 0; i < runlength; i++) {
      for (int j = 0; j < xlen; j++) {
        for (int k = 0; k < ylenpacked; k++) {
          // printf("\nx: %d, y: %d, count: %d, alive: %d\n", j, k*8, buffer[j][k*4].pos0, is_alive(board[j][k], 0));
          // printf("x: %d, y: %d, count: %d, alive: %d\n", j, k*8+1, buffer[j][k*4].pos1, is_alive(board[j][k], 1));
          // printf("x: %d, y: %d, count: %d, alive: %d\n", j, k*8+2, buffer[j][k*4+1].pos0, is_alive(board[j][k], 2));
          // printf("x: %d, y: %d, count: %d, alive: %d\n", j, k*8+3, buffer[j][k*4+1].pos1, is_alive(board[j][k], 3));
          // printf("x: %d, y: %d, count: %d, alive: %d\n", j, k*8+4, buffer[j][k*4+2].pos0, is_alive(board[j][k], 4));
          // printf("x: %d, y: %d, count: %d, alive: %d\n", j, k*8+5, buffer[j][k*4+2].pos1, is_alive(board[j][k], 5));
          // printf("x: %d, y: %d, count: %d, alive: %d\n", j, k*8+6, buffer[j][k*4+3].pos0, is_alive(board[j][k], 6));
          // printf("x: %d, y: %d, count: %d, alive: %d\n", j, k*8+7, buffer[j][k*4+3].pos1, is_alive(board[j][k], 7));
          if(!is_alive(dirty_bit1[j][k/8], k%8)) {
            skip_count++;
            continue;
          }
          non_skip_count++;
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
          
          signed char xc = c^board[j][k];

          if (xc) {
            set_alive(dirty_bit2[j+1][k/8], k%8);
            set_alive(dirty_bit2[j][k/8], k%8);
            set_alive(dirty_bit2[j-1][k/8], k%8);

            if (is_alive(xc, 0)) {
              set_alive(dirty_bit2[j+1][k/8-(k%8==0)], safe_mod(k-1, 8));
              set_alive(dirty_bit2[j][k/8-(k%8==0)], safe_mod(k-1, 8));
              set_alive(dirty_bit2[j-1][k/8-(k%8==0)], safe_mod(k-1, 8));
            }

            if (is_alive(xc, 7)) {
              set_alive(dirty_bit2[j+1][k/8+(k%8==7)], mod(k+1, 8));
              set_alive(dirty_bit2[j][k/8+(k%8==7)], mod(k+1, 8));
              set_alive(dirty_bit2[j-1][k/8+(k%8==7)], mod(k+1, 8));
            }

            if(is_alive(xc, 0)) {
              signed char delta = is_alive(c, 0) ? 1 : -1;
              buffer2[j-1][(k*4)-1].pos1+=delta;
              buffer2[j-1][(k*4)].pos0+=delta;
              buffer2[j-1][(k*4)].pos1+=delta;

              buffer2[j][(k*4)-1].pos1+=delta;
              buffer2[j][(k*4)].pos1+=delta;

              buffer2[j+1][(k*4)-1].pos1+=delta;
              buffer2[j+1][(k*4)].pos0+=delta;
              buffer2[j+1][(k*4)].pos1+=delta;
            }
            if(is_alive(xc, 1)) {
              signed char delta = is_alive(c, 1) ? 1 : -1;
              buffer2[j-1][(k*4)].pos0+=delta;
              buffer2[j-1][(k*4)].pos1+=delta;
              buffer2[j-1][(k*4)+1].pos0+=delta;

              buffer2[j][(k*4)].pos0+=delta;
              buffer2[j][(k*4)+1].pos0+=delta;

              buffer2[j+1][(k*4)].pos0+=delta;
              buffer2[j+1][(k*4)].pos1+=delta;
              buffer2[j+1][(k*4)+1].pos0+=delta;
            }
            if(is_alive(xc, 2)) {
              signed char delta = is_alive(c, 2) ? 1 : -1;
              buffer2[j-1][(k*4)+1-1].pos1+=delta;
              buffer2[j-1][(k*4)+1].pos0+=delta;
              buffer2[j-1][(k*4)+1].pos1+=delta;

              buffer2[j][(k*4)+1-1].pos1+=delta;
              buffer2[j][(k*4)+1].pos1+=delta;

              buffer2[j+1][(k*4)+1-1].pos1+=delta;
              buffer2[j+1][(k*4)+1].pos0+=delta;
              buffer2[j+1][(k*4)+1].pos1+=delta;
            }
            if(is_alive(xc, 3)) {
              signed char delta = is_alive(c, 3) ? 1 : -1;
              buffer2[j-1][(k*4)+1].pos0+=delta;
              buffer2[j-1][(k*4)+1].pos1+=delta;
              buffer2[j-1][(k*4)+1+1].pos0+=delta;

              buffer2[j][(k*4)+1].pos0+=delta;
              buffer2[j][(k*4)+1+1].pos0+=delta;

              buffer2[j+1][(k*4)+1].pos0+=delta;
              buffer2[j+1][(k*4)+1].pos1+=delta;
              buffer2[j+1][(k*4)+1+1].pos0+=delta;
            }
            if(is_alive(xc, 4)) {
              signed char delta = is_alive(c, 4) ? 1 : -1;
              buffer2[j-1][(k*4)+2-1].pos1+=delta;
              buffer2[j-1][(k*4)+2].pos0+=delta;
              buffer2[j-1][(k*4)+2].pos1+=delta;

              buffer2[j][(k*4)+2-1].pos1+=delta;
              buffer2[j][(k*4)+2].pos1+=delta;

              buffer2[j+1][(k*4)+2-1].pos1+=delta;
              buffer2[j+1][(k*4)+2].pos0+=delta;
              buffer2[j+1][(k*4)+2].pos1+=delta;
            }
            if(is_alive(xc, 5)) {
              signed char delta = is_alive(c, 5) ? 1 : -1;
              buffer2[j-1][(k*4)+2].pos0+=delta;
              buffer2[j-1][(k*4)+2].pos1+=delta;
              buffer2[j-1][(k*4)+2+1].pos0+=delta;

              buffer2[j][(k*4)+2].pos0+=delta;
              buffer2[j][(k*4)+2+1].pos0+=delta;

              buffer2[j+1][(k*4)+2].pos0+=delta;
              buffer2[j+1][(k*4)+2].pos1+=delta;
              buffer2[j+1][(k*4)+2+1].pos0+=delta;
            }
            if(is_alive(xc, 6)) {
              signed char delta = is_alive(c, 6) ? 1 : -1;
              buffer2[j-1][(k*4)+3-1].pos1+=delta;
              buffer2[j-1][(k*4)+3].pos0+=delta;
              buffer2[j-1][(k*4)+3].pos1+=delta;

              buffer2[j][(k*4)+3-1].pos1+=delta;
              buffer2[j][(k*4)+3].pos1+=delta;

              buffer2[j+1][(k*4)+3-1].pos1+=delta;
              buffer2[j+1][(k*4)+3].pos0+=delta;
              buffer2[j+1][(k*4)+3].pos1+=delta;
            }
            if(is_alive(xc, 7)) {
              signed char delta = is_alive(c, 7) ? 1 : -1;
              buffer2[j-1][(k*4)+3].pos0+=delta;
              buffer2[j-1][(k*4)+3].pos1+=delta;
              buffer2[j-1][(k*4)+3+1].pos0+=delta;

              buffer2[j][(k*4)+3].pos0+=delta;
              buffer2[j][(k*4)+3+1].pos0+=delta;

              buffer2[j+1][(k*4)+3].pos0+=delta;
              buffer2[j+1][(k*4)+3].pos1+=delta;
              buffer2[j+1][(k*4)+3+1].pos0+=delta;
            }
            board[j][k] = c;
          }
        }
      }
      signed char** temp = buffer2;
      buffer2 = buffer;
      buffer = temp;
      temp = dirty_bit1;
      dirty_bit1 = dirty_bit2;
      dirty_bit2 = temp;
      for (int j = 0; j < xlen; j++) {
          dirty_bit1[j][0] |= 0x1;
          set_alive(dirty_bit1[j][ylenpackeddouble-1], ylenpacked%8-1);

          buffer[j][0].pos0 += buffer[j][ylenpackedhalf].pos0;
          
          buffer[j][ylenpackedhalf-1].pos1 += buffer[j][-1].pos1;
          buffer[j][ylenpackedhalf].pos0 = 0;
          buffer[j][-1].pos1 = 0;
          memcpy(buffer2[j], buffer[j], ylenpackedhalf*sizeof(struct packed_short));
          memset(dirty_bit2[j], 0, sizeof(char)*ylenpackeddouble);
      }      
    }
    unpack_8(env, array, board, xlen, ylenpacked);
    for (int i = 0; i < xlen; i++) {
      free(buffer[i]-1);
    }
    free(board);
    free(buffer-1);
    // printf("Skipped: %lu, didn't skip %lu\n", skip_count, non_skip_count);
    }