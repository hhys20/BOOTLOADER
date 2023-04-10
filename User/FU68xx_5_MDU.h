/******************* (C) COPYRIGHT 2020 Fortiortech Shenzhen *******************
* File Name          : FU68xx_2_SMDU.h
* Creat Author       : Any Lin, R&D
* Modify Author      : Any Lin, R&D
* Creat Date         : 2020-08-19
* Modify Date        : 2020-08-21
* Description        :
********************************************************************************
* All Rights Reserved
*******************************************************************************/

#ifndef __FU68XX_5_SMDU_H__
#define __FU68XX_5_SMDU_H__

/******************************************************************************///Including Header Files
#include <FU68xx_5_MCU.h>
/******************************************************************************///Define Macro
/******************************************************************************///Define Type
/**
 * @brief SMDU的模式类型
 *
 * @note 使用@ref SMDU_RunNoBlock时, 其中的mode参数可以直接使用本枚举的内容
 * @note 使用@ref SMDU_RunBlock时, 其中的mode参数可以直接使用本枚举的内容
 */
typedef enum
{
    S1MUL   = 0, /**< 有符号乘法, 计算结果左移1位 */
    SMUL    = 1, /**< 有符号乘法 */
    UMUL    = 2, /**< 无符号乘法 */
    DIV     = 3, /**< 32/16无符号除法 */
    SIN_COS = 4, /**< Sin/Cos */
    ATAN    = 5, /**< ATan */
    LPF     = 6, /**< 低通滤波 */
    PI      = 7  /**< PI */
} ETypeSMDUMode;
/******************************************************************************///External Symbols
/******************************************************************************///External Function
/**
 * @brief 运行SMDU且不等待运行结束
 *
 * @param  stan (0-3) 要启动的计算单元编号
 * @param  mode (0-7) 指定计算单元的模式, 可使用@ref ETypeSMDUMode 作为计算模式的设置\n
 * @ref S1MUL   有符号乘法, 计算结果左移1位 \n
 * @ref SMUL    有符号乘法 \n
 * @ref UMUL    无符号乘法 \n
 * @ref DIV     32/16无符号除法 \n
 * @ref SIN_COS Sin/Cos \n
 * @ref ATAN    ATan \n
 * @ref LPF     低通滤波 \n
 * @ref PI      PI \n
 */
#define SMDU_RunNoBlock(stan, mode)   do                                                  \
                                      {                                                   \
                                          MDU_CR = MDUSTA0 << stan | (unsigned char)mode; \
                                      } while (0)

/**
 * @brief 运行SMDU且等待运行结束
 *
 * @param  stan (0-3) 要启动的计算单元编号
 * @param  mode (0-7) 指定计算单元的模式, 可使用@ref ETypeSMDUMode 作为计算模式的设置\n
 * @ref S1MUL   有符号乘法, 计算结果左移1位 \n
 * @ref SMUL    有符号乘法 \n
 * @ref UMUL    无符号乘法 \n
 * @ref DIV     32/16无符号除法 \n
 * @ref SIN_COS Sin/Cos \n
 * @ref ATAN    ATan \n
 * @ref LPF     低通滤波 \n
 * @ref PI      PI \n
 */
#define SMDU_RunBlock(stan, mode)   do                                       \
                                    {                                        \
                                        SMDU_RunNoBlock(stan, mode);         \
                                        while (MDU_CR & MDUBSY);             \
                                    } while (0);
																		
/**
 * 32Bit/16Bit除法----求商,只取低位
 *
 * @CodeLen 0(Xdata) 0(Idata) 0(Data)
 * @RunTime 0(Xdata) 0(Idata) 0(Data)
 * @Exp     C = A / B
 *
 * @param   wAh  (uint16)  被除数的高16位
 * @param   wAl  (uint16)  被除数的低16位
 * @param   wB   (uint16)  除数
 * @param   wCl  (uint16)  商的低16位
 */
#define DivQ_L_MDU(wAh, wAl, wB, wCl)                       do {\
                                                                DIV3_DAH  = wAh;\
                                                                DIV3_DAL  = wAl;\
                                                                DIV3_DB  	= wB;\	
                                                                MDU_CR 		= MDUMOD1 | MDUMOD0|MDUSTA3;\
                                                                while (MDU_CR & MDUBSY);\
                                                                wCl    = DIV3_DQL;\
                                                            } while (0)																		


/*************************************************************************************///External Function
/**
 * 低通滤波器  使用计算单元0 低通滤波模式 MDU_CR=MDUMOD2|MDUMOD1|MDUSTA0;
 *
 * @CodeLen 0x0049(Xdata) 0x003d(Idata) 0x002f(Data)
 * @RunTime 4.21us(Xdata) 2.88us(Idata) 2.29us(Data)
 * @Exp     Y += K * (X - Y)
 *
 * @param   iYh  (int16)  滤波输出的高16位(填入上一次计算的输出，取出新的输出)
 * @param   iYl  (int16)  滤波输出的低16位(填入上一次计算的输出，取出新的输出)
 * @param   iX   (int16)  滤波输入
 * @param   ucK  (uint8)  滤波系数(Q8格式：0~255)
 */
#define LPF_MDU(iX, ucK, iYh, iYl)                          do { \
                                                                LPF0_K  = ucK;\
                                                                LPF0_X  = iX;\
                                                                LPF0_YH  = iYh;\
                                                                MDU_CR = MDUMOD2|MDUMOD1|MDUSTA0;\
                                                                while (MDU_CR & MDUBSY);\
                                                                iYh    = LPF0_YH;\
                                                                iYl    = LPF0_YL;\
                                                            } while (0)
																
#endif
