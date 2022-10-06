#include <stdlib.h>
#include <string.h>

#include "cuda_runtime.h"
#include "device_launch_parameters.h"


#include "game_of_life_GameOfLifeMultithread.h"

#define is_alive(char, pos) ((1 << (pos)) & (char))
#define set_alive(char, pos) (char |= (1 << (pos)))
#define getlow(n, len) (n-1 >= 0 ? n-1 : len-1)
#define gethigh(n, len) (n+1 < len ? n+1 : 0)
#define get_integral_row_left(left, center) (((left & 1) << 5)) | (center >> 3)
#define get_integral_row_right(center, right) (((center & 0x1F) << 1) | (right & (1 << 7)) >> 7)
#define index(xlen, x, y) (x*xlen+y)
#define MIN(a,b) ((a) < (b) ? (a) : (b))

// jbyte* gpu_lookuptable;
// unsigned char* gpu_board1;
// unsigned char* gpu_board2;


struct packed_short{
    unsigned char pos0 : 4;
    unsigned char pos1 : 4;
};

struct thread_work{
    int genlength;
    int start;
    int end;
};

void barrier_wait();

__device__ unsigned int get_integral_val_left(unsigned int nw, unsigned int n, unsigned int w, unsigned int c, unsigned int sw, unsigned int s) {
    unsigned int top = get_integral_row_left(nw, n);
    unsigned int mid = get_integral_row_left(w, c);
    unsigned int bot = get_integral_row_left(sw, s);
    return ((top << 12) | (mid << 6) | bot);
}

__device__ unsigned int get_integral_val_right(unsigned int n, unsigned int ne, unsigned int c, unsigned int e, unsigned int s, unsigned int se) {
    unsigned int top = get_integral_row_right(n, ne);
    unsigned int mid = get_integral_row_right(c, e);
    unsigned int bot = get_integral_row_right(s, se);
    return ((top << 12) | (mid << 6) | bot);
}

unsigned char* pack_8(JNIEnv* env, jobjectArray array, int* xlen_store, int* ylen_store) {
  const jsize xlen = (env)->GetArrayLength(array);
  jobject dim1 = (env)->GetObjectArrayElement(array, 0); 
  const jsize ylen = (env)->GetArrayLength((jobjectArray)dim1);
  const jsize ylenpacked = ylen/8;
  *xlen_store = xlen;
  *ylen_store = ylen;
  if (ylen % 8) {
    return NULL;
  }
  unsigned char* board = (unsigned char*) malloc(xlen*(ylen%8));
  if (!board) {
    return NULL;
  }
  for (int i = 0; i < xlen; i++) {
      jobject boolArrayi = (env)->GetObjectArrayElement(array, i);
      jboolean isCopy = JNI_FALSE;
      //entering critical
      jboolean* boolElementsi = (jboolean*) (env)->GetPrimitiveArrayCritical((jbooleanArray)boolArrayi, &isCopy);
      if (isCopy) {
          return NULL;
      }
      //pack
      for (int j = 0; j < ylenpacked; j++) {
        signed char c = 0;
        for (int k = 0; k < 8; k++) {
          c<<=1;
          c+=(boolElementsi[j*8+k]);
        }
        board[index(xlen, i, j)] = c;
      }
    (env)->ReleasePrimitiveArrayCritical((jbooleanArray)boolArrayi, boolElementsi, 0);
    //exiting critical
  }
  return board;
}

