/********************************************************************************

 **** Copyright (C), 2020, Fortior Technology Co., Ltd.                      ****

 ********************************************************************************
 * File Name     : FU68xx_5_Type.h
 * Author        : Bruce HW&RD
 * Date          : 2020-12-02
 * Description   : .C file function description
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2020-12-02
 *   Author      : Bruce HW&RD
 *   Modification: Created file

********************************************************************************/

#ifndef __FU68XX_5_TYPE_H__
#define __FU68XX_5_TYPE_H__

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

#endif
