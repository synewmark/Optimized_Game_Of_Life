#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <synchapi.h>
#include <synchapi.h>
#include <crtdbg.h>

#include "game_of_life_GameOfLifeMultithread.h"

#define is_alive(char, pos) ((1 << (pos)) & (char))
#define set_alive(char, pos) (char |= (1 << (pos)))
#define mod(n, m) ((n)%(m))
#define safe_mod(n, m) mod((n+m), m)
#define get_integral_row_left(left, center) (((left & 1) << 5)) | (center >> 3)
#define get_integral_row_right(center, right) (((center & 0x1F) << 1) | (right & (1 << 7)) >> 7)

jbyte* lookuptable;
unsigned char** board1;
unsigned char** board2;
unsigned char** dirty_bit1;
unsigned char** dirty_bit2;
int xlen;
int ylenpacked;

unsigned long long skip_count = 0;
unsigned long long non_skip_count = 0;

SYNCHRONIZATION_BARRIER* barrier;

struct packed_short{
    unsigned char pos0 : 4;
    unsigned char pos1 : 4;
};

struct thread_work{
    int genlength;
    int start;
    int end;
};

unsigned int get_integral_val_left(unsigned int nw, unsigned int n, unsigned int w, unsigned int c, unsigned int sw, unsigned int s) {
    unsigned int top = get_integral_row_left(nw, n);
    unsigned int mid = get_integral_row_left(w, c);
    unsigned int bot = get_integral_row_left(sw, s);
    return ((top << 12) | (mid << 6) | bot);
}

unsigned int get_integral_val_right(unsigned int n, unsigned int ne, unsigned int c, unsigned int e, unsigned int s, unsigned int se) {
    unsigned int top = get_integral_row_right(n, ne);
    unsigned int mid = get_integral_row_right(c, e);
    unsigned int bot = get_integral_row_right(s, se);
    // printf("center is: %x\n", s);
    // printf("center shift is: %x\n", ((s & 0x1F) << 1));
    // printf("right shift is: %x\n", ((se & (1 << 7)) >> 7));
    // printf("bot is: %x\n", bot);

    return ((top << 12) | (mid << 6) | bot);
}

unsigned char** pack_8(JNIEnv * env, jobjectArray array, int* xlen_store, int* ylen_store) {
  const jsize xlen = (*env)->GetArrayLength(env, array);
  jobjectArray dim1 = (*env)->GetObjectArrayElement(env, array, 0); 
  const jsize ylen = (*env)->GetArrayLength(env, dim1);
  const jsize ylenpacked = ylen/8;
  *xlen_store = xlen;
  *ylen_store = ylen;
  if (ylen % 8) {
    return NULL;
  }
  unsigned char** board = malloc(sizeof(char*)*(xlen+2));
  if (!board)  {
    return NULL;
  }
  board = board+1;
  for (int i = 0; i < xlen; i++) {
      jbooleanArray boolArrayi = (*env)->GetObjectArrayElement(env, array, i);
      jboolean isCopy = JNI_FALSE;
      //entering critical
      jboolean* boolElementsi = (*env)->GetPrimitiveArrayCritical(env, boolArrayi, &isCopy);
      if (isCopy) {
          return NULL;
      }
      //pack
      board[i] = calloc(sizeof(char), ylenpacked+2);
      if (!board[i]) {
        return NULL;
      }
      board[i] = board[i]+1;
      // printf("board[%d] = %x\n", i, board[i]);
      for (int j = 0; j < ylenpacked; j++) {
        signed char c = 0;
        for (int k = 0; k < 8; k++) {
          c<<=1;
          c+=(boolElementsi[j*8+k]);
        }
        board[i][j] = c;
      }
      board[i][-1] = board[i][ylenpacked-1];
      board[i][ylenpacked] = board[i][0];
    (*env)->ReleasePrimitiveArrayCritical(env, boolArrayi, boolElementsi, 0);
    //exiting critical
  }
  board[-1] = board[xlen-1];
  board[xlen] = board[0];
  return board;
}

void unpack_8(JNIEnv* env, jobjectArray array, unsigned char** board, int xlen, int ylenpacked){
  for (int i = 0; i < xlen; i++) {
      jbooleanArray boolArrayi = (*env)->GetObjectArrayElement(env, array, i);
      jboolean* boolElementsi = (*env)->GetPrimitiveArrayCritical(env, boolArrayi, 0);
      for (int j = 0; j < ylenpacked; j++) {
        if (board[i][j]) {
          // printf("not blank!");
        }
        for (int k = 0; k < 8; k++) {
            boolElementsi[j*8+(7-k)] = is_alive(board[i][j], k) ? JNI_TRUE : JNI_FALSE;
        }
      }
      // printf("board[%d] = %x\n", i, board[i]);
      free(board[i]-1);
      (*env)->ReleasePrimitiveArrayCritical(env, boolArrayi, boolElementsi, 0);
    }
    free(board-1);
}

