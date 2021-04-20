#include <stdio.h>
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

char *COMMON_TEA_PASSWORD = "WAMINGTECHNOLOGY";

int main( int argc, char **argv )
{
	uint8_t mingwen[] = "hello world i am duanshuai!\r\n";

	printf("明文:%s", mingwen);

	int size = 0;
	int len = strlen(mingwen);
	if (len % 8 != 0) {
		size = ((len / 8) + 1) * 8;
	} else {
		size = len;
	}

	//原文长度如果不是8的倍数，那么就新建一个数组使其凑足8的倍数
	//原文的字符串数组
	uint8_t *jiami_buffer = (uint8_t *)malloc(size * sizeof(uint8_t));
	//加密后的数组
	uint8_t *after = (uint8_t *)malloc(size * sizeof(uint8_t));

	memset(jiami_buffer, 0, size);
	memset(after, 0, size);
	memcpy(jiami_buffer, mingwen, strlen(mingwen));
	for (int i=0; i < size; i++) {
		printf("%02x ", jiami_buffer[i]);
	}
	printf("\r\n");
	
	//进行加密操作
	for (int i = 0; i < size; i+= 8)
	{
		TEA_Encipher((uint32_t *)&after[i], (uint32_t *)&jiami_buffer[i], COMMON_TEA_PASSWORD);
	}
	printf("加密后得到的密文\r\n");
	for(int i = 0; i < size; i++) {
		printf("%02x ", after[i]);
	}
	printf("\r\n");


	//进行解密
	for (int i = 0; i < strlen(after); i+= 8)
	{
		TEA_Decrypt((char *)&after[i], COMMON_TEA_PASSWORD);
	}

	printf("解密后得到的明文:%s", after);
	printf("对应的十六进制数组:");
	for(int i = 0; i < size; i++) {
		printf("%02x ", after[i]);
	}
	printf("\r\n");

	return 0;
}

