/**
 * @brief
 * JNI implementation for a hyper-optimized Conway's Game of Life
 * 
 * @author Shmuel Newmark <https://github.com/synewmark>
 * Except barrier_wait() which was taken from User Tsyvarev on StackOveflow: 
 * https://stackoverflow.com/questions/33598686/spinning-thread-barrier-using-atomic-builtins
 * 
 * @details
 * Code packs 8 booleans into a single char value (@see pack_8(), and unpack_8() methods)
 * 
 * For example a boolean array like this:
 * [0] [1] [2] [3] [4] [5] [6] [7] [8] ... 
 * ..0 ..0 ..1 ..0 ..1 ..0 ..1 ..1 ..1 ...
 * Will turn into:
 *   [0]        [1]
 * 00101011  1.......
 * 
 * #IMPORTANT# Throughout code, "leftmost" bit is the highest bit and "rightmost" is lowest
 * However, the lowest value on the array is left and highest is right
 * Because numerical values are usually read most sig to least, this simplifies
 * the intuition when it comes to merging values but must be kept consistent
 * 
 * Code uses a "lookup table" to calculate values
 * Lookup is done 18 bits at a time as a 6x3.
 * Because we're storing values packed in 8 bits this requires 2 lookups
 * For example suppose the following section of a life board:
 * ... ........ ...
 * ..0 00101011 1..
 * ..1 10100101 1..
 * ..1 01010010 0..
 * ... ........ ...
 * If we wanted to lookup the value of the middle row we'd need to follow 2 steps
 * Step 1. split the row in half and attach the adjacent values on either side
 * For the left value that would look like:
 * 1 10100
 * But we also need to the row on top and bottom so,
 * 0 00101
 * 1 10100
 * 1 01010
 * Step 2. we'd need to combine those into a single bit string: 000101110100101010
 * Same deal for the right:
 * 01011 1
 * 00101 1
 * 10010 0
 * As a string: 010111001011100100
 * We'll use the int value of both of these bit strings as indices on our lookup table
 * 
 * Code favors speed over clarity for all innermost loop functions:
 * @see get_integral_val_left(), get_integral_val_right(), perform_single_line(), thread_do_work()
 */

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "game_of_life_GameOfLifeMultithread.h"

#define is_alive(char, pos) ((1 << (pos)) & (char))
#define set_alive(char, pos) (char |= (1 << (pos)))
// Don't use mod for wrapping because division is too expensive
#define get_low(n, len) (n-1 >= 0 ? n-1 : len-1)
#define get_high(n, len) (n+1 < len ? n+1 : 0)
// Step 1 for left: get the 5 left bits and the rightmost bit on the one to the left
#define get_integral_row_left(left, center) (((left & 1) << 5)) | (center >> 3)
// Step 1 for right: get the 5 right bits and the leftmost bit on the one to the right
#define get_integral_row_right(center, right) (((center & 0x1F) << 1) | (right & (1 << 7)) >> 7)
#define swap_board(x, y) ({unsigned char** _temp = x; x = y; y =_temp;})


jbyte* lookuptable = NULL;
unsigned char** board1 = NULL;
unsigned char** board2 = NULL;
unsigned char** dirty_bit1 = NULL;
unsigned char** dirty_bit2 = NULL;


int xlen;
int ylen;
int ylenpacked;

int global_thread_count = 8;

struct thread_work{
    int genlength;
    int start;
    int end;
};

static void barrier_wait();

static void* safe_calloc(size_t NumOfElements, size_t SizeOfElements) {
  // malloc family can return NULL for 0 allocs
  // Want to distinguish between a failure and a "succesful" 0 allocation
  if (!NumOfElements || !SizeOfElements) {
    return NULL;
  }
  errno = 0;
  void* result = calloc(NumOfElements, SizeOfElements);
  if (!result) {
    printf("Calloc of size %lld failed with code: %d\n", NumOfElements*SizeOfElements, errno);
  }
  return result;
}

static void free2d(int x, unsigned char** array) {
  if (!array) {
    return;
  }
  for (int i = 0; i < x; i++) {
    if (!array[i]) {
      return;
    }
    free(array[i]);
  }
  free(array);
}

