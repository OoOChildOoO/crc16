/*
 * @Author: xph
 * @Date: 2021-08-08 13:28:14
 * @LastEditTime: 2021-08-08 15:09:23
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \CRC\crc16.h
 */
#ifndef __CRC16_H
#define __CRC16_H
#include <stdio.h>
#include <string.h>
typedef unsigned short width_t;
typedef struct
{
    unsigned char width;  //长度-位
    width_t poly;         //多项式
    width_t init;         //初始值
    unsigned char refin;  //逆序输入标志位
    unsigned char refout; //逆序输出标志位
    width_t xorout;       //结果异或值 out ^= xorout
} CRC_INFO_t;

#define WIDTH (8 * sizeof(width_t)) //长度 16位
#define TOPBIT (1 << (WIDTH - 1))   //首位
#define BIT(x) (1u << (x))

#define CRC16_IBM 0         //多项式x16+x15+x2+1（0x8005），初始值0x0000，低位在前，高位在后，结果与0x0000异或
#define CRC16_MAXIM 1       //多项式x16+x15+x2+1（0x8005），初始值0x0000，低位在前，高位在后，结果与0xFFFF异或
#define CRC16_USB 2         //多项式x16+x15+x2+1（0x8005），初始值0xFFFF，低位在前，高位在后，结果与0xFFFF异或
#define CRC16_MODBUS 3      //多项式x16+x15+x2+1（0x8005），初始值0xFFFF，低位在前，高位在后，结果与0x0000异或
#define CRC16_CCITT 4       //多项式x16+x12+x5+1（0x1021），初始值0x0000，低位在前，高位在后，结果与0x0000异或
#define CRC16_CCITT_FALSE 5 //多项式x16+x12+x5+1（0x1021），初始值0xFFFF，低位在后，高位在前，结果与0x0000异或
#define CRC16_X25 6         //多项式x16+x12+x5+1（0x1021），初始值0xFFFF，低位在前，高位在后，结果与0xFFFF异或
#define CRC16_XMODEM 7      //多项式x16+x12+x5+1（0x1021），初始值0x0000，低位在后，高位在前，结果与0x0000异或

extern void crc_table_init(unsigned char crcInfoIndex);
extern width_t crc_compute(unsigned char *ptr, unsigned int len);

#endif // !__CRC16_H
