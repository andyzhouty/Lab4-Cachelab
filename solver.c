/*
 * solver.c - Starter for Cache Lab Bonus.
 * solve() is called once per mystery cache. See solver.h and README.md.
 */
#include "solver.h"
#define u32 uint32_t
#define VERIFICATION_TURNS 600

static int all_hit(u32 stride, int n) {
    cache_flush();
    for (int i = 0; i < n; i++) cache_load(i * stride);
    for (int i = 0; i < n; i++) {
        if (!cache_load(i * stride)) return 0;
    }
    return 1;
}

static int find_E(int b) {
    u32 stride = 1u << (b + 9);
    for (int i = 1;;i++) {
        if (!all_hit(stride, i)) {
            return i-1;
        }
    }
}

static int find_s(int b, int E) {
    u32 stride = 1u << b;
    for (int i = 0;;i++) {
        if (!all_hit(stride, E+1)) {
            return i;
        }
        stride <<= 1;
    }
}

static int detect_write_allocate(void) {
    cache_flush();
    cache_store(0);
    return cache_load(0);
}

static int sample_policy(int s, int b, int E) {
    cache_flush();
    int addr = 0;
    for (int i = 0; i < E; i++) {
        cache_load(addr);
        addr += (1u << (s + b));
    }
    cache_load(0);
    cache_load(addr);
    return cache_load(0) ? POLICY_LRU : POLICY_FIFO;
}

static int detect_policy(int s, int b, int E) {
    if (E == 1) return POLICY_LRU;
    int policy = sample_policy(s, b, E);
    for (int i = 0; i < VERIFICATION_TURNS; i++) {
        if (policy != sample_policy(s, b, E)) {
            return POLICY_RANDOM;
        }
    }
    return policy;
}

void solve(void)
{
    int b = 0;

    /* Load block 0. The first power-of-two offset that misses is its size. */
    cache_flush();
    cache_load(0);
    while (b < 31 && cache_load(1u << b))
        b++;

    /* TODO: Find s, E, policy and write_allocate, then report them here. */
    int E = find_E(b);
    int s = find_s(b, E);
    report(b, s, E, detect_policy(s, b, E), detect_write_allocate());
}
