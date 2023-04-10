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
#define _I                                        volatile const                // ֻ���Ĵ�����־
#define _O                                        volatile                      // ֻд�Ĵ�����־
#define _IO                                       volatile                      // ��д�Ĵ�����־

typedef unsigned char                             uint8;                        // �޷����ַ��ͱ���
typedef unsigned short                            uint16;                       // �޷������ͱ���
typedef unsigned long                             uint32;                       // �޷��ų����ͱ���
typedef long                                      int32;                        // �з����ַ��ͱ���
typedef short                                     int16;                        // �з������ͱ���
typedef char                                      int8;                         // �з����ֳ�������
#define bool                                      bit                           // ��������

// �����ͱ���ֵ
#define false                                     (0)
#define true                                      (!false)

#endif
