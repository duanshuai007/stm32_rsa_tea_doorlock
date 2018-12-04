#ifndef _RSA_16_H_
#define _RSA_16_H_

#include <stdio.h>
#include "user_config.h"

//#define MAX_NUM 63001
#define MAX_NUM     5000
#define MAX_PRIME   251

#define OK 100
#define ERROR_NOEACHPRIME 101
#define ERROR_NOPUBLICKEY 102
#define ERROR_GENERROR 103

/*
*      duanshuai@ubuntu:~/aaa$ ./r -k 179 251
*      正在生成私钥d集合......
*      共有17599组密钥对！
*      请输入您想使用的密钥对的组数：(从1开始到17599)15935
*      d = 40291, e = 1311
*      PublicKey(e,n): (1311,44929)
*      PrivateKey(d,n): (40291,44929)
*/
#define USER_RSA_P    179
#define USER_RSA_Q    251
#define USER_RSA_E    1311
#define USER_RSA_D    40291
#define USER_RSA_N    44929

//!　保存私钥d集合
typedef struct {
		unsigned int set[ MAX_NUM ];
		unsigned int size;
}pKeyset;

//! 保存公、私钥对
typedef struct {
		unsigned int d;
		unsigned int e;
		unsigned int n;
}pPairkey ;


#if 0
// 名称：IsPrimeNumber
// 功能：判断素数
//  参数：num: 输入整数
// 返回：素数返回true，否则返回false;
bool IsPrimeNumber( unsigned int num );
unsigned int MakePrivatedKeyd( unsigned int uiP, unsigned int uiQ );
unsigned int GetPrivateKeyd( unsigned int iWhich );
unsigned int MakePairkey( unsigned int uiP, unsigned int uiQ, unsigned int uiD );
unsigned int GetPairKey( unsigned int *d, unsigned int *e );
void rsa_encrypt( int n, int e, char *mw, int iLength, int *cw );
#endif

void rsa_decrypt( int n, int d, int *cw, int cLength, char *mw );
void RSA_Decrypt(int *cw, int len, char *mw);

void setRSAKey(int n, int e, int d);

void outputkey();

#endif