static unsigned char** malloc2d(int x, int y) {
  unsigned char** board = safe_calloc(x, sizeof(*board));
  if (!board) {
    return NULL;
  }
  for (int i = 0; i < x; i++) {
    if (!(board[i] = safe_calloc(y, sizeof(*board[i])))) {
      free2d(x, board);
      return NULL;
    }
  }
  return board;
}

static int set_length_values(JNIEnv * env, jobjectArray array, int* xlen_store, int* ylen_store, int* ylenpacked_store) {
  // Caller ensures the array is not of 0 length and is a square 
  // so we just need the x and any y lengths
  int xlen = (*env)->GetArrayLength(env, array);
  jobjectArray dim1 = (*env)->GetObjectArrayElement(env, array, 0);
  int ylen = (*env)->GetArrayLength(env, dim1);
  if (ylen % 8) {
    return -1;
  }
  *xlen_store = xlen;
  *ylen_store = ylen;
  *ylenpacked_store = ylen/8;
  return 0;
}

static int pack_8(JNIEnv * env, jobjectArray array, unsigned char** board) {
  for (int i = 0; i < xlen; i++) {
      jbooleanArray boolArrayi = (*env)->GetObjectArrayElement(env, array, i);
      jboolean isCopy = JNI_FALSE;
      //entering critical
      jboolean* boolElementsi = (*env)->GetPrimitiveArrayCritical(env, boolArrayi, &isCopy);
      if (isCopy) {
          return -1;
      }
      //pack 8 boolean values into each char
      for (int j = 0; j < ylenpacked; j++) {
        unsigned char c = 0;
        // This places the lowest bit as the most significant
        // @see @details
        for (int k = 0; k < 8; k++) {
          c<<=1;
          c+=(boolElementsi[j*8+k]);
        }
        board[i][j] = c;
      }
    (*env)->ReleasePrimitiveArrayCritical(env, boolArrayi, boolElementsi, 0);
    //exiting critical
  }
  return 0;
}

static void unpack_8(JNIEnv* env, jobjectArray array, unsigned char** board){
  for (int i = 0; i < xlen; i++) {
      jbooleanArray boolArrayi = (*env)->GetObjectArrayElement(env, array, i);
      jboolean* boolElementsi = (*env)->GetPrimitiveArrayCritical(env, boolArrayi, 0);
      for (int j = 0; j < ylenpacked; j++) {
        for (int k = 0; k < 8; k++) {
          /** We cannot simply mask and place back into the array because Java only reliably treats values with the lowest bit set as true
          * Somewhat amusingly, not doing this creates less problems than you might imagine
          * in my case failing to do so still had comparing individual values perform as expected
          * printing the values also printed True no matter which bit was set and False otherwise
          * It was only when I used Arrays.equals() that it broke
          * This isn't exactly shocking: a compare should really just be a xor or a test
          * whereas an arrays.equals would be optimized to a streaming extension
          * it's just a good example of how undefined behavior doesn't mean "blows up in your face immediately"
          */
          boolElementsi[j*8+(7-k)] = is_alive(board[i][j], k) ? JNI_TRUE : JNI_FALSE;
          // We want to put the value in k-7 because we reversed the bits when packing
          // So we need to un-reverse the bits when unpacking
          // @see @details
        }
      }
      (*env)->ReleasePrimitiveArrayCritical(env, boolArrayi, boolElementsi, 0);
    }
}

