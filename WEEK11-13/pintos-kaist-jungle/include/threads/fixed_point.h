#define F (1 << 14)  //fixed point 1/
#define INT_MAX ((1 << 31) - 1)
#define INT_MIN (-(1 << 31))
/* ũ�⺰�� ���� �ڷ����� ���ǵ� ��� ����, int_64_t�� ����ϱ� ���� */
#include <stdint.h>
// x and y denote fixed_point numbers in 17.14 format
// n is an integer
int int_to_fp(int n);         /* integer�� fixed point�� ��ȯ */
int fp_to_int_round(int x);   /* FP�� int�� ��ȯ(�ݿø�) */
int fp_to_int(int x);         /* FP�� int�� ��ȯ(����) */
int add_fp(int x, int y);     /* FP�� ���� */
int add_mixed(int x, int n);  /* FP�� int�� ���� */
int sub_fp(int x, int y);     /* FP�� ����(x-y) */
int sub_mixed(int x, int n);  /* FP�� int�� ����(x-n) */
int mult_fp(int x, int y);    /* FP�� ���� */
int mult_mixed(int x, int n); /* FP�� int�� ���� */
int div_fp(int x, int y);     /* FP�� ������(x/y) */
int div_mixed(int x, int n);  /* FP�� int ������(x/n) */

int int_to_fp(int n) {
    return n * F;
}
int fp_to_int_round(int x) {  // �ݿø�
    return (x >= 0) ? (x + F / 2) / F : (x - F / 2) / F;
}
int fp_to_int(int x) {  // ����
    return x / F;
}
int add_fp(int x, int y) { /* FP�� ���� */
    return (x + y);
}
int add_mixed(int x, int n) { /* FP�� int�� ���� */
    return x + n * F;
}
int sub_fp(int x, int y) { /* FP�� ����(x-y) */
    return x - y;
}
int sub_mixed(int x, int n) { /* FP�� int�� ����(x-n) */
    return x - n * F;
}
int mult_fp(int x, int y) { /* FP�� ���� */
    return ((int64_t)x) * y / F;
}
int mult_mixed(int x, int n) { /* FP�� int�� ���� */
    return x * n;
}

int div_fp(int x, int y) { /* FP�� ������(x/y) */
    return ((int64_t)x) * F / y;
}
int div_mixed(int x, int n) { /* FP�� int ������(x/n) */
    return x / n;
}