/**************************** (C) COPYRIGHT 2017 Fortiortech Shenzhen ******************************
* File Name          : FU68xx_3_Type.h
* Author             : Any Lin
* Version            : V1.0
* Date               : 2018-03-08
* Description        :
****************************************************************************************************
* All Rights Reserved
***************************************************************************************************/

#ifndef __FU68XX_3_TYPE_H__
#define __FU68XX_3_TYPE_H__

/**************************************************************************************************///Including Header Files
/**************************************************************************************************///Define Macro
#define _I                                        volatile const                // 只读寄存器标志
#define _O                                        volatile                      // 只写寄存器标志
#define _IO                                       volatile                      // 读写寄存器标志

typedef unsigned char                             uint8;                        // 无符号字符型变量
typedef unsigned short                            uint16;                       // 无符号整型变量
typedef unsigned long                             uint32;                       // 无符号长整型变量
typedef long                                      int32;                        // 有符号字符型变量
typedef short                                     int16;                        // 有符号整型变量
typedef char                                      int8;                         // 有符号字长整变量
#define bool                                      bit                           // 布尔变量

// 布尔型变量值
#define false                                     (0)
#define true                                      (!false)

#define DISABLE                                   (false)                       // 定义禁能标志
#define ENABLE                                    (true)                        // 定义使能标志
/**************************************************************************************************///Define Global Symbols
/**************************************************************************************************///Function Subject
#endif