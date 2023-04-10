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

#define _I                              volatile const //< Defines 'read only' permissions
#define _O                              volatile       //< Defines 'write only' permissions
#define _IO                             volatile       //< Defines 'read&write' permissions

//#define bool                            bit
//#define false                           (0)
//#define true                            (!false)

typedef unsigned char                   uint8;
typedef unsigned short                  uint16;
typedef unsigned long                   uint32;
typedef long                            int32;
typedef short                           int16;
typedef char                            int8;

typedef enum{false = 0, true}           bool;
typedef enum{DISABLE = 0, ENABLE}       ebool;

#endif
