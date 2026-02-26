/*
 * Program  : Count the number of times a given word appears in a sentence
 * Language : CUDA C
 * Concept  : Each GPU thread checks one word token; atomicAdd() ensures
 *            thread-safe increment of the shared counter.
 *
 * Compile  : nvcc word_count_cuda.cu -o word_count_cuda
 * Run      : ./word_count_cuda
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <cuda_runtime.h>

/* ───────────────────────────── Constants ───────────────────────────────── */
#define MAX_SENTENCE_LEN  1024
#define MAX_WORD_LEN      64
#define MAX_WORDS         256
#define THREADS_PER_BLOCK 256

/* ═══════════════════════════════════════════════════════════════════════════
 * DEVICE KERNEL
 * ---------------------------------------------------------------------------
 * One thread  →  one word token
 * If the token matches the target word, atomicAdd() increments d_count.
 * atomicAdd guarantees no two threads corrupt the counter simultaneously.
 * ═══════════════════════════════════════════════════════════════════════════ */
__global__ void countWordKernel(char  *d_words,      /* flat token array    */
                                int    wordSlotLen,  /* bytes per slot      */
                                int    totalWords,   /* number of tokens    */
                                char  *d_target,     /* word to search for  */
                                int    targetLen,    /* length of target    */
                                int   *d_count)      /* output counter      */
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;

    if (idx < totalWords) {

        char *token = d_words + idx * wordSlotLen;  /* this thread's word  */
        int   match = 1;

        /* ── Case-insensitive character comparison ── */
        int i;
        for (i = 0; i < targetLen; i++) {
            if (tolower((unsigned char)token[i]) !=
                tolower((unsigned char)d_target[i])) {
                match = 0;
                break;
            }
        }

        /* Token must end exactly where target ends (no prefix match) */
        if (match && token[targetLen] != '\0') {
            match = 0;
        }

        /* ── ATOMIC INCREMENT ──────────────────────────────────────────────
         * atomicAdd(address, val) :
         *   reads *address, adds val, writes back — all as ONE indivisible
         *   operation.  No race condition even with thousands of threads.
         * ─────────────────────────────────────────────────────────────────*/
        if (match) {
            atomicAdd(d_count, 1);
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HOST HELPER : tokenise sentence → fixed-width word slots
 * ═══════════════════════════════════════════════════════════════════════════ */
int tokenise(const char *sentence, char *words, int slotLen)
{
    char buf[MAX_SENTENCE_LEN];
    strncpy(buf, sentence, MAX_SENTENCE_LEN - 1);
    buf[MAX_SENTENCE_LEN - 1] = '\0';

    int   count = 0;
    char *tok   = strtok(buf, " \t\n\r,.");

    while (tok && count < MAX_WORDS) {
        strncpy(words + count * slotLen, tok, slotLen - 1);
        (words + count * slotLen)[slotLen - 1] = '\0';
        count++;
        tok = strtok(NULL, " \t\n\r,.");
    }
    return count;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HOST HELPER : print a separator line
 * ═══════════════════════════════════════════════════════════════════════════ */
void printLine(void) { printf("------------------------------------------------\n"); }

/* ═══════════════════════════════════════════════════════════════════════════
 * MAIN
 * ═══════════════════════════════════════════════════════════════════════════ */
int main(void)
{
    /* ── 1. Inputs ─────────────────────────────────────────────────────── */
    const char sentence[] =
        "cuda is fun and cuda is powerful and cuda helps "
        "in parallel computing with cuda";
    const char target[] = "cuda";

    printLine();
    printf("   CUDA Word Counter — atomicAdd() Demo\n");
    printLine();
    printf("Sentence : \"%s\"\n", sentence);
    printf("Target   : \"%s\"\n", target);
    printLine();

    /* ── 2. Tokenise on host ───────────────────────────────────────────── */
    char h_words[MAX_WORDS * MAX_WORD_LEN];
    memset(h_words, 0, sizeof(h_words));

    int totalWords = tokenise(sentence, h_words, MAX_WORD_LEN);
    int targetLen  = (int)strlen(target);
    int h_count    = 0;          /* result will be copied back here */

    printf("Total tokens : %d\n", totalWords);
    printf("Tokens       : ");
    for (int i = 0; i < totalWords; i++)
        printf("[%s] ", h_words + i * MAX_WORD_LEN);
    printf("\n");
    printLine();

    /* ── 3. Allocate device memory ─────────────────────────────────────── */
    char *d_words  = NULL;
    char *d_target = NULL;
    int  *d_count  = NULL;

    cudaMalloc((void **)&d_words,  totalWords * MAX_WORD_LEN * sizeof(char));
    cudaMalloc((void **)&d_target, (targetLen + 1)           * sizeof(char));
    cudaMalloc((void **)&d_count,  sizeof(int));

    /* ── 4. Copy host → device ─────────────────────────────────────────── */
    cudaMemcpy(d_words,  h_words,  totalWords * MAX_WORD_LEN * sizeof(char),
               cudaMemcpyHostToDevice);
    cudaMemcpy(d_target, target,   (targetLen + 1)           * sizeof(char),
               cudaMemcpyHostToDevice);
    cudaMemcpy(d_count, &h_count,  sizeof(int),               /* init = 0 */
               cudaMemcpyHostToDevice);

    /* ── 5. Kernel configuration ───────────────────────────────────────── */
    int blocks = (totalWords + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    printf("Grid config  : %d block(s) × %d threads = %d threads total\n",
           blocks, THREADS_PER_BLOCK, blocks * THREADS_PER_BLOCK);
    printLine();

    /* ── 6. Launch kernel ──────────────────────────────────────────────── */
    countWordKernel<<<blocks, THREADS_PER_BLOCK>>>(
        d_words, MAX_WORD_LEN, totalWords, d_target, targetLen, d_count);

    /* ── 7. Synchronise & error check ─────────────────────────────────── */
    cudaDeviceSynchronize();

    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        fprintf(stderr, "CUDA kernel error: %s\n", cudaGetErrorString(err));
        cudaFree(d_words);
        cudaFree(d_target);
        cudaFree(d_count);
        return 1;
    }

    /* ── 8. Copy result device → host ─────────────────────────────────── */
    cudaMemcpy(&h_count, d_count, sizeof(int), cudaMemcpyDeviceToHost);

    /* ── 9. Display result ─────────────────────────────────────────────── */
    printf("Word \"%s\" appears  -->  %d  time(s)\n", target, h_count);
    printLine();

    /* ── 10. Free device memory ────────────────────────────────────────── */
    cudaFree(d_words);
    cudaFree(d_target);
    cudaFree(d_count);

    return 0;
}