// Very performance sensitive: Runs g*x*(y/8) times
static inline unsigned int get_integral_val_left(unsigned int nw, unsigned int n, unsigned int w, unsigned int c, unsigned int sw, unsigned int s) {
    unsigned int top = get_integral_row_left(nw, n);
    unsigned int mid = get_integral_row_left(w, c);
    unsigned int bot = get_integral_row_left(sw, s);
    // Step 2: combine all 3 rows into a single bit string
    return ((top << 12) | (mid << 6) | bot);
}
// Very performance sensitive: Runs g*x*(y/8) times
static inline unsigned int get_integral_val_right(unsigned int n, unsigned int ne, unsigned int c, unsigned int e, unsigned int s, unsigned int se) {
    unsigned int top = get_integral_row_right(n, ne);
    unsigned int mid = get_integral_row_right(c, e);
    unsigned int bot = get_integral_row_right(s, se);
    // Step 2: combine all 3 rows into a single bit string
    return ((top << 12) | (mid << 6) | bot);
}
// Very performance sensitive: runs g*x times
static inline void perform_single_line(int xpos, unsigned char** board1, unsigned char** board2, unsigned char** dirty_bit1, unsigned char** dirty_bit2) {
  // inner loop runs: runs g*x*y times
  for (int i = 0; i < ylenpacked; i++) {
        // If dirty bit is clear we don't run
        if(!dirty_bit1[xpos][i]) {
          continue;
        }
        // Watch for over and underflow when wrapping
        int up = get_low(xpos, xlen);
        int down = get_high(xpos, xlen);
        int left = get_low(i, ylenpacked);
        int right = get_high(i, ylenpacked);
        // Split the lookup into 2 request: left and right
        // Each request is 18 bits, or 6 for each row
        unsigned int lookupvalleft = get_integral_val_left(board1[up][left], board1[up][i], board1[xpos][left], board1[xpos][i], board1[down][left], board1[down][i]);
        unsigned char newvalleft = lookuptable[lookupvalleft];
        
        unsigned int lookupvalright = get_integral_val_right(board1[up][i], board1[up][right], board1[xpos][i], board1[xpos][right], board1[down][i], board1[down][right]);
        unsigned char newvalright = lookuptable[lookupvalright];

        unsigned char newval = (newvalleft << 4) | newvalright;
        char xc = newval^board1[xpos][i];
        
        // Set all dirty bits around the just changed value
        // Even though we may have a race condition on access it's safe 
        // because char access is atomic and we're only ever setting them, never clearing
        // If there's any difference between the curr and prev value we must set the dirty on the ones above and below
        // But we only need to set the left and right dirty values if the left and right most values were changed
        if (xc) {
          dirty_bit2[down][i] = 1;
          dirty_bit2[xpos][i] = 1;
          dirty_bit2[up][i] = 1;

          if (is_alive(xc, 0)) {
            dirty_bit2[down][right] = 1;
            dirty_bit2[xpos][right] = 1;
            dirty_bit2[up][right] = 1;
          }

          if (is_alive(xc, 7)) {
            dirty_bit2[down][left] = 1;
            dirty_bit2[xpos][left] = 1;
            dirty_bit2[up][left] = 1;
          }
        }
        board2[xpos][i] = newval;
      }
}

void* thread_do_work(void* threadwork) {
  struct thread_work* work = (struct thread_work*) threadwork;
  unsigned char** cache_board1 = board1;
  unsigned char** cache_board2 = board2;
  unsigned char** cache_dirtybit1 = dirty_bit1;
  unsigned char** cache_dirtybit2 = dirty_bit2;
  // Somewhat performace sensitive: each barrier_wait runs g times
  for (int i = 0; i < work->genlength; i++) {
    for (int j = work->start; j < work->end; j++) {
        perform_single_line(j, cache_board1, cache_board2, cache_dirtybit1, cache_dirtybit2);
    }
    barrier_wait();
    swap_board(cache_board1, cache_board2);
    swap_board(cache_dirtybit1, cache_dirtybit2);
    for (int j = work->start; j < work->end; j++) {
      // clear all dirty bits after reading them
      // The big drawback of the approach is that it requires 2 barrier waits
      // Otherwise we risk clearing the bits while another thread is reading them
      memset(cache_dirtybit2[j], 0, ylenpacked);
    }
    barrier_wait();
  }
  return NULL;
}

static int initialize_boards(JNIEnv* env, jobjectArray array) {
  board1 = malloc2d(xlen, ylenpacked);
  if (!board1) {        
    printf("Indeterminate error, terminating\n");
    return -1;
  }
  if (pack_8(env, array, board1)) {
    printf("Indeterminate error, terminating\n");
    return -1;
  }
  board2 = malloc2d(xlen, ylenpacked);
  if (!board2) {
    printf("Indeterminate error, terminating\n"); 
    return -1;
  }
  return 0;
}

