/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  return ~(x & y) & ~( ~x & ~y );
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  return 1 << 31;

}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  //0和10···相反数都是自己
  x = ~x;
  int y = ((~x+1)^x);
  //这里很奇怪  !((~x+1)^x)   isTmax(2147483647[0x7fffffff])显示的值为0,需要单独用局部变量保存，再!y，很奇怪，不知道处理机制导致的
  return !y & !!(x);
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  /*要先*去除掉偶数位的影响在比较相等*/
  int mask = 0xAA+(0xAA<<8)+(0xAA<<16)+(0xAA<<24);
  return !(mask^(x&(mask)));
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return ~x+1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
   /* x的前缀是否为0011，且后缀是否有大于1000的数*/
  return (!(0x3^(x>>4)))&(!((!!(0x8&x))&(!!(0x6&x))));
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  /*需要x为0的时候 跟y的表达式为0 跟z的表达式为z， 
    当x不为0的时候跟y的表达式为y，跟z的表达式为0，
    1. 通过相同与x，不同的运算符能否实现完全兑成的语意？ 不行，找不到，而应该从值本身的完全相反
    2. x能相反，如x0为0，-0 为0···, -1为1···，用~x+1可以实现用相同的符号构成表达式
  */
  int negateBoolValue =  ~!!x+1;
  return (negateBoolValue&y) | ((~negateBoolValue) & z);
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  // a<b -> a-b < 0  小于0 可以通过与最小值与，为0则表明a-b大于0，不为0则大于0
  // 但是a-b 与最小值与的前提是 a-b 不会溢出，溢出会导致与最小值不能比较出大于0还是小于0 ，如4bit 1-（-7）
  // 溢出的情况只有在符号不同的情况下出现，若是 符号不同，则直接与最小值与，为0的是正数，为大
  // 加上等于判断
  int min = 1 << 31;
  //signX和signY只可能是1000··，或者是00···；
  int signX = x & min;
  int signY = y & min;
  // sameSign =0,x与y符号相同
  int sameSign = signX^signY;
  int bool = !!sameSign;
  int negateBoolValue = ~bool+1;
  int sameSignedLess = !!((x+(~y+1))&min);
  int equal = !(x^y);
  // sameSign?signX>>31：sameSignedLess
  return (!!(negateBoolValue&(signX>>31)) | ((~negateBoolValue) & sameSignedLess))|equal;
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
  //(~x+1)>>32 寄存器每个操作计算完都被截取了尾数，不会保留32位
  //要研究0到1的多种过程中，其他数字相同的流程是到0
  //0的特性是正负数都为0···，还有最小值的相反数也是自己
  //同样右移31位，算术右移补充的是1,0补充的是0
  //0右移31位的数字+1 = 1，而1···+1 = 0；
  return ((x|(~x+1))>>31)+1;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
  /*判断最低需要多少位有一个隐藏的特性：判断最高位（除了符号位）最高位的1在哪个位置
    所以最好是右移判断，右移剩的值bool为1，则+1
    注意：-1可以用signed 1表示
    但是右移会涉及算术运算符，需要确保负数右移确定出来的位数一样，即只要~*/
  int negateBoolValue =  ~!!(x & 1<<31)+1;
  int operateX = (negateBoolValue&(~x)) | ((~negateBoolValue) & (x));
  // 最简单可以一位一位移动，用二分法ops更少

  //如果高位16有1，则低位16是必须的，<<4表示需要低16位
  int bit16 = (!!(operateX>>16))<<4;
  // 如果bit 16为1，x 右移16位，如果bit 16 为0，则保留低16位 
  operateX = operateX>>bit16;


  //剩下的值高位8如果有1，则低位8是必须的
  int bit8 = (!!(operateX>>8))<<3;
  operateX = operateX>>bit8;

  //剩下的值高位4如果有1，则低位4是必须的
  int bit4 = (!!(operateX>>4))<<2;
  operateX = operateX>>bit4;

  //剩下的值高位2如果有1，则低位2是必须的
  int bit2 = (!!(operateX>>2))<<1;
  operateX = operateX>>bit2;

  //剩下的值高位1如果有1，则低位1是必须的
  int bit1 = (!!(operateX>>1))<<0;
  operateX = operateX>>bit1;
  
  //符号位始终为0，所以需要判断最后一位是否为1
  int bit0 = !!(operateX);
  //最后得加上必须的符号位
  return bit16+bit8+bit4+bit2+bit1+bit0+1;
}
//float
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
  
  unsigned frac = uf & 0x007FFFFF;
  //Nan
  unsigned first12 = uf >> 23 << 3;
  if( first12 == 0xFF8 || first12 == 0x7F8){
    return uf;  
  }
  //非规格化
  else if(first12 == 0 || first12 == 0x800){
    return first12 << 20 |((frac) << 1);
  }else{
  //规格化
    unsigned plusOne = ((first12 >> 3)+1) << 3;
    //x*2越界情况处理
    if(plusOne == 0xFF8 || plusOne == 0x7F8){
      return plusOne << 20;
    }
    return plusOne << 20 | frac;
  }
  
  }
/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
  // 分解
    unsigned s = uf & (1 << 31);
    int exp = (uf & 0x7f800000) >> 23;
    unsigned frac = uf & (~0xff800000);

  //如果大于2^32则返回 0x80000000u
  int E = exp-127;
  if(exp >= 255 || E > 31){
    return 0x80000000u;
  }
  if(E < 0) return 0;
  //只会存在规格化数
  // 1+frac 
  unsigned M = frac | (1 << 23); 
  //v = (-1)^s * M * 2^E;  M为2^(-23) * int(M) 
  //因此v = (-1)^s *int(M)* 2^(-23+E)
  // -23+E > 0 左移， -23+E < 0 右移 ，书2.3.6 如果除以2的幂，得是无符号才能直接用>>
  unsigned V = E>23? M <<(E-23):M >>(23-E);
  //上面判断保证了不会溢出
  return s?(~V+1):V;
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {
  if(x > 127) return 0x7F800000;
  //可以用规格化表示时，即f = 0， M = 1，e = E+bias , V = M * 2.0^x
  if(x>=-126) return (x+127) << 23;
  //当E = 0 时，0到2^-126之间，还可以用frac 表示 2^-1 *2.0^(-126) 到2^-23 * 2.0^(-126);
  //If the result is too small to be represented as a denorm, return
  // if(x > -150) return 1 << x+149 ;
  return 0;
}
