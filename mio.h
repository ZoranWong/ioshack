#ifndef _MIO_H_
#define _MIO_H_

/*
 * 等待输入一行命令，并解析参数
 */
void MioGetArg();

/*
 * 返回参数总个数
 */
int MioGetArgCount();

/*
 * 获得第index个参数，返回其参数字符串的指针
 * 如果没有则返回一个指向空字符串的指针
 */
char* MioGetArgByIndex(int index);

/*
 * 检查参数是否为数字，如果是则将字符串转化为数字并存到num中
 * 成功返回0，失败返回其他值
 */
int MioGetArg2Num(int index, int *num);

#endif