static int initialize_dirty_bits(int xlen, int ylenpacked) {
  dirty_bit1 = malloc2d(xlen, ylenpacked);
  if (!dirty_bit1) {
    return -1;
  }
  // turn on all dirty bits on the first run so nothing gets skipped
  for (int i = 0; i < xlen; i++) {
    memset(dirty_bit1[i], 0xFF, ylenpacked);
  }

  dirty_bit2 = malloc2d(xlen, ylenpacked);
  if (!dirty_bit2) {
    return -1;
  }
  return 0;
}

static void initialize_lookup_table(JNIEnv* env, jarray array) {
  lookuptable = (*env)->GetPrimitiveArrayCritical(env, array, NULL);
}

static void free_all_resources() {
  free2d(xlen, board1);
  free2d(xlen, board2);
  free2d(xlen, dirty_bit1);
  free2d(xlen, dirty_bit2);
}

static int do_all_thread_work(int threadcount, int runlength) {
  // Each thread works on it's own x range, no reason to make more threads than xlen
  global_thread_count = xlen < threadcount ? xlen : threadcount;
  struct thread_work workpool[global_thread_count];
  int startwork = 0;
  for (int i = 0; i < global_thread_count-1; i++) {
    // If there are more threads
    int amountofwork = (xlen/global_thread_count) + ((xlen%global_thread_count) > i);
    workpool[i] = (struct thread_work) {runlength, startwork, startwork+amountofwork};
    // The possibility of some threads being created succesfully, but choking on the rest is unhandled
    int error;
    if ((error = (pthread_create(NULL, 0, thread_do_work, workpool+i)))) {
      printf("Creating thread: %d failed with code: %d\n", i+1, error);
      return -1;
    }
    startwork+=amountofwork;
  }
  int amountofwork = (xlen/global_thread_count);
  struct thread_work finalwork = (struct thread_work) {runlength, startwork, startwork+amountofwork};
  thread_do_work(&finalwork);
  return 0;
}

static int initialize_all_resources(JNIEnv* env, jint threadcount, jbyteArray lookup, jint runlength, jobjectArray array) {
  if (set_length_values(env, array, &xlen, &ylen, &ylenpacked)) {
    printf("Array must have a y size divisable by 8\n");
    return -1;
  }
  if (initialize_boards(env, array)) {
    return -1;
  }
  if (initialize_dirty_bits(xlen, ylenpacked)) {
    return -1;
  }
  initialize_lookup_table(env, lookup);
  do_all_thread_work(threadcount, runlength);
  return 0;
}

JNIEXPORT void JNICALL Java_game_1of_1life_GameOfLifeMultithread_getNGenerationNative
  (JNIEnv* env, jobject object, jint threadcount, jbyteArray lookup, jint runlength, jobjectArray array) {
  printf("Running for %d generations!\n", runlength);
  if (!initialize_all_resources(env, threadcount, lookup, runlength, array)) {
    unsigned char** finalboard = runlength % 2 ? board2 : board1;  
    unpack_8(env, array, finalboard);
  }
  free_all_resources();
}

// Barrier code is taken from User Tsyvarev: 
// https://stackoverflow.com/questions/33598686/spinning-thread-barrier-using-atomic-builtins

// Because we're saturating the cores and denying multitasking, there's no reason to sleep on waits
// So we're busy-waiting instead to squeeze out a bit more performance

int bar = 0; // Counter of threads, faced barrier.
volatile int passed = 0; // Number of barriers, passed by all threads.

// Due to __sync primitives, code is GCC/Clang dependant
static void barrier_wait()
{
    int passed_old = passed; // Should be evaluated before incrementing *bar*!

    if(__sync_fetch_and_add(&bar,1) == (global_thread_count - 1))
    {
        // The last thread, faced barrier.
        bar = 0;
        // *bar* should be reseted strictly before updating of barriers counter.
        __sync_synchronize(); 
        passed++; // Mark barrier as passed.
        // return 1;
    }
    else
    {
        // Not the last thread. Wait others.
        while(passed == passed_old) {};
        // Need to synchronize cache with other threads, passed barrier.
        __sync_synchronize();
        // return 0;
    }
}