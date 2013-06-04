#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mio.h"

/* һ�����1024���ַ� */
#define MAX_INPUT_LEN	1024

/* ��֧��20��������ÿ���������Ȳ��ܳ���50 */
#define MAX_ARG_COUNT	20
#define MAX_ARG_LEN		50

char mio_arg[MAX_ARG_COUNT][MAX_ARG_LEN+1];
char mio_tp = '\0';
int mio_argc = 0;

void MioGetArg()
{
	char c;
	char str[MAX_INPUT_LEN+1];
	int i = 0;
	

	char flag = 0;
	int x,y;

	while(1)
	{
		c = getchar();

		if ( c!='\n' )
		{
			str[i++] = c;
			if ( i == MAX_INPUT_LEN )
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

	str[i] = '\0';


	x = 0;
	y = 0;

	i = 0;

	/* ȥ�����׵Ŀո��tab */
	while ( str[i]==' ' || str[i]=='\t' )
	{
		i++;
	}

	/* ����ÿ���������ַ�������arg�� */
	while(1)
	{
		if ( str[i]!=' ' && str[i]!='\t' && str[i]!='\0' )
		{
			/* ֻ��С����󳤶ȵ��ַ��ż�¼��������� */
			if (y<MAX_ARG_LEN)
			{
				mio_arg[x][y] = str[i];
				y++;
			}
			flag = 1;
		}
		else
		{
			if ( flag == 1 )
			{
				/* �ѽ�����һ�������������ַ��� */
				mio_arg[x][y] = '\0';

				x++;
				/* �������֧�ֵĲ������ȣ�����Ĳ������� */
				if ( x>=MAX_ARG_COUNT )
				{
					break;
				}
				flag = 0;
				y = 0;
			}
		}

		if (str[i]=='\0')
		{
			break;
		}
		
		i++;
	}

	mio_argc = x;
}

int MioGetArgCount()
{
	return mio_argc;
}

char* MioGetArgByIndex(int index)
{
	if ( index<mio_argc )
	{
		return mio_arg[index];
	}
	else
	{
		return &mio_tp;
	}
}

int MioGetArg2Num(int index, int *num)
{
	char *p;

	if ( index>=mio_argc )
	{
		return 1;
	}
	
	p = mio_arg[index];

	while( *p!='\0' )
	{
		if ( ( *p<'0' || *p>'9' ) && *p!='-'  && *p!='+'  && *p!='.' )
		{
			return 2;
		}
		p++;
	}

	*num = atoi(mio_arg[index]);

	return 0;
}