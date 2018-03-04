
#include "crc.h"

#define POLY 0xEDB88320L // CRC32生成多项式

static unsigned int crc_table[256];

static unsigned int get_sum_poly(unsigned char data)
{
    int j, hi;
    unsigned int sum_poly = data;
    for (j = 0; j<8; j++)
    {
        hi = sum_poly & 0x01; // 取得reg的最高位
        sum_poly >>= 1;
        if (hi) sum_poly = sum_poly ^ POLY;
    }
    return sum_poly;
}

void create_crc_table()
{
    int i;
    for (i = 0; i<256; i++)
    {
        crc_table[i] = get_sum_poly((unsigned char)(i & 0xFF));
    }
}

unsigned int CRC32_4(const unsigned char* data, unsigned int reg, int len)
{
    int i;
    //	unsigned int reg = 0; // 0xFFFFFFFF，见后面解释
    reg ^= 0xFFFFFFFF;
    for (i = 0; i<len; i++)
    {
        reg = (reg >> 8) ^ crc_table[(reg & 0xFF) ^ data[i]];
    }
    return reg ^ 0xFFFFFFFF;
}

/*
// 最终生成的校验表将是：
// {0x00000000,  0x77073096,  0xEE0E612C,  0x990951BA,
//  0x076DC419,  0x706AF48F,  0xE963A535,  0x9E6495A3,
//  ....}



// 字节逆转前的CRC32算法，字节序为b8,b7,...,b1

#define POLY 0x04C11DB7L // CRC32生成多项式

static unsigned int crc_table[256];

unsigned int get_sum_poly(unsigned char data)
{
int j, hi;
unsigned int sum_poly = data;

sum_poly <<= 24;
for(j=0; j<8; j++)
{
hi = sum_poly & 0x80000000; // 取得reg的最高位
sum_poly <<= 1;
if (hi) sum_poly = sum_poly ^ POLY;
}
return sum_poly;
}

void create_crc_table()
{
int i;
for(i=0; i<256; i++)
{
crc_table[i] = get_sum_poly((unsigned char)(i&0xFF));
}
}

unsigned int CRC32_4(const unsigned char* data, unsigned int reg, int len)
{
int i;
//	unsigned int reg = 0;// 0xFFFFFFFF
for(i=0; i<len; i++)
{
reg = (reg<<8) ^ crc_table[(reg>>24) & 0xFF ^ data[i]];
}
return reg;
}
*/