static inline void perform_single_line(int xpos) {
  for (int k = 0; k < ylenpacked; k++) {
        // printf("\nx: %d, y: %d, count: %d, alive: %d\n", j, k*8, buffer[j][k*4].pos0, is_alive(board[j][k], 0));
        // printf("x: %d, y: %d, count: %d, alive: %d\n", j, k*8+1, buffer[j][k*4].pos1, is_alive(board[j][k], 1));
        // printf("x: %d, y: %d, count: %d, alive: %d\n", j, k*8+2, buffer[j][k*4+1].pos0, is_alive(board[j][k], 2));
        // printf("x: %d, y: %d, count: %d, alive: %d\n", j, k*8+3, buffer[j][k*4+1].pos1, is_alive(board[j][k], 3));
        // printf("x: %d, y: %d, count: %d, alive: %d\n", j, k*8+4, buffer[j][k*4+2].pos0, is_alive(board[j][k], 4));
        // printf("x: %d, y: %d, count: %d, alive: %d\n", j, k*8+5, buffer[j][k*4+2].pos1, is_alive(board[j][k], 5));
        // printf("x: %d, y: %d, count: %d, alive: %d\n", j, k*8+6, buffer[j][k*4+3].pos0, is_alive(board[j][k], 6));
        // printf("x: %d, y: %d, count: %d, alive: %d\n", j, k*8+7, buffer[j][k*4+3].pos1, is_alive(board[j][k], 7));
        if(!dirty_bit1[xpos][k]) {
          skip_count++;
          continue;
        }
        non_skip_count++;
        unsigned int lookupvalleft = get_integral_val_left(board1[xpos-1][k-1], board1[xpos-1][k], board1[xpos][k-1], board1[xpos][k], board1[xpos+1][k-1], board1[xpos+1][k]);
        unsigned char newvalleft = lookuptable[lookupvalleft];
        
        unsigned int lookupvalright = get_integral_val_right(board1[xpos-1][k], board1[xpos-1][k+1], board1[xpos][k], board1[xpos][k+1], board1[xpos+1][k], board1[xpos+1][k+1]);
        unsigned char newvalright = lookuptable[lookupvalright];
        
        // printf("Lookup val for x: %x, y: %d left is: %x\n", j, k, lookupvalleft);
        // printf("Lookup val for x: %x, y: %d right is: %x\n", j, k, lookupvalright);
        // printf("Lookup val result left: %x\n", newvalleft);
        // printf("Lookup val result right: %x\n", newvalright);

        unsigned char newval = (newvalleft << 4) | newvalright;
        char xc = newval^board1[xpos][k];
        // printf("Center is: %x\n", board1[j][k]);
        // printf("Lookup result is: %x\n", newval);
        if (xc) {
          dirty_bit2[xpos+1][k] = 1;
          dirty_bit2[xpos][k] = 1;
          dirty_bit2[xpos-1][k] = 1;

          if (is_alive(xc, 0)) {
            dirty_bit2[xpos+1][k+1] = 1;
            dirty_bit2[xpos][k+1] = 1;
            dirty_bit2[xpos-1][k+1] = 1;
          }

          if (is_alive(xc, 7)) {
            dirty_bit2[xpos+1][k-1] = 1;
            dirty_bit2[xpos][k-1] = 1;
            dirty_bit2[xpos-1][k-1] = 1;
          }
          board2[xpos][k] = newval;

        }
      }
}

DWORD WINAPI thread_do_work(void* threadwork) {
    struct thread_work* work = (struct thread_work*) threadwork;
    // printf("Starting thread from %d to %d\n", work->start, work->end);
    fflush(stdout);
    for (int i = 0; i < work->genlength; i++) {
        unsigned char** cache_board1 = board1;
        unsigned char** cache_board2 = board2;
        unsigned char** cache_dirtybit1 = dirty_bit1;
        unsigned char** cache_dirtybit2 = dirty_bit2;

        for (int j = work->start; j < work->end; j++) {
            perform_single_line(j);
        }
        if (EnterSynchronizationBarrier(barrier, 0)) {
            unsigned char** temp;
            temp = dirty_bit1;
            dirty_bit1 = dirty_bit2;
            dirty_bit2 = temp;
            temp = board1;
            board1 = board2;
            board2 = temp;
        }
        for (int j = work->start; j < work->end; j++) {
            cache_dirtybit2[j][0] |= cache_dirtybit2[j][ylenpacked];
            cache_dirtybit2[j][ylenpacked-1] |= cache_dirtybit2[j][-1];

            cache_board2[j][-1] = cache_board2[j][ylenpacked-1];
            cache_board2[j][ylenpacked] = cache_board2[j][0];

            memset(cache_dirtybit1[j], 0, sizeof(char)*ylenpacked);
            
            memcpy(cache_board1[j], cache_board2[j], ylenpacked);
        }
        EnterSynchronizationBarrier(barrier, 0);
    }
    return 0;
}