void unpack_8(JNIEnv* env, jobjectArray array, unsigned char* board, int xlen, int ylenpacked){
  for (int i = 0; i < xlen; i++) {
      jobjectArray boolArrayi = (jobjectArray)(env)->GetObjectArrayElement((jobjectArray)array, i);
      jboolean* boolElementsi = (jboolean*)(env)->GetPrimitiveArrayCritical((jbooleanArray)boolArrayi, 0);
      for (int j = 0; j < ylenpacked; j++) {
        for (int k = 0; k < 8; k++) {
            boolElementsi[j*8+(7-k)] = is_alive(board[index(xlen, i, j)], k) ? JNI_TRUE : JNI_FALSE;
        }
      }
      (env)->ReleasePrimitiveArrayCritical(boolArrayi, boolElementsi, 0);
    }
    free(board);
}
__device__ void perform_single_line(int xpos, int xlen, int ylenpacked, jbyte* gpu_lookuptable, unsigned char* gpu_board1, unsigned char* gpu_board2) {
  for (int k = 0; k < ylenpacked; k++) {
        int up =    getlow(xpos, xlen);
        int down =  gethigh(xpos, xlen);
        int left =  getlow(k, ylenpacked);
        int right = gethigh(k, ylenpacked);
        
        unsigned int lookupvalleft = get_integral_val_left(gpu_board1[index(xlen, up, left)], gpu_board1[index(xlen, up, k)], gpu_board1[index(xlen, xpos, left)], gpu_board1[index(xlen, xpos, k)], gpu_board1[index(xlen, down, left)], gpu_board1[index(xlen, down, k)]);
        unsigned char newvalleft = gpu_lookuptable[lookupvalleft];
        
        unsigned int lookupvalright = get_integral_val_right(gpu_board1[index(xlen, up, k)], gpu_board1[index(xlen, up, right)], gpu_board1[index(xlen, xpos, k)], gpu_board1[index(xlen, xpos, right)], gpu_board1[index(xlen, down, k)], gpu_board1[index(xlen, down, right)]);
        unsigned char newvalright = gpu_lookuptable[lookupvalright];
        
        unsigned char newval = (newvalleft << 4) | newvalright;
        gpu_board2[index(xlen, xpos, k)] = newval;
      }
}
__global__ void thread_do_work(int xlen, int ylenpacked, int genlength, jbyte* gpu_lookuptable, unsigned char* gpu_board1, unsigned char* gpu_board2) {
    int threadId = (blockIdx.x * blockDim.x) + threadIdx.x;
    int totalThreadCount = (gridDim.x * blockDim.x);
    int start = (threadId * (xlen/totalThreadCount)) + MIN(threadId, xlen%totalThreadCount);
    int length = xlen/totalThreadCount + (threadId < (xlen%totalThreadCount));
    for (int i = 0; i < genlength; i++) {
        for (int j = 0; j < length; j++) {
            perform_single_line(j+start, xlen, ylenpacked, gpu_lookuptable, gpu_board1, gpu_board2);
        }
      unsigned char* temp;
      temp = gpu_board1;
      gpu_board1 = gpu_board2;
      gpu_board2 = temp;
      __syncthreads();
    }
}

JNIEXPORT void JNICALL Java_game_1of_1life_GameOfLifeMultithread_getNGenerationNative
  (JNIEnv * env, jobject object, jint threadcount, jbyteArray lookup, jint runlength, jobjectArray array) {
    unsigned long long skip_count = 0;
    unsigned long long non_skip_count = 0;
    int num = 0;
    int xlen = -1;
    int ylen = -1;
    unsigned char* board = pack_8(env, array, &xlen, &ylen);
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
    jboolean isCopy = JNI_FALSE;
      //entering critical
      jbyte* lookuptable = (jbyte*) (env)->GetPrimitiveArrayCritical(lookup, &isCopy);
      // if (isCopy) {
      //     printf("Indeterminate error, terminating\n");
      //     return;
      // }
      jbyte* gpu_lookuptable;
      unsigned char* gpu_board1;
      unsigned char* gpu_board2;
      cudaError_t error;
      error = cudaMalloc(&gpu_lookuptable, sizeof(*gpu_lookuptable)*(1<<18));
      if (error) {
        printf("Failed with %s\n", cudaGetErrorString(error));
      }
      error = cudaMemcpy(lookuptable, gpu_lookuptable, sizeof(*gpu_lookuptable)*(1<<18), cudaMemcpyHostToDevice);
      if (error) {
        printf("Failed with %s\n", cudaGetErrorString(error));
      }
      error = cudaMalloc(&gpu_board1, xlen*ylenpacked*sizeof(*gpu_board1));
      if (error) {
        printf("Failed with %s\n", cudaGetErrorString(error));
      }
      error = cudaMemcpy(board, gpu_board1, xlen*ylenpacked*sizeof(*gpu_board1), cudaMemcpyHostToDevice);
      if (error) {
        printf("Failed with %s\n", cudaGetErrorString(error));
      }
      error = cudaMalloc(&gpu_board2, xlen*ylenpacked*sizeof(*gpu_board2));
      if (error) {
        printf("Failed with %s\n", cudaGetErrorString(error));
      }
    thread_do_work<<<xlen/32, 32>>>(xlen, ylenpacked, runlength, gpu_lookuptable, gpu_board1, gpu_board2);
    cudaDeviceSynchronize();
    // int errorval = 0;
    cudaMemcpy(board, gpu_board1, xlen*ylenpacked*sizeof(*gpu_board1), cudaMemcpyDeviceToHost);
    cudaFree(gpu_lookuptable);
    cudaFree(gpu_board1);
    cudaFree(gpu_board2);
    unpack_8(env, array, board, xlen, ylenpacked);
    }