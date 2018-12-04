#include <stdio.h>
//#define MAX_NUM           63001
#define MAX_NUM             1000000
#define MAX_PRIME           1009

#define OK                  100
#define ERROR_NOEACHPRIME   101
#define ERROR_NOPUBLICKEY   102
#define ERROR_GENERROR      103

struct pKeyset
{
		unsigned int set[ MAX_NUM ];
		unsigned int size;
}pset;

typedef struct
{
		unsigned int d;
		unsigned int e;
		unsigned int n;
}pPairkey;

pPairkey pairkey;

unsigned int MakePrivatedKeyd( unsigned int uiP, unsigned int uiQ );
unsigned int GetPrivateKeyd( unsigned int iWhich );
//unsigned int MakePairkey( unsigned int uiP, unsigned int uiQ, unsigned int uiD );
pPairkey *MakePairkey( unsigned int uiP, unsigned int uiQ, unsigned int uiD);

unsigned int GetPairKey( unsigned int *d, unsigned int *e );
void rsa_encrypt( int n, int e, char *mw, int iLength, int *cw );
void rsa_decrypt( int n, int d, int *cw, int cLength, char *mw );
void outputkey();

