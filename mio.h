#ifndef _MIO_H_
#define _MIO_H_

/*
 * �ȴ�����һ���������������
 */
void MioGetArg();

/*
 * ���ز����ܸ���
 */
int MioGetArgCount();

/*
 * ��õ�index������������������ַ�����ָ��
 * ���û���򷵻�һ��ָ����ַ�����ָ��
 */
char* MioGetArgByIndex(int index);

/*
 * �������Ƿ�Ϊ���֣���������ַ���ת��Ϊ���ֲ��浽num��
 * �ɹ�����0��ʧ�ܷ�������ֵ
 */
int MioGetArg2Num(int index, int *num);

#endif