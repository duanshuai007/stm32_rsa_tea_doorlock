/*
   	RSA加密、解密程序
	版本：1.1
	作者：童虎
	QQ：78814701
*/
#include <stdio.h>
//#include <afxwin.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tea.h"

#include "time.h"

#include "rsa.h"

#define DECRYPT_FILE "rsaencrypt.txt"
#define ENCRYPT_FILE "rsadecrypt.txt"
//! 约束文件最大2M
#define MAX_FILE 1024*1024*2

static unsigned int cnt; 

// 名称：usage
// 功能：帮助信息
//  参数：应用程序名称
// 返回：提示信息
void Usage( const char *appname )
{
	printf( "正确格式如下：\n" );
	printf( "生成密钥对：r -k p q num id\n");
    printf("\t生成以质数p q对的一个密钥集合，num表示生成的数量，id表示设备的起始地址\n\t总是从小到大开始生成.\n");
    printf("执行加密：r -e originfile e n\n执行解密：r -d RSA加密密文.txt d n\n" );
}

// 名称：IsNumber
// 功能：判断数字字符数组
//  参数：strNumber:字符数组
// 返回：数字字组数组返回true，否则返回false;
bool IsNumber( const char *strNumber )
{
	unsigned int i;

	if( !strNumber )
		return false;

	for ( i = 0 ; i < strlen(strNumber) ; i++ )
	{
		if ( strNumber[i] < '0' || strNumber[i] > '9' )
			return false;
	}

	return true;
}


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


// 名称：FileIn
// 功能：读取磁盘文件到内存
//  参数：strFile:文件名称；inBuff:指向文件内容缓冲区
// 返回：实际读取内容大小(字节)
int FileIn( const char *strFile, unsigned char *inBuff )
{
	int iFileLen=0, iBuffLen=0;

	//! 打开密文文件
	FILE *fp = fopen( strFile, "r+");
	if(fp == NULL)
	{
		printf( "%s：FileIn无法打开文件！\n", strFile );
		goto out;
	}

	fseek( fp, 0, SEEK_END );
	iFileLen = ftell(fp);
	fseek( fp , 0 , SEEK_SET );
	if( iFileLen > MAX_FILE )
	{
		printf( "文件长度不能大于 %dM,!\n", MAX_FILE/(1024*1024) );
		goto out;
	}
	iBuffLen = iFileLen;

	//if( !inBuff )
	//		goto out;

	memset( inBuff, 0, sizeof( inBuff ) );
	fread( inBuff,  iFileLen, sizeof(char), fp );
	printf("明文内容：%s\n", inBuff);
	//file.Read( inBuff, iFileLen );
	//file.Close();
	fclose( fp );
out:
	return iBuffLen;
}


// 名称：FileOut
// 功能：加/解密结果输出到当前目录磁盘文件中
//  参数：strOut指向输出字符缓冲区，输出大小len，strFile为输出文件
// 返回：无
void FileOut( const void *strOut, int len, const char *strFile )
{
	//! 输出到文件
	//CFile outfile( strFile , CFile::modeCreate | CFile::modeWrite );
	FILE *fp = fopen( strFile , "w+");
	if(fp == NULL)
	{
		printf( "%s：FileOut无法打开文件！\n", strFile );
		exit(1);
	}
	fwrite( strOut, len, sizeof(char), fp );
	fclose(fp);
}


// 名称：CheckParse
// 功能：校验应用程序入口参数
// 参数：argc等于main主函数argc参数，argv指向main主函数argv参数
// 返回：若参数合法返回true，否则返回false
// 备注：简单的入口参数校验
bool CheckParse( int argc, char** argv )
{
	bool bRes = false;

	//if( argc != 4 && argc != 5 )
	if( argc != 5 )
		goto out;

	if( argv[1][1] == 'k' )
	{
		//! 生成公、私钥对
		if( !IsNumber( argv[2] ) || 
				!IsNumber( argv[3] ) ||
				atoi( argv[2] ) > MAX_PRIME ||
				atoi( argv[3] ) > MAX_PRIME )
			goto out;
	}
	else if( (argc == 5) && (argv[1][1] == 'e' || argv[1][1] == 'd') )
	{
		//! 加密、解密操作
		if( !IsNumber( argv[3] ) ||
				!IsNumber( argv[4] ) ||
				atoi( argv[3] ) > MAX_NUM ||
				atoi( argv[4] ) > MAX_NUM )
			goto out;
	}
	else
		Usage(*argv);
	bRes = true;


out:
	return bRes;
}


