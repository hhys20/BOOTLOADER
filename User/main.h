/******************* (C) COPYRIGHT 2019 Fortiortech Shenzhen *******************
* File Name          : main.h
* Creat Author       : Any Lin, R&D
* Modify Author      : Any Lin, R&D
* Creat Date         : 2019-04-25
* Modify Date        : 2019-04-26
* Description        : 1. FLAG_BASE 下的标志独占一个扇区, 扇区中不可以有与主程序或Bootloader相关的代码
*                      2. 主程序最好以一个新的扇区开始以方便flash操作
********************************************************************************
* All Rights Reserved
*******************************************************************************/

#ifndef __MAIN_H__
#define __MAIN_H__
/****************************** 芯片存储参数 **********************************/
#define FLA_SECTOR_WIDT                 256U                                     /* 扇区宽度 */
#define FLASH_ROOM                      (32 * 1024)                              /* FLASH总空间大小 */
#define FLASH_END_ADDR                  (FLASH_ROOM - FLA_SECTOR_WIDT)           /* 操作最大地址 最后一个扇区不能使用 */
#define FLAG_BASE                       0x1000                                   /* 标志存放基址, 位于flash中 */
#define PADDR                           0x1100                                   /* 存放主程序的起始地址 */
#define FLAG_SIZE                       (FLASH_END_ADDR-PADDR)                   /* 可编程的最大宽度 */
#define ERRADDR                         (FLASH_ROOM - (FLA_SECTOR_WIDT*2))       /* 存放故障的地址可以自己设置位置 */
#define CRCSize                         0x5000                                   /* 校验app段数据是否正确的 */
#define WDTtime   600U       /***ms 看门狗复位时间***/
/**********************************UDS配置参数**********************************/
#define LIN_UDS (0)       /* LIN的 NDA地址字段 */
#define UDS_Mode (LIN_UDS)
/* 0x27服务对应的密匙 */
#define PARA1 (0x00)
#define PARA2 (0x11)
#define PARA3 (0x22)
#define PARA4 (0x33)
/* 0x27服务对应的key */
#define KEY1 (0xff)
#define KEY2 (0x55)
#define KEY3 (0xff)
#define KEY4 (0x55)

/* 0x31服务对应的删除存储空间 */
#define RIEFM1 (0xff)
#define RIEFM2 (0x00)
/* 0x31服务对应的CRC校验 */
#define RICRC1 (0x55)
#define RICRC2 (0x00)
/* 0x34服务LFID */
#define LFID (0x10)
/* MNROB */
#define Block_Len (66)/* 实际有效的长度是64 但是数据首保还包含两个（SID,BSC）字节 */
#if (UDS_Mode == LIN_UDS)
/* 0x3C主节点请求的frameID，0x3D从节点响应的frameID */
//#define MNAD (0x00)                /* NDA第一次为0 后续为app写入的nad */
#define SBYTE (1)                  /* 起始字节  PCI所在字节 */
#define FirstFrameLength (5)       /* 首帧连续帧有效的数据长度 */
#define ConsecutiveFrameLength (6) /* 连续帧有效的数据长度 */
#define Unusedbyte (0xff)          /* 未使用的数据字节填充有值 FF */
#endif
