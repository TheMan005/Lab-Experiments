/*
 * Program  : CUDA String RS Builder
 * ---------------------------------------------------------------------------
 * Input  S  : PCAP  (length N = 4)
 * Output RS : PCAPPCAPCP  (length = N*(N+1)/2 = 10)
 *
 * Rule     : Thread i copies (i+1) characters from S into RS.
 *            Thread i writes at offset = 0+1+2+...+i = i*(i+1)/2
 *
 *  Thread 0  →  copies 1 char  →  "P"     at offset 0
 *  Thread 1  →  copies 2 chars →  "PC"    at offset 1
 *  Thread 2  →  copies 3 chars →  "PCA"   at offset 3
 *  Thread 3  →  copies 4 chars →  "PCAP"  at offset 6
 *  Result    →  P | PC | PCA | PCAP  =  "PCAPPCAPCP"
 *
 * Compile  : nvcc rs_string.cu -o rs_string
 * Run      : ./rs_string
 * ═══════════════════════════════════════════════════════════════════════════ */

#include <stdio.h>
#include <string.h>
#include <cuda_runtime.h>

/* ───────────────────────────── Constants ───────────────────────────────── */
#define MAX_LEN 256

/* ═══════════════════════════════════════════════════════════════════════════
 * DEVICE KERNEL
 * ---------------------------------------------------------------------------
 * Each thread i :
 *   1. Computes its write offset  = i*(i+1)/2
 *   2. Copies (i+1) characters from d_S  into  d_RS[offset]
 *      Characters are taken cyclically from d_S  (index mod N).
 * ═══════════════════════════════════════════════════════════════════════════ */
__global__ void buildRS(const char *d_S,    /* input string                 */
                        int         N,      /* length of S                  */
                        char       *d_RS)   /* output string (pre-zeroed)   */
{
    int i = blockIdx.x * blockDim.x + threadIdx.x;  /* thread index         */

    if (i < N) {
        /* ── Offset where this thread starts writing ── */
        int offset = i * (i + 1) / 2;

        /* ── Copy (i+1) characters from S (cyclic) ── */
        int numChars = i + 1;
        for (int j = 0; j < numChars; j++) {
            d_RS[offset + j] = d_S[j % N];   /* cyclic wrap via mod         */
        }
    }
}

/* ───────────────────────────── Separator ───────────────────────────────── */
void printLine(void) { printf("------------------------------------------------\n"); }

/* ═══════════════════════════════════════════════════════════════════════════
 * MAIN
 * ═══════════════════════════════════════════════════════════════════════════ */
int main(void)
{
    /* ── 1. Input ──────────────────────────────────────────────────────── */
    const char h_S[] = "PCAP";
    int N             = (int)strlen(h_S);
    int rsLen         = N * (N + 1) / 2;   /* = 4*5/2 = 10 for "PCAP"      */

    printLine();
    printf("   CUDA RS String Builder\n");
    printLine();
    printf("Input  S  (length %d) : %s\n", N, h_S);
    printf("Output RS (length %d) : ?\n",  rsLen);
    printLine();

    /* ── 2. Show thread plan ───────────────────────────────────────────── */
    printf("Thread plan:\n");
    for (int i = 0; i < N; i++) {
        int offset   = i * (i + 1) / 2;
        int numChars = i + 1;
        printf("  Thread %d  →  copies %d char(s) from S  →  offset %d\n",
               i, numChars, offset);
    }
    printLine();

    /* ── 3. Host memory ────────────────────────────────────────────────── */
    char h_RS[MAX_LEN];
    memset(h_RS, 0, sizeof(h_RS));

    /* ── 4. Device memory ──────────────────────────────────────────────── */
    char *d_S  = NULL;
    char *d_RS = NULL;

    cudaMalloc((void **)&d_S,  (N + 1)      * sizeof(char));
    cudaMalloc((void **)&d_RS, (rsLen + 1)  * sizeof(char));

    /* ── 5. Copy S to device; zero-fill d_RS ──────────────────────────── */
    cudaMemcpy(d_S,  h_S,   (N + 1) * sizeof(char), cudaMemcpyHostToDevice);
    cudaMemset(d_RS, 0,     (rsLen + 1) * sizeof(char));

    /* ── 6. Kernel configuration & launch ─────────────────────────────── */
    int threadsPerBlock = 256;
    int blocks          = (N + threadsPerBlock - 1) / threadsPerBlock;

    printf("Kernel config : %d block(s), %d thread(s)/block\n",
           blocks, threadsPerBlock);
    printLine();

    buildRS<<<blocks, threadsPerBlock>>>(d_S, N, d_RS);

    /* ── 7. Synchronise & check errors ────────────────────────────────── */
    cudaDeviceSynchronize();

    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        fprintf(stderr, "CUDA error: %s\n", cudaGetErrorString(err));
        cudaFree(d_S);
        cudaFree(d_RS);
        return 1;
    }

    /* ── 8. Copy RS back to host ───────────────────────────────────────── */
    cudaMemcpy(h_RS, d_RS, (rsLen + 1) * sizeof(char), cudaMemcpyDeviceToHost);
    h_RS[rsLen] = '\0';   /* null-terminate */

    /* ── 9. Display result ─────────────────────────────────────────────── */
    printf("Input  S  : %s\n", h_S);
    printf("Output RS : %s\n", h_RS);
    printLine();

    /* ── 10. Visual segment breakdown ─────────────────────────────────── */
    printf("Segment breakdown:\n");
    int pos = 0;
    for (int i = 0; i < N; i++) {
        int numChars = i + 1;
        printf("  Thread %d  →  \"", i);
        for (int j = 0; j < numChars; j++) putchar(h_RS[pos++]);
        printf("\"\n");
    }
    printLine();

    /* ── 11. Free device memory ────────────────────────────────────────── */
    cudaFree(d_S);
    cudaFree(d_RS);

    return 0;
}