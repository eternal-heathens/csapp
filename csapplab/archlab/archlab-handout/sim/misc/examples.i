# 0 "examples.c"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "examples.c"
# 10 "examples.c"
typedef struct ELE {
    long val;
    struct ELE *next;
} *list_ptr;


long sum_list(list_ptr ls)
{
    long val = 0;
    while (ls) {
 val += ls->val;
 ls = ls->next;
    }
    return val;
}


long rsum_list(list_ptr ls)
{
    if (!ls)
 return 0;
    else {
 long val = ls->val;
 long rest = rsum_list(ls->next);
 return val + rest;
    }
}


long copy_block(long *src, long *dest, long len)
{
    long result = 0;
    while (len > 0) {
 long val = *src++;
 *dest++ = val;
 result ^= val;
 len--;
    }
    return result;
}
