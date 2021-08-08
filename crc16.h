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
    unsigned char width;  //����-λ
    width_t poly;         //����ʽ
    width_t init;         //��ʼֵ
    unsigned char refin;  //���������־λ
    unsigned char refout; //���������־λ
    width_t xorout;       //������ֵ out ^= xorout
} CRC_INFO_t;

#define WIDTH (8 * sizeof(width_t)) //���� 16λ
#define TOPBIT (1 << (WIDTH - 1))   //��λ
#define BIT(x) (1u << (x))

#define CRC16_IBM 0         //����ʽx16+x15+x2+1��0x8005������ʼֵ0x0000����λ��ǰ����λ�ں󣬽����0x0000���
#define CRC16_MAXIM 1       //����ʽx16+x15+x2+1��0x8005������ʼֵ0x0000����λ��ǰ����λ�ں󣬽����0xFFFF���
#define CRC16_USB 2         //����ʽx16+x15+x2+1��0x8005������ʼֵ0xFFFF����λ��ǰ����λ�ں󣬽����0xFFFF���
#define CRC16_MODBUS 3      //����ʽx16+x15+x2+1��0x8005������ʼֵ0xFFFF����λ��ǰ����λ�ں󣬽����0x0000���
#define CRC16_CCITT 4       //����ʽx16+x12+x5+1��0x1021������ʼֵ0x0000����λ��ǰ����λ�ں󣬽����0x0000���
#define CRC16_CCITT_FALSE 5 //����ʽx16+x12+x5+1��0x1021������ʼֵ0xFFFF����λ�ں󣬸�λ��ǰ�������0x0000���
#define CRC16_X25 6         //����ʽx16+x12+x5+1��0x1021������ʼֵ0xFFFF����λ��ǰ����λ�ں󣬽����0xFFFF���
#define CRC16_XMODEM 7      //����ʽx16+x12+x5+1��0x1021������ʼֵ0x0000����λ�ں󣬸�λ��ǰ�������0x0000���

extern void crc_table_init(unsigned char crcInfoIndex);
extern width_t crc_compute(unsigned char *ptr, unsigned int len);

#endif // !__CRC16_H