// 名称：kOption1
// 功能：程序k选项操作：由素数P、Q生成私钥d集合
// 参数：uiP: 程序入口参数P; uiQ: 程序入口参数Q
// 返回：执行正确返回生成私钥数目，否则返回0
unsigned int kOption1( unsigned int uiP, unsigned int uiQ , unsigned int num, unsigned int id)
{
    unsigned int uiRes = 0;

    if( !IsPrimeNumber( uiP ) ) {
        printf( "P输入错误，P必须为(0, %d]素数\n", MAX_PRIME );
        return uiRes;
    }
    
    if( !IsPrimeNumber( uiQ ) ) {
        printf( "Q输入错误，Q必须为(0, %d]素数\n", MAX_PRIME );
        return uiRes;
    }
    
    if( id > 0xffff ) {
        printf("device id must be < 0xffff\r\n");
        return uiRes;
    }

    if ( id + num > 0xffff) {
        printf("没有足够的设备号可以分配，请重新配置id和/或设备数量\r\n");
        return uiRes;
    }

    if( uiP == uiQ ) {
        printf( "素数P与素数Q相同，很容易根据公钥n开平方得出素数P和Q，这种加密不安全，请更换素数!\n" );
        return uiRes;
    }

    printf( "正在生成私钥d集合......\n" );
    uiRes = MakePrivatedKeyd( uiP, uiQ );

    //修改代码，愿功能是打印出第d-1个密钥
    //修改后是输出1-d的密钥到key.txt文件中
    int fd = open("key.txt", O_RDWR | O_CREAT,
            S_IRUSR|S_IWUSR|S_IXUSR
            |S_IRGRP|S_IWGRP|S_IXGRP
            |S_IROTH|S_IWOTH|S_IXOTH);
    if(fd < 0) {
        printf("open key.txt filed\r\n");
        return -1;
    }

    int fd_plain = open("key_plain.txt", O_RDWR | O_CREAT,
            S_IRUSR|S_IWUSR|S_IXUSR
            |S_IRGRP|S_IWGRP|S_IXGRP
            |S_IROTH|S_IWOTH|S_IXOTH);
    if ( fd_plain < 0 ) {
        printf("open key_plain.txt failed\r\n");
        return -1;
    }

    uint8_t buff[17] = {0};
    uint8_t crypt[17] = {0};
    uint8_t savemsg[64] = {0};
    uint8_t random_pw[16] = {0};
    int i, j, random_num, write_len;
    pPairkey *pp;

    for(i=0;i<num;i++) {
        
        pp = MakePairkey( uiP, uiQ, i );
        
		outputkey();

        memset(buff, 0, sizeof(buff));
        memset(crypt, 0, sizeof(crypt));
        memset(savemsg, 0, sizeof(savemsg));
        memset(random_pw, 0, sizeof(random_pw));

        //将密钥补转换为字符串，然后补全位数，要求是32位的整数
        //buff[0-3]为一个32位数
        sprintf(buff, "%04x%04x%04x%04x\n", pp->d, pp->e, pp->n, id + i);
        printf("miyao:%s\r\n", buff);

        //保存明文信息
        write(fd_plain, buff, strlen(buff));

        //生成随机数，并转换为十六进制字符串
        srand((int)time(0) + i);
        random_num = rand();
        sprintf(random_pw, "%08x", random_num);
        printf("random number:%s\r\n", random_pw);

        //使用随机密码对密钥进行加密
        //printf("jiami:%s\r\n", buff);
        TEA_Encipher((uint32_t *)crypt,     (uint32_t *)buff,       (uint32_t *)random_pw);
        TEA_Encipher((uint32_t *)(crypt+8), (uint32_t *)(buff+8),   (uint32_t *)random_pw);
        //TEA_Decrypt((uint32_t *)crypt, (uint32_t *)random_pw);
        //TEA_Decrypt((uint32_t *)(crypt+8), (uint32_t *)random_pw);
        //printf("jiemi:%s\r\n", crypt);
        for(j=0;j<16;j++)
            printf("%02x ", crypt[j]);
        printf("\r\n");

        //使用通用密码对随机密码进行加密
        TEA_Encipher((uint32_t *)random_pw, (uint32_t *)random_pw, (uint32_t *)COMMON_TEA_PASSWORD);
        printf("jiami pw:%s\r\n", random_pw);
        for(j=0;j<8;j++)
            printf("%02x ", random_pw[j]);
        printf("\r\n");
        //TEA_Decrypt((uint32_t *)random_pw, (uint32_t *)COMMON_TEA_PASSWORD);
        //printf("jiemi pw:%s\r\n", random_pw);

        //构造信息格式
        //信息头0xaabbcddc
        savemsg[0] = 'F';
        savemsg[1] = 'R';
        savemsg[2] = 'O';
        savemsg[3] = 'G';
        //然后是加密后的随机密码
        //sprintf(savemsg + 4, "+%s+", random_pw);
        for(j=0;j<8;j++) {
            sprintf(savemsg + 4 + (j*2), "%02x", random_pw[j]);
        }
        //然后是使用随机密码进行加密的密钥
        for(j=0;j<16;j++) {
           sprintf(savemsg + 16 + 4 + (j*2), "%02x", crypt[j]);
        }
        savemsg[32+20] = '\n';
        savemsg[33+20] = '\r';

        //保存到文件内
        printf("save:%s\r\n", savemsg);
        write(fd, savemsg, strlen(savemsg));
    }

    close(fd_plain);
    close(fd);

    return uiRes;
}

