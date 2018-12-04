#include "rsa_16.h"
#include <stdio.h>
#include <string.h>
//#include <stdbool.h>
#include <math.h>
#include "user_config.h"

static pPairkey pairkey;

void setRSAKey(int n, int e, int d)
{
  pairkey.e = e;
  pairkey.n = n;
  pairkey.d = d;
#if 1
//  printf("e=%d,n=%d,d=%d\r\n",pairkey.e,pairkey.n,pairkey.d);
#endif
}

int getRSAKey_n(void)
{
  return pairkey.n;
}

int getRSAKey_e(void)
{
  return pairkey.e;
}

int getRSAKey_d(void)
{
  return pairkey.d;
}

static pKeyset pset;
// 名称：IsPrimeNumber
// 功能：判断素数
//  参数：num: 输入整数
// 返回：素数返回true，否则返回false;
bool IsPrimeNumber( unsigned int num )
{
  unsigned int i;
  if( num <= 1 )
    return false;

  unsigned int sqr = (unsigned int)sqrt((double)num);
  for( i = 2; i <= sqr; i++ )
  {
    if( num % i == 0 )
      return false;
  }
  
  return true;
}

// 名称：isPrime
// 功能：判断两个数是否互质
//  参数：m: 数a; n: 数b
// 返回：m、n互质返回true; 否则返回false
bool isPrime( unsigned int m, unsigned int n )
{
  unsigned int i=0;
  bool Flag = true;
  
  if( m<2 || n<2 )
    return false;
  
  unsigned int tem = ( m > n ) ? n : m;
  for( i=2; i<=tem && Flag; i++ )
  {
    bool mFlag = true;
    bool nFlag = true;
    if( m % i == 0 )
      mFlag = false;
    if( n % i == 0 )
      nFlag = false;
    if( !mFlag && !nFlag )
      Flag = false;
  }
  if( Flag )
    return true;
  else
    return false;
}

// 名称：MakePrivatedKeyd
// 功能：由素数Q、Q生成私钥d
//  参数：uiP: 素数P; uiQ: 素数Q
// 返回：私钥d
unsigned int MakePrivatedKeyd( unsigned int uiP, unsigned int uiQ )
{
  unsigned int i = 0;
  
  //! 得到所有与z互质的数( 私钥d的集合 )
  unsigned int z = ( uiP -1 ) * ( uiQ -1 );
  pset.size = 0;
  for( i = 0; i < z; i ++ )
  {
    if( isPrime( i, z ) )
    {
      pset.set[ pset.size++ ] = i;
    }
  }
  
  return pset.size;
}

// 名称：MakePairKey
// 功能：生成RSA公、私钥对
//  参数：uiP: 素数P; uiQ: 素数Q; uiD: 私钥d
// 返回：错误代码
unsigned int MakePairkey( unsigned int uiP, unsigned int uiQ, unsigned int uiD )
{
  bool bFlag = true;
  unsigned int i = 0, e;
  unsigned int z = ( uiP-1 ) * ( uiQ-1 );
  unsigned int d = pset.set[uiD];
  //d=uiD;
  
  if( !isPrime( z, d ) )
    return ERROR_NOEACHPRIME;
  
  for( i = 2;  i < z; i ++ )
  {
    if( (i*d)%z == 1 )
    {
      e = i;
      bFlag = false;
    }
  }
  if( bFlag )
    return ERROR_NOPUBLICKEY;
  
  if( (d*e)%z != 1 )
    return ERROR_GENERROR;
  
//  printf( "d = %d, e = %d\n", d, e );
  pairkey.d = d;
  pairkey.e = e;
  pairkey.n = uiP * uiQ;
  return OK;
}


// 名称：GetPairKey
// 功能：对外提供接口，获得公、私钥对
//  参数：uiP: 素数P; uiQ: 素数Q; uiD: 私钥d
// 返回：
unsigned int GetPairKey( unsigned int *d, unsigned int *e )
{
  *d = pairkey.d;
  *e = pairkey.e;
  return pairkey.n;
}

// 名称：GetPrivateKeyd
// 功能：对外提供接口，由用户选择ID得以私钥d
//  参数：iWhich: 用户选择私钥d的ID
// 返回：私钥d值
unsigned int GetPrivateKeyd( unsigned int iWhich )
{
  if( pset.size >= iWhich )
    return pset.set[ iWhich ];
  else
    return 0;
}

// 名称：rsa_encrypt
// 功能：RSA加密运算
//  参数：n: 公钥n; e: 公钥e; mw: 加密明文; iLength: 明文长度; cw: 密文输出
// 返回：无
void rsa_encrypt( int n, int e, char *mw, int mLength, int *cw )
{
  int i=0, j=0;
  int temInt = 0;
  
  for( i=0; i<mLength; i++ )
  {
    temInt = mw[i];
    if( e!=0 )
    {
      for( j=1; j<e; j++ )
      {
        temInt = ( temInt * mw[i] ) % n;
      }
    }
    else
    {
      temInt = 1;
    }
    
    cw[i] = (int)temInt;
  }
}

void RSA_Encrypt(char *mw, int mlen, int *cw)
{
  int n, e;
  n = getRSAKey_n();
  e = getRSAKey_e();
  rsa_encrypt(n , e, mw, mlen, cw);
}

// 名称：rsa_decrypt
// 功能：RSA解密运算
// 参数: 私钥n; d: 私钥d; cw: 密文; cLength: 密文长度; mw: 明文输出
// 返回：无
void rsa_decrypt( int n, int d, int *cw, int cLength, char *mw )
{
  int i = 0, j = -1;
  int temInt = 0;
  
  for( i=0; i<cLength/4; ++i )
  {
    mw[i] = 0;
    temInt = cw[i];
    
    if( d != 0 )
    {	
      for( j=1; j<d; j++ )
      {
        temInt = (int)( temInt * cw[i] ) % n;
      }
    }
    else
    {
      temInt = 1;
    }
    
    mw[i] = (char)temInt;
  }
}

void RSA_Decrypt(int *cw, int len, char *mw)
{
  int n, d;
  n = getRSAKey_n();
  d = getRSAKey_d();
  
  rsa_decrypt(n, d, cw, len, mw);
}

void outputkey()
{
//  printf("PublicKey(e,n): (%d,%d)\n",pairkey.e,pairkey.n);
//  printf("PrivateKey(d,n): (%d,%d)\n",pairkey.d,pairkey.n);
}

