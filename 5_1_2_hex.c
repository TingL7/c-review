#include <stdint.h>
#include <stdio.h>

union u_qword {
    struct {
        uint32_t low;
        uint32_t high;
    } dwords;
    uint64_t qword;
};

struct udiv_result {
    union u_qword q;
    union u_qword r;
};

int Func32(uint32_t x)
{
    return x ? Func32(x >> 1) - 1:32;
}

static inline int Func64(uint64_t val)
{
    uint32_t lo = (uint32_t) val;
    uint32_t hi = (uint32_t) (val >> 32);
    return hi ? Func32(hi) : 32 + Func32(lo);
}

static int do_udiv32(uint32_t dividend,
                     uint32_t divisor,
                     struct udiv_result *res)
{
    /* Ensure dividend is always greater than or equal to the divisor. */
    uint32_t mask = Func32(divisor) - Func32(dividend);

    divisor <<= mask;  /* align divisor */
    mask = 1U << mask; /* align dividend */
    do {
        if (dividend >= divisor) {
            dividend -= divisor;
            res->q.dwords.low |= mask;
        }
        divisor >>= 1;
    } while ((mask >>= 1) && dividend);
    res->r.dwords.low = dividend;
    return 0;
}

int udiv64(uint64_t dividend, uint64_t divisor, struct udiv_result *res)
{
    uint64_t mask;
    uint64_t bits;

    res->q.qword = res->r.qword = 0;
    if (divisor == 0) { /* division by 0 ? */
        res->q.qword = 0xffffffffffffffffull;
        return -1;
    }
    if (divisor == dividend) {
        res->q.qword = 1;
        return 0;
    }
    if (divisor > dividend) {
        res->r.qword = dividend;
        return 0;
    }
    /* only 32 bit operands that the preconditions are fulfilled. */
    if (!(divisor >> 32) && !(dividend >> 32))
        return do_udiv32((uint32_t) dividend, (uint32_t) divisor, res);

    bits = Func64(divisor) - Func64(dividend);
    divisor <<= bits; /* align divisor and dividend */
    mask = 1ULL << bits;
    /* division loop */
    do {
        if (dividend >= divisor) {
            dividend -= divisor;
            res->q.qword |= mask;
        }
        divisor >>= 1;
        mask >>= 1;
    } while ((mask != 0) && dividend);

    res->r.qword = dividend;
    return 0;
}

int main()
{
    char digitbuff[20];
    char *pos = digitbuff + sizeof(digitbuff);
    union u_qword v;  /* current value */
    union u_qword nv; /* next value */
    struct udiv_result d;

    int64_t value = 0x345CAFEBABE; /* Java classfile magic number */
    v.qword = (unsigned long long) value;
    while (v.dwords.high != 0) { /* process 64 bit value as long as needed */
        /* determine digits from right to left */
        udiv64(v.qword, 16, &d);
        *--pos = d.r.dwords.low + ((d.r.dwords.low > 9) ? 0x37 :  0x30);
        v.qword = d.q.qword;
    }

    do { /* process 32 bit (or reduced 64 bit) value */
        nv.dwords.low = v.dwords.low / 16;
        *--pos = (v.dwords.low - (16 * nv.dwords.low)) + (((v.dwords.low - (16 * nv.dwords.low)) > 9) ? 0x37 : 0x30);
    } while ((v.dwords.low = nv.dwords.low) != 0);

    int len = digitbuff + sizeof(digitbuff) - pos;
    char *p = pos;
    while (len--)
        putchar(*p++);
    printf("\n");
    printf("%lx\n", value);
    return 0;
}