JNIEXPORT void JNICALL Java_game_1of_1life_GameOfLifeMultithread_getNGenerationNative
  (JNIEnv * env, jobject object, jint threadcount, jbyteArray lookup, jint runlength, jobjectArray array) {
    _CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF);
    int num = 0;
    // int xlen = -1;
    int ylen = -1;
    board1 = pack_8(env, array, &xlen, &ylen);
    if (!board1) {
      if (ylen % 8) {
        printf("Array must have a y size divisable by 8\n");
        return;
      } else {
        printf("Indeterminate error, terminating\n");
        return;
      }
    }
    board2 = pack_8(env, array, &xlen, &ylen);
    if (!board2) {
      printf("Indeterminate error, terminating\n");
      return;
    }
     jboolean isCopy = JNI_FALSE;
      //entering critical
      lookuptable = (*env)->GetPrimitiveArrayCritical(env, lookup, &isCopy);
      if (isCopy) {
          printf("Indeterminate error, terminating\n");
          return;
      }

    ylenpacked = ylen/8;
    int ylenpackedhalf = ylen/2;
    dirty_bit1 = malloc(sizeof(char*)*(xlen+2));
    dirty_bit2 = malloc(sizeof(char*)*(xlen+2));
    dirty_bit1 = dirty_bit1+1;
    dirty_bit2 = dirty_bit2+1;
    for (int i = 0; i < xlen; i++) {
      dirty_bit2[i] = calloc(ylenpacked+2, sizeof(char));
      if (!dirty_bit2[i]) {
        printf("Errno: %d\n", errno);
      }
      dirty_bit2[i] = dirty_bit2[i]+1;
      dirty_bit1[i] = malloc(sizeof(char)*(ylenpacked+2));
      if (!dirty_bit1[i]) {
        printf("Errno: %d\n", errno);
      }
      dirty_bit1[i] = dirty_bit1[i]+1;
      memset(dirty_bit1[i], 1, sizeof(char)*ylenpacked);
    }
    dirty_bit1[-1] = dirty_bit1[xlen-1];
    dirty_bit2[-1] = dirty_bit2[xlen-1];

    dirty_bit1[xlen] = dirty_bit1[0];
    dirty_bit2[xlen] = dirty_bit2[0];
    barrier = malloc(sizeof(SYNCHRONIZATION_BARRIER));
    threadcount = xlen < threadcount ? xlen : threadcount;
    if (!InitializeSynchronizationBarrier(barrier, threadcount, -1)) {
        printf("Barrier initialization failed with error %d, terminating\n", GetLastError());
        return;
    }

    HANDLE* threadpool = malloc(sizeof(HANDLE) * threadcount-1);
    struct thread_work* workpool = malloc(sizeof(struct thread_work)*threadcount-1);
    int startwork = 0;
    // printf("Creating %d threads\n", threadcount);
    for (int i = 0; i < threadcount-1; i++) {
        int amountofwork = (xlen/threadcount) + ((xlen%threadcount) > i);
        workpool[i] = (struct thread_work) {runlength, startwork, startwork+amountofwork};
        threadpool[i] = CreateThread(NULL, 0, thread_do_work, workpool + i, 0, NULL);
        startwork+=amountofwork;
    }
    int amountofwork = (xlen/threadcount);
    struct thread_work finalwork = (struct thread_work) {runlength, startwork, startwork+amountofwork};
    thread_do_work(&finalwork);

    for (int i = 0; i < threadcount - 1; i++) {
        CloseHandle(threadpool[i]);
    }
    (*env)->ReleasePrimitiveArrayCritical(env, lookup, lookuptable, 0);
    unpack_8(env, array, board1, xlen, ylenpacked);
    for (int i = 0; i < xlen; i++) {
      free(board2[i]-1);
      free(dirty_bit1[i]-1);
      free(dirty_bit2[i]-1);
    }
    free(board2-1);
    free(dirty_bit1-1);
    free(dirty_bit2-1);
    free(barrier);
    fflush(stdout);
    }