#define F (1 << 14)  //fixed point 1/
#define INT_MAX ((1 << 31) - 1)
#define INT_MIN (-(1 << 31))
/* Å©±âº°·Î Á¤¼ö ÀÚ·áÇüÀÌ Á¤ÀÇµÈ Çì´õ ÆÄÀÏ, int_64_t¸¦ »ç¿ëÇÏ±â À§ÇÔ */
#include <stdint.h>
// x and y denote fixed_point numbers in 17.14 format
// n is an integer
int int_to_fp(int n);         /* integer¸¦ fixed point·Î ÀüÈ¯ */
int fp_to_int_round(int x);   /* FP¸¦ int·Î ÀüÈ¯(¹İ¿Ã¸²) */
int fp_to_int(int x);         /* FP¸¦ int·Î ÀüÈ¯(¹ö¸²) */
int add_fp(int x, int y);     /* FPÀÇ µ¡¼À */
int add_mixed(int x, int n);  /* FP¿Í intÀÇ µ¡¼À */
int sub_fp(int x, int y);     /* FPÀÇ »¬¼À(x-y) */
int sub_mixed(int x, int n);  /* FP¿Í intÀÇ »¬¼À(x-n) */
int mult_fp(int x, int y);    /* FPÀÇ °ö¼À */
int mult_mixed(int x, int n); /* FP¿Í intÀÇ °ö¼À */
int div_fp(int x, int y);     /* FPÀÇ ³ª´°¼À(x/y) */
int div_mixed(int x, int n);  /* FP¿Í int ³ª´°¼À(x/n) */

int int_to_fp(int n) {
    return n * F;
}
int fp_to_int_round(int x) {  // ¹İ¿Ã¸²
    return (x >= 0) ? (x + F / 2) / F : (x - F / 2) / F;
}
int fp_to_int(int x) {  // ¹ö¸²
    return x / F;
}
int add_fp(int x, int y) { /* FPÀÇ µ¡¼À */
    return (x + y);
}
int add_mixed(int x, int n) { /* FP¿Í intÀÇ µ¡¼À */
    return x + n * F;
}
int sub_fp(int x, int y) { /* FPÀÇ »¬¼À(x-y) */
    return x - y;
}
int sub_mixed(int x, int n) { /* FP¿Í intÀÇ »¬¼À(x-n) */
    return x - n * F;
}
int mult_fp(int x, int y) { /* FPÀÇ °ö¼À */
    return ((int64_t)x) * y / F;
}
int mult_mixed(int x, int n) { /* FP¿Í intÀÇ °ö¼À */
    return x * n;
}

int div_fp(int x, int y) { /* FPÀÇ ³ª´°¼À(x/y) */
    return ((int64_t)x) * F / y;
}
int div_mixed(int x, int n) { /* FP¿Í int ³ª´°¼À(x/n) */
    return x / n;
}