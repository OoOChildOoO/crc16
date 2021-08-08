/*
 * @Author: xph
 * @Date: 2021-08-08 13:28:08
 * @LastEditTime: 2021-08-08 15:08:19
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings EditWW
 * @FilePath: \CRC\crc16.c
 */
#include "crc16.h"

static CRC_INFO_t crcMode;
static width_t crcTable[256];
static const CRC_INFO_t crcInfo[] = {
    //crc16
    [CRC16_IBM] = {16, 0x8005, 0x0000, 1, 1, 0x0000},         //CRC16_IBM
    [CRC16_MAXIM] = {16, 0x8005, 0x0000, 1, 1, 0xffff},       //CRC16_MAXIN
    [CRC16_USB] = {16, 0x8005, 0xffff, 1, 1, 0xffff},         //CRC16_USB
    [CRC16_MODBUS] = {16, 0x8005, 0xffff, 1, 1, 0x0000},      //CRC16_MODBUS
    [CRC16_CCITT] = {16, 0x1021, 0x0000, 1, 1, 0x0000},       //CRC16_CCITT
    [CRC16_CCITT_FALSE] = {16, 0x1021, 0xffff, 0, 0, 0x0000}, //CRC16_CCITT_FALSE
    [CRC16_X25] = {16, 0x1021, 0xffff, 1, 1, 0xffff},         //CRC16_X25
    [CRC16_XMODEM] = {16, 0x1021, 0x0000, 0, 0, 0x0000},      //CRC16_XMODEM
};

/**
 * @description: 位逆转
 * @param {width_t} input
 * @param {unsigned char} bits
 * @return {*}
 */
static width_t crc_reflected(width_t input, unsigned char bits)
{
    width_t var = 0;
    while (bits--)
    {
        var <<= 1;
        if (input & 0x01)
            var |= 1;
        input >>= 1;
    }
    return var;
}
/**
 * @description: 根据CRC_INFO_t crcInfo 初始化crc源表
 * @param {unsigned char} crcInfoIndex :crc_struct表序号
 * @return {*}
 */
void crc_table_init(unsigned char crcInfoIndex)
{
    unsigned short i;
    unsigned char j;
    width_t poly, value;
    width_t valid_bits = (2 << (crcMode.width - 1)) - 1;

    crcMode.width = crcInfo[crcInfoIndex].width;
    crcMode.poly = crcInfo[crcInfoIndex].poly;
    crcMode.init = crcInfo[crcInfoIndex].init;
    crcMode.refin = crcInfo[crcInfoIndex].refin;
    crcMode.refout = crcInfo[crcInfoIndex].refout;
    crcMode.xorout = crcInfo[crcInfoIndex].xorout;

    //逆序LSB输入
    if (crcMode.refin)
    {
        //poly 以及init 都要先逆序, crcTable 的 init = 0;
        poly = crc_reflected(crcMode.poly, crcMode.width);

        for (i = 0; i < 256; i++)
        {
            value = i;
            for (j = 0; j < 8; j++)
            {
                if (value & 1)
                    value = (value >> 1) ^ poly;
                else
                    value = (value >> 1);
            }
            crcTable[i] = value & valid_bits;
        }
    }

    //正序MSB输入
    else
    {
        //如果位数小于8，poly要左移到最高位
        poly = crcMode.width < 8 ? crcMode.poly << (8 - crcMode.width) : crcMode.poly;
        width_t bit = crcMode.width > 8 ? BIT(crcMode.width - 1) : 0x80;

        for (i = 0; i < 256; i++)
        {
            value = crcMode.width > 8 ? i << (crcMode.width - 8) : i;

            for (j = 0; j < 8; j++)
            {
                if (value & bit)
                    value = (value << 1) ^ poly;
                else
                    value = (value << 1);
            }

            //如果width < 8，那么实际上，crc是在高width位的，需要右移 8 - width
            //但是为了方便后续异或（还是要移位到最高位与*ptr的bit7对齐），所以不处理
            // if (crcMode.width < 8)
            //	  value >>=  8 - crcMode.width;
            // crcTable[i] = value & (2 << (crcMode.width - 1)) - 1);
            crcTable[i] = value & (crcMode.width < 8 ? 0xff : valid_bits);
        }
    }
}

/**
 * @description: 获取CRC的校验值
 * @param {unsigned char} *ptr 输入数据指针
 * @param {unsigned int} len 数据长度
 * @return {*} width_t 校验值
 */
width_t crc_compute(unsigned char *ptr, unsigned int len)
{
    width_t value;

    //逆序 LSB 输入
    if (crcMode.refin)
    {
        value = crc_reflected(crcMode.init, crcMode.width);

        // 为了减少移位等操作，width大于8和小于8的分开处理
        //当width <= 8时，
        // value = (value >> 8) ^ crcTable[value & 0xff ^ *ptr++];
        //可简化为
        // value = crcTable[value ^ *ptr++];

        while (len--)
        {
            value = (value >> 8) ^ crcTable[(value & 0xff) ^ *ptr++];
        }
    }

    //正序 MSB 输入
    else
    {
        //为了减少移位等操作，width大于8和小于8的分开处理
        //当width <= 8时，
        // high = value >> (crcMode.width - 8);
        // value = (value << 8) ^ crcTable[high ^ *ptr++];
        //可简化为
        // value = crcTable[value ^ *ptr++];

        unsigned char high;
        value = crcMode.init;

        while (len--)
        {
            high = value >> (crcMode.width - 8);
            value = (value << 8) ^ crcTable[high ^ *ptr++];
        }
    }

    //逆序输出
    if (crcMode.refout != crcMode.refin)
        value = crc_reflected(value, crcMode.width);

    //异或输出
    value ^= crcMode.xorout;

    return value & ((2 << (crcMode.width - 1)) - 1); //&0xFFFF
}

/**
 * @description: 打印table信息
 * @param {*}
 * @return {*}
 */
static void printfTable()
{
    int i = 0;
    printf("width :%d poly:%d init:%d refin:%d refout:%d xorout:%d\n",
           crcMode.width, crcMode.poly, crcMode.init, crcMode.refin, crcMode.refout, crcMode.xorout);
    for (i = 0; i < 256; ++i)
    {
        if (i % 16 == 0)
            printf("\n");

        printf("0x%02X,", crcTable[i]);
    }
}

int main()
{
    const char *name[] = {
        //crc16
        "CRC16_IBM",
        "CRC16_MAXIN",
        "CRC16_USB",
        "CRC16_MODBUS",
        "CRC16_CCITT",
        "CRC16_CCITT_FALSE",
        "CRC16_X25",
        "CRC16_XMODEM",
    };

    unsigned int i;
    char *str = "HelloWorld!";
    unsigned int value;

    printf("input string is \"%s\"\n", str);

    for (i = 0; i < sizeof(crcInfo) / sizeof(crcInfo[0]); i++)
    {
        crc_table_init(i);

        printf("\n%s Table{", name[i]);
        printfTable();
        printf("\n};\n\n");

        value = crc_compute((unsigned char *)str, strlen(str));

        printf("%s(\"%s\") = 0x%x\n", name[i], str, value);
        printf("=======================================================\n");
    }
    getchar();
    return 0;
}