int main( int argc, char **argv )
{
    unsigned int p , q , d , n , e;//two prime p & q, public key(n, e) , private key(n , d)
    unsigned int num;
    CheckParse(argc,  argv );
    cnt = 0;
    d = 0;//uid
    //printf("argv[0]=%s\r\n", argv[0]);
    //printf("argv[1]=%s\r\n", argv[1]);
    //printf("argv[2]=%s\r\n", argv[2]);
    //printf("argv[3]=%s\r\n", argv[3]);
    //printf("argv[4]=%s\r\n", argv[4]);

    if (argc > 2) {
        if ( 0 == strcmp(argv[1], "-k")) 
        {
            printf("argc=%d\r\n", argc);
            if( argc < 6) {
                printf("参数错误\r\n");
                return -1;
            }

            int id_start;
            char op;
            char fileName[80];
            unsigned char inBuffer[1024]; //输入缓冲区
            int len = 0;
            int *cw = (int *)malloc(MAX_FILE);
            p = atoi( argv[2] );
            q = atoi( argv[3] );
            num = atoi( argv[4]);
            id_start = atoi( argv[5]); 

            if( (cnt = kOption1(p, q, num, id_start)) > 0 )
            {
                printf("Now exiting...\n");
                sleep(1);
                exit(1);
            }

            printf("共有%d组密钥对！\n", cnt);
            printf("请输入您想使用的密钥对的组数：(从1开始到%d)", cnt);
            scanf("%d", &d);
            if(d < 1 || d > cnt)
            {
                printf( "不存在的密钥对！\n" );
                exit(1);
            }

            //MakePrivatedKeyd(p, q);
            MakePairkey( p, q, d - 1 );

            outputkey();
            printf("现在是否使用该组密钥对的公钥进行加密操作？\n输入Y或者y确认，输入其它不执行:");
            scanf( " %c", &op );
            if('y' == op || 'Y' == op)
            {
                memset(inBuffer, 0, sizeof(inBuffer));
                memset(fileName, 0, sizeof(fileName));
                printf( "请输入需要加密字符串的来源文件名：" );
                scanf("%s", fileName);
                len = FileIn( fileName , inBuffer );
                rsa_encrypt( pairkey.n, pairkey.e, (char *)inBuffer, len, cw );
                FileOut( cw, 4*len, DECRYPT_FILE );
            }
            else
            {
                printf("Now exiting...");
                sleep(1);
                exit(1);
            }
        }/*end of if(ret > 0)*/
        else if ((0 == strcmp(argv[1], "-e")) || (0 == strcmp(argv[1], "-d")))
        {
            char FileName[80];
            strcpy( FileName, argv[2] );
            int len;

            printf( "您选择直接了执行加解密，请确认您提供了正确可用的密钥对，否则加解密的结果将不可知！\n" );

            if(argv[1][1] == 'e' )
            {
                unsigned char inBuffer[1024]; //输入缓冲区
                int *cw = (int *)malloc(MAX_FILE);
                memset(inBuffer, 0, sizeof(inBuffer));
                len = FileIn( FileName , inBuffer );
                e = atoi(argv[3]);
                n = atoi(argv[4]);
                rsa_encrypt( n, e, (char *)inBuffer, len, cw );
                FileOut( cw, 4*len, DECRYPT_FILE );
            }
            else if(argv[1][1] == 'd')
            {
                char Buffer[1024]; //输入缓冲区
                int *cw = (int *)malloc(MAX_FILE);
                memset(Buffer, 0, sizeof(Buffer));
                len = FileIn(FileName, (unsigned char *)cw);
                printf("file len=%d\r\n", len);
                d = atoi(argv[3]);
                n = atoi(argv[4]);
                rsa_decrypt( n, d, cw, len, Buffer );
				printf("解密结果:%s\r\n", Buffer);
                //FileOut( Buffer, len/4, ENCRYPT_FILE );
            }
            else
            {
                printf("未知选项\n");
                exit(1);
            }
        } 
    } else {
        Usage(argv[0]);
    }

    return 0;
}

