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
 * @description: λ��ת
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
 * @description: ����CRC_INFO_t crcInfo ��ʼ��crcԴ��
 * @param {unsigned char} crcInfoIndex :crc_struct�����
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

    //����LSB����
    if (crcMode.refin)
    {
        //poly �Լ�init ��Ҫ������, crcTable �� init = 0;
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

    //����MSB����
    else
    {
        //���λ��С��8��polyҪ���Ƶ����λ
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

            //���width < 8����ôʵ���ϣ�crc���ڸ�widthλ�ģ���Ҫ���� 8 - width
            //����Ϊ�˷��������򣨻���Ҫ��λ�����λ��*ptr��bit7���룩�����Բ�����
            // if (crcMode.width < 8)
            //	  value >>=  8 - crcMode.width;
            // crcTable[i] = value & (2 << (crcMode.width - 1)) - 1);
            crcTable[i] = value & (crcMode.width < 8 ? 0xff : valid_bits);
        }
    }
}

/**
 * @description: ��ȡCRC��У��ֵ
 * @param {unsigned char} *ptr ��������ָ��
 * @param {unsigned int} len ���ݳ���
 * @return {*} width_t У��ֵ
 */
width_t crc_compute(unsigned char *ptr, unsigned int len)
{
    width_t value;

    //���� LSB ����
    if (crcMode.refin)
    {
        value = crc_reflected(crcMode.init, crcMode.width);

        // Ϊ�˼�����λ�Ȳ�����width����8��С��8�ķֿ�����
        //��width <= 8ʱ��
        // value = (value >> 8) ^ crcTable[value & 0xff ^ *ptr++];
        //�ɼ�Ϊ
        // value = crcTable[value ^ *ptr++];

        while (len--)
        {
            value = (value >> 8) ^ crcTable[(value & 0xff) ^ *ptr++];
        }
    }

    //���� MSB ����
    else
    {
        //Ϊ�˼�����λ�Ȳ�����width����8��С��8�ķֿ�����
        //��width <= 8ʱ��
        // high = value >> (crcMode.width - 8);
        // value = (value << 8) ^ crcTable[high ^ *ptr++];
        //�ɼ�Ϊ
        // value = crcTable[value ^ *ptr++];

        unsigned char high;
        value = crcMode.init;

        while (len--)
        {
            high = value >> (crcMode.width - 8);
            value = (value << 8) ^ crcTable[high ^ *ptr++];
        }
    }

    //�������
    if (crcMode.refout != crcMode.refin)
        value = crc_reflected(value, crcMode.width);

    //������
    value ^= crcMode.xorout;

    return value & ((2 << (crcMode.width - 1)) - 1); //&0xFFFF
}

/**
 * @description: ��ӡtable��Ϣ
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