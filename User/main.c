/******************* (C) COPYRIGHT 2019 Fortiortech Shenzhen *******************
 * File Name          : main.c
 * Creat Author       : Any Lin, R&D
 * Modify Author      : carl chen, R&D
 * Creat Date         : 2022-02-11
 * Modify Date        : 2022-02-11
 * Description        :
 ********************************************************************************
 * All Rights Reserved
 *******************************************************************************/

/******************************************************************************/ // Including Header Files
#include <main.h>
#include <FU68xx_5_MCU.h>
/******************************************************************************/ // Define Macro
#define GOTO_MAIN (FLAG_BASE + 0x00)                                             // 31数据校验完成标志  Flash_Sector_Write(GOTO_MAIN, 0xaa); //boot完成标记
#define MAIN_NO_ENPTY (FLAG_BASE + 0x01)                                         // 主程序是否空标志
#define NAD_ADD (FLAG_BASE + 0x02)                                               // 主程序是否空标志
#define CRCH_ADD (FLAG_BASE + 0x08)                                               // 存放app校验的地址
#define CRCL_ADD (FLAG_BASE + 0x09)                                               // 存放app校验的地址
/******************************************************************************/ // Define Global Symbols
typedef struct
{
    uint8 Flowlost_flag : 1;     /* 丢包标志0：数据包正常 1：数据包丢失 */
    uint8 Flow_Mode : 2;         /* 当前的数据流状态 0：数据流没有开始 1：数据流开始 2：数据流接收完成  */
    uint8 BSC_ERR_flag : 1;      /* 36服务的bsc数据块错误 */
    uint8 : 4;                   /* 空 */
    uint8 PCI;                   /* 数据长度 */
    uint8 SID;                   /* 服务id */
    uint8 DID;                   /* 数据id */
    uint8 N_Data[Block_Len + 6]; /* uds数据 注意最大的数据长度为64+2  */
    uint8 N_DataCount;           /* 服务字节的计数 */
    uint8 N_DataFlow;            /* 多包数据流记录 数值0x20-0x2f*/
} UDS_TypeDef;                   /* UDS服务数据 */

typedef struct
{
    uint8 NAD;       /* NDA地址字段 */
    uint8 PCI;       /* 有效数据长度 */
    uint8 SID;       /* 服务ID */
    uint8 DATA[6];   /* 回复uds数据 */
    uint8 NRSID;     /* 否定应答时候的NRSID */
    uint8 BSC;       /* 否定应答时候的BSC */
    uint8 ErrorCode; /* 否定应答时候的NCR*/
} Respond_TypeDef;   /* 回复数据 */

typedef struct
{
    uint8 P_flag : 4;     /* 编程模式0：其他， 1：进入编程模式 ，2：解锁完成 ，3：删除存储空间完成 4:获取下载地址和空间 5:数据传输 6:结束数据传输 6:结束数据传输 7.CRC16数据检查*/
    uint8 KEY_flag : 1;   /* 解锁操作标志位   1：解锁完成 ，0：未解锁 */
    uint8 Reset_flag : 1; /* 复位标志 回复完数据后开始复位操作  1：等待复位 ，0：其他 */
    uint8 Erase_flag : 2; /* 擦除模式 0：结束擦除 1：接收到擦除 ，2：回复擦除 3：擦除中 */
    uint16 WriteAddress;  /* 存储地址  */
    uint32 MemoryAddress; /* 存储地址  */
    uint32 MemorySize;    /* 存储空间所占字节数 */
} Program_TypeDef;        /* 编程参数设置 */

typedef struct
{
    uint8 RUN_flag;   // LIN标志位
    uint8 Syn;        //同步数据
    uint8 PID;        // LINID 此处的PID  ,ID=PID&0x3f是低6位
    uint8 Data[8];    // LIN数据
    uint8 SData[8];   // LIN发送数据
    uint8 Leng;       // LIN数据长度
    uint8 Check;      // LIN数据校验
    uint8 CheckMode;  // LIN数据校验模式 1:增强校验和  0:标准校验
    uint8 Count;      // LIN计数
    uint8 Err;        // LIN错误
    uint8 Sleep_flag; //总线休眠
    uint8 Send_flag;  //总线发送标志位
    uint16 Timeblank; //总线4秒无数据休眠  同步间隔段时间
} LIN_Struc;

typedef enum
{
    NO_ERR = 0,    //无错误
    ERR_BLANK = 1, //同步间隔段
    ERR_SYNC = 2,  //同步帧错误
    ERR_PID = 3,   // PID超时错误
    ERR_DATA = 4,  //数据超时错误
    ERR_CHECK = 5, //效验和错误
} LINStaType;
typedef enum
{
    FLA_OK = 0,
    FLA_ERR,
    FLA_FZ,
} ETypeFlaSta;

uint8 MNAD = 0;
UDS_TypeDef xdata UDS = {0};
Respond_TypeDef xdata LINReply = {0};
Program_TypeDef xdata USER = {0};
LIN_Struc xdata Lin = {0};

/*******LIN通信硬件接口函数*********/
void MODE_IO(void);
void MODE_UART(void);
void TIM3_Init(void);
void PO0_INT(void);
void LIN_INT(void);
void TIEM3_INT(void);
void LIN_detect(void);
void Cutover_IO(void);
/*******LIN数据通信函数*********/
void LIN_Parse(uint8 *LIN_data);
void LIN_Resp(void);
void LIN_list(uint8 PID);
void ReceiveSYS(uint8 PID);
uint8 LinCheck(uint8 CheckMode, uint8 PID, uint8 Leng, uint8 *str);
void LIN_Send(uint8 *str, uint8 Check, uint8 Leng);

/******* Flash操作函数*********/
void Flash_Erase_APP(void);
void Write64Byte64Flash(uint16 FlashAddress, uint8 *Buff, uint8 leng);
void Flash_CRC_APP(uint32 Add,uint32 Size);

/******* uds下载函数*********/
void Service_Inquiry(void);
uint8 UDS_Srv10_Resp(void);
uint8 UDS_Srv27_Resp(void);
uint8 UDS_Srv31_Resp(void);
uint8 UDS_Srv34_Resp(void);
uint8 UDS_Srv36_Resp(void);
uint8 UDS_Srv37_Resp(void);
uint8 UDS_Srv11_Resp(void);
uint8 UDS_Srv19_Resp(void);

/* 配置串口2的rx管脚为外部中断 */
void MODE_IO(void)
{
    Lin.Sleep_flag = 0;         /* 总线休眠标志位 */
    UT2REN = 0;                 /* 0-->不允许串行输入 1-->允许串行输入，软件清0; */
                                /*   UT2RI = 0;  清除接收中断 */
    ClrBit(UT2_BAUD, UART2IEN); /* 关闭 UART2 中断进入io模式 */
    ClrBit(P0_OE, PIN1);        /* 输入模式*/
    ClrBit(P0_PU, PIN1);        /*上拉禁止*/
    /* 设置EXT0==0001：配置 P0.1 为外部中断 0 接口 */
    ClrBit(LVSR, EXT0CFG2); /* P01 接口外部中断 0 配置 */
    ClrBit(LVSR, EXT0CFG1);
    SetBit(LVSR, EXT0CFG0);
    /* 配置为下降沿触发中断 */
    IT01 = 1; /* 电平变化触发中断 */
    IT00 = 0;
    /* 清除外部中断 */
    IF0 = 0;
    /* 配置中断优先级 */
    EX0 = 1; /* 使能外部中断0 */
}

/*-------------------------------------------------------------------------------------------------
    Function Name :	void UART_Init(void)
    Description   :	UART 参数配置 并 初始化
    Input         :	无
    Output		  :	无
-------------------------------------------------------------------------------------------------*/
void MODE_UART(void)
{

    EX0 = 0;                   /* 关闭外部中断0  进入串口模式 */
    SetBit(PH_SEL, UART2EN);   /* P3[6]as UART2_RXD; P3[7]as UART2_TXD */
    SetBit(PH_SEL1, UART2CH1); /* 1X： P0.1 为 RXD、 P0.0 为 TXD（P0.1 为单线模式的输入输出）*/
    UT2MOD1 = 0;               /* 00-->单线制8bit        01-->8bit uart(波特率可设置) */
    UT2MOD0 = 1;               /* 10-->单线制9bit        11-->9bit uart(波特率可设置) */
    UT2SM2 = 0;                /* 0-->单机通讯          1-->多机通讯；*/
    UT2REN = 1;                /* 0-->不允许串行输入 1-->允许串行输入，软件清0; */
    UT2TB8 = 0;                /* 模式2/3下数据发送第9位，在多机通信中，可用于判断当前数据帧的数据是地址还是数据，TB8=0为数据，TB8=1为地址 */
    UT2RB8 = 0;                /* 模式2/3下数据接收第9位，若SM2=0,作为停止位 */
    ClrBit(IP3, PSPI_UT21);    /* 中断优先级 */
    SetBit(IP3, PSPI_UT20);    /* 中断优先级 */

    UT2_BAUD = 0x4D; /*波特率可设置 = 24000000/(16/(1+ UT_BAUD[BAUD_SEL]))/(UT_BAUD+1)*/
    /* 9B-->9600 0x000c-->115200 0x0005-->256000  4D-->19200*/
    ClrBit(UT2_BAUD, BAUD2_SEL); /*倍频使能0-->Disable  1-->Enable*/
    SetBit(UT2_BAUD, UART2IEN);  /* UART2 中断使能0-->Disable  1-->Enable*/
}

/*---------------------------------------------------------------------------*/
/*  Name     :   void TIM3_Init(void)
    /* Input    :   NO
    /* Output   :   NO
    /* Description: Timer3的初始化，只用于计数器使用  。5.28ms触发定时器
/*---------------------------------------------------------------------------*/
void TIM3_Init(void)
{
    ClrBit(PH_SEL, T3SEL); /* Timer3端口不使能 */

    /*-------------------------------------------------------------------------------------------------
    Tim3_CR0
    [7:5]  T4PSC2 T4PSC1 T4PSC0  定时器分频配置0
                    //000-->24M     001-->12M       010-->6M    011-->3M
                    //100-->1.5M    101-->750K      110-->375K  111-->187.5K
    [4]    T4OCM            		 输入 timer 模式：周期沿选择
    [3]    T4IRE    						 输入 timer 模式：脉宽检测中断使能
    [2]    RSV
    [1]    T4OPM          			 输入 timer 模式：PWM 周期检测或计数器上溢事件
    [0]    T4MOD            		 0：输入 timer 模式   1：输出模式
    -------------------------------------------------------------------------------------------------*/
    ClrBit(TIM3_CR0, T3PSC2); /* 计数器时钟分频选择 011-->3M   0。333us */
    SetBit(TIM3_CR0, T3PSC1); /* 000-->24M		001-->12M		010-->6M	011-->3M */
    SetBit(TIM3_CR0, T3PSC0); /* 100-->1.5M	101-->750K		110-->375K	111-->187.5K */

    SetBit(TIM3_CR0, T3OCM); /* 输入timer模式，周期沿选择   0-->高电平脉宽   1-->低电平脉宽 */
    ClrBit(TIM3_CR0, T3IRE); /* 比较匹配中断/脉宽检测中断   0-->Disable     1-->Enable */
    ClrBit(TIM3_CR0, T3OPM); /* 输入模式：PWM周期检测或计数器上溢事件  0-->计数器不停止		1-->单次模式(清除 TxCEN) */
    ClrBit(TIM3_CR0, T3MOD); /* 0-->Timer模式       1-->输出模式 */

    ClrBit(TIM3_CR1, T3IR); /* 清除标志位 比较中断 */
    ClrBit(TIM3_CR1, T3IP); /* 清除标志位 */
    ClrBit(TIM3_CR1, T3IF); /* 清除标志位 溢出中断 */

    ClrBit(TIM3_CR1, T3IPE); /* 输入Timer PWM周期检测中断使能 0-->Disable  1-->Enable */
    SetBit(TIM3_CR1, T3IFE); /* 计数器上溢中断使能 0-->Disable  1-->Enable */
    ClrBit(TIM3_CR1, T3NM1); /* 输入噪声脉宽选择 */
    ClrBit(TIM3_CR1, T3NM0); /* 00-->不滤波	01-->4cycles    10-->8cycles  11-->16cycles */
    SetBit(TIM3_CR1, T3EN);  /* TIM3使能    0-->Disable  1-->Enable */

    /* 注意溢出中断的时间要小于调度表的时间  当前设定时间10ms */
    TIM3__ARR = 24001; /*  IR比较中断 重装载寄存器 */
    TIM3__DR = 24002;  /* 设定10ms中断  IF溢出中断 没有使能 */
    TIM3__CNTR = 1;

}

/*  ----------------------------------------------------------------------------------------------*/
/*  Function Name  : put_char
    /*  Description    : put_char
    /*  Date           : 2020-09-06
    /*  Parameter      : c: [输入/出]
    /*  ----------------------------------------------------------------------------------------------*/
void put_char(uint8 c)
{
    UT2_DR = c;
    while (UT2TI == 0)
        ;
    UT2TI = 0;
}

void LIN_Send(uint8 *str, uint8 Check, uint8 Leng)
{
    uint8 i = 0;

    for (i = 0; i < Leng; i++)
    {
        put_char(str[i]);
    }
    put_char(Check);
}
/*  ----------------------------------------------------------------------------------------------*/
/*  Function Name  : LinCheck 计算数据的校验和
    /*  Description    : 输入CheckMode:校验模式 ;
                             PID:PDI;
                             Leng:数据长度;
                             str：计算数组;
    /*  Date           : 2020-09-06
    /*  Parameter      : 返回校验和
    /*  ----------------------------------------------------------------------------------------------*/
uint8 LinCheck(uint8 CheckMode, uint8 PID, uint8 Leng, uint8 *str)
{
    uint16 CheckDATA = 0; /* 默认标准校验 */
    uint8 i = 0;
    //  CheckDATA = PID;/* 增强型校验 */
    for (i = 0; i < Leng; i++)
    {
        CheckDATA = CheckDATA + str[i];
        if (CheckDATA > 0xff) /* 校验方法 */
        {
            CheckDATA = CheckDATA - 0xff;
        }
    }
    CheckDATA = 0xff - CheckDATA;
    return (uint8)CheckDATA;
}

void Cutover_IO(void)
{
    Lin.RUN_flag = 0;
    MODE_IO();
} /* 端口切换到外部中断模式 */

/*---------------------------------------------------------------------------*/
/*  Name     :   void TIEM3_INT(void)
    /* Input    :   NO
    /* Output   :   NO
    /* Description: lin总线专用的计时器
/*---------------------------------------------------------------------------*/
void TIEM3_INT(void)
{
    if (Lin.RUN_flag == 1)
    {
        Lin.RUN_flag = 0; /* 同步间隔段超时错误 */
        Lin.Err = ERR_BLANK;
    }
    else if (Lin.RUN_flag == 11)
    {
        Lin.RUN_flag = 0; /* 同步信号超时错误 */
    }
    else if (Lin.RUN_flag == 12)
    {
        Lin.RUN_flag = 0; /* 接收ID信号超时错误 */
        Lin.Err = ERR_PID;
    }
    else if (Lin.RUN_flag == 13)
    {
        Lin.RUN_flag = 0; /* 接收数据超时错误 */
        Lin.Err = ERR_DATA;
    }
    else if (Lin.RUN_flag == 23)
    {
        Lin.RUN_flag = 0; /* 发送数据超时错误 */
    }
    Cutover_IO(); /* 切换回io模式 */
                  /* put_char(0X55);//测试串口 */
    ClrBit(TIM3_CR1, T3IF);
}

/*  -------------------------------------------------------------------------------------------------
    Function Name  : FO_INT
    Description    : GP01外部中断触发事件电平变化触发中断
    Date           : 2020-04-10
    Parameter      : None
    ------------------------------------------------------------------------------------------------- */
void PO0_INT(void)
{
    if (GP01 == 0) /*接收到下降沿*/
    {
        if (Lin.RUN_flag == 0)
        {
            Lin.RUN_flag = 1;
            ClrBit(TIM3_CR1, T3EN); /* 先停止计数 */
            TIM3__CNTR = 1;         /* 清除time3的计数器 */
            Lin.Timeblank = 0;
            SetBit(TIM3_CR1, T3EN); /* 使能计数 */
        }
    }
    else
    {
        if (Lin.RUN_flag == 1)
        {
            /* 低电平持续时间满足13位的同步间隔断要求 */
            ClrBit(TIM3_CR1, T3EN); /* 先停止计数 */
            Lin.Timeblank = TIM3__CNTR;
            /* 19200对应的同步时间 13位  0.677MS  2031计数值  同步阶段误差5%可以接收 */
            // if ((Lin.Timeblank < 5060) && (Lin.Timeblank > 3860)) /*9600*/
            if ((Lin.Timeblank < 3000) && (Lin.Timeblank > 1830)) /*19200*/
            {
                Lin.RUN_flag = 11;
                MODE_UART(); /*切换到串口模式*/
            }
            else
            {
                Lin.RUN_flag = 0;
            }
            /* 数一组数据最大不会停留7ms 根据发送调度表计算 */
            TIM3__CNTR = 1;         /* 清除time3的计数器 */
            SetBit(TIM3_CR1, T3EN); /* 使能计数 */
        }
    }

    IF0 = 0; /* clear P01 interrupt flag*/
}
/* -------------------------------------------------------------------------------------------------
    Function Name  : LIN_IRQ
    Description    : LIN中断函数
    Date           : 2020-07-20
    Parameter      : None
------------------------------------------------------------------------------------------------- */
void LIN_INT(void)
{
    switch (Lin.RUN_flag)
    {

    /**************接收同步信号***********/
    case 11:
    {
        Lin.Syn = UT2_DR;
        if (Lin.Syn == 0X55) /* 同步信号 */
        {
            Lin.RUN_flag = 12;
        }
        else
        {
            Lin.Err = ERR_SYNC;
        }
    }
    break;

    /**************注意此处接收的不是ID而是PID，PID是id加前两位校验组合得到的注意,注意,注意,注意,注意***********/
    case 12:
    {
        Lin.PID = UT2_DR;
        /*--------数据列表-----------*/
        LIN_list(Lin.PID);
        /*--------数据列表-----------*/
    }
    break;

    /************接收数据阶段数据长度一定要匹配上***********/
    case 13:
    {
        if (Lin.Count == Lin.Leng) /* 接收最后一个校验位 注意接收的数据长度Leng一定要正确 */
        {
            Lin.Check = UT2_DR;
            if (Lin.Check == LinCheck(Lin.CheckMode, Lin.PID, Lin.Leng, &Lin.Data))
            {
                /*--------数据列表处理-----------*/
                ReceiveSYS(Lin.PID);
                /*--------数据列表处理-----------*/
            }
            else /* 数据校验错误 */
            {
                Lin.Err = 1; /* 暂时不做错检测 */
            }
            Cutover_IO();
        }
        else /* 接收定长数据 */
        {
            Lin.Data[Lin.Count] = UT2_DR;
            Lin.Count++;
        }
    }
    break;

        /************数据发送***********/
    case 23:
        break;

    /************接收到其他数据返回IO检测***********/
    default:
    {
        Cutover_IO();
        break;
    }
    }
    UT2RI = 0; /*清除接收中断*/
}
/* lin信号数据检测 */
void LIN_detect(void)
{
    if (IF0) /* 外部中断标志 置一 */
    {
        PO0_INT();
    }
    if (UT2RI) /*接收完成中断*/
    {
        LIN_INT();
    }
    if (ReadBit(TIM3_CR1, T3IF)) /*定时器3溢出中断事件*/
    {
        TIEM3_INT();
    }
    if (ReadBit(TIM3_CR1, T3IP)) /*比较中断事件*/
    {
        ClrBit(TIM3_CR1, T3IP);
    }
    if (ReadBit(TIM3_CR1, T3IR)) /*比较中断事件*/
    {
        ClrBit(TIM3_CR1, T3IR);
    }
}

/*  -------------------------------------------------------------------------------------------------
    Function Name  : ReceiveSYS
    Description    : 总线接收数据处理
    Date           :
                   : [输入]
    ------------------------------------------------------------------------------------------------- */
void ReceiveSYS(uint8 PID)
{
    uint8 LIN_WD = 0;
    uint8 temp = 0;
    switch (PID)
    {
    case 0x3c: /*接收数据ID=0x3c*/
    {
        if (Lin.Data[0] == MNAD) /* 数据nda符合要求 */
        {
            LIN_Parse(&Lin.Data); /* 进入诊断函数诊断 */
        }
    }
    break;

    default:
        break;
    }
}

/******************************************************************************/ // Function Subject
ETypeFlaSta Flash_Erase(uint8 xdata *FlashAddress)
{
    SetBit(WDT_CR, WDTRF);/* 先喂狗 */
    FLA_CR = 0x05; /* 预编程使能 */
    FLA_CR = 0x03; /* 使能自擦除 */
    FLA_KEY = 0x5a;
    FLA_KEY = 0x1f; /* flash预编程解锁 */
    *FlashAddress = 0xff; /* 写任意数据 */
    FLA_CR = 0x08;        /* 完成后Flash再次上锁 */
    if ((FLA_CR & FLAERR) == FLAERR)
        return FLA_ERR;
    return FLA_OK;
}
/*
*************************************************************************************************************************************************************************************
@   Brief    ：Flash_Erase_APP
@   Param    ：清除app段的程序
@    Date    ：2022 - 01 - 11
**************************************************************************************************************************************************************************************
*/
void Flash_Erase_APP(void)
{
    uint8 i = 0;
    uint8 xdata *FlashStartAddr = PADDR;/* 保存nad配置 */
   /* 防止指针跑飞进入这里去擦除主程序 */
	if (*(unsigned char code *)GOTO_MAIN && *(unsigned char code *)MAIN_NO_ENPTY)
    {
        (*(void (*)())PADDR)(); /* 跳转函数  */ 
    }
	if(USER.P_flag == 3)
	{
	    for (i = 0; (FlashStartAddr + (i * FLA_SECTOR_WIDT) < FLASH_END_ADDR); i++)
		{
			if (Flash_Erase(FlashStartAddr + (i * FLA_SECTOR_WIDT)) != FLA_OK) /* 没有擦除正确，在擦除一次 */
			{
				Flash_Erase(FlashStartAddr + (i * FLA_SECTOR_WIDT));
			}
		}
	}

}
/* -------------------------------------------------------------------------------------------------
    Function Name  : Flash_Sector_Write
    Description    : Flash自烧写: 对扇区预编程和自擦除后，可以对扇区内的地址进行Flash烧写，
                    一次烧写一个byte,一定要在解锁后才给DPTR赋值
    Date           : 2020-07-23
    Parameter      : FlashAddress: [输入/出]
------------------------------------------------------------------------------------------------- */
void Flash_Sector_Write(uint16 FlashAddress, uint8 FlashData)
{
    /* EA = 0; Flash自烧写前先关总中断 */
    FLA_CR = 0x01;                            /* 使能Flash编程 */
    FLA_KEY = 0x5a;                           /* 解锁 */
    FLA_KEY = 0x1f;                           /* flash预编程解锁 */
    *(uint8 xdata *)FlashAddress = FlashData; /* 写编程数据 */
    FLA_CR = 0x08;                            /* 开始预编程，完成后Flash再次上锁 */
}
/* -------------------------------------------------------------------------------------------------
    Function Name  : void WriteData2Flash(uint8 xdata *BlockStartAddr,uint16 NewData2Flash)
    Description    : 写入leng个字节到FLASH。其中Input：FlashAddress：目标起始 FLASH地址  NewData2Flash：被写入数据 长度:leng
    Output：1:扇区未满,写入完成  0:扇区已满,写入失败
    Date           : 2020-07-23
    Parameter      : 断电之前写入当前的位置信息
------------------------------------------------------------------------------------------------- */
void Write64Byte64Flash(uint16 FlashAddress, uint8 *Buff, uint8 leng)
{
    uint16 i;
    uint8 FlashData = 0;
    for (i = 0; i < leng; i++)
    {
        FlashData = *Buff++;
        Flash_Sector_Write((FlashAddress + i), FlashData);
    }
}

/* -------------------------------------------------------------------------------------------------
    Function Name  : void Flash_CRC_APP(uint8 xdata *BlockStartAddr,uint16 NewData2Flash)
    Description    : CRC校验 的起始地址和结束地址是由0x36服务所给的参数决定的
    Date           : 2022-02-23
    Parameter      : 断电之前写入当前的位置信息
------------------------------------------------------------------------------------------------- */
uint8 CRCL = 0;
uint8 CRCH = 0;
void Flash_CRC_APP(uint32 Add,uint32 Size)
{
    ClrBit(CRC_CR, AUTOINT); /* 停止计算 */
    SetBit(CRC_CR, CRCVAL);  /* 将 CRC 结果初始化为 0xFFFF */
    SetBit(CRC_CR, CRCDINI); /* 初始化有效 */

    CRC_BEG = Add >> 8;
    CRC_CNT = (Size >> 8) - 1;
    SetBit(CRC_CR, AUTOINT); /* 开始计算 */
    while (ReadBit(CRC_CR, CRCDONE) == 0)
    {
        ;
    }
    ClrBit(CRC_CR, CRCPNT); /* 取crc低位 */
    CRCL = CRC_DR;
    SetBit(CRC_CR, CRCPNT); /* 取crc高位 */
    CRCH = CRC_DR;
}

/* -------------------------------------------------------------------------------------------------
    Function Name  :  LIN_Parse(uint8 *LIN_data)
    Description    :  lin接收数据解析
    Date           : 2022-02-28
------------------------------------------------------------------------------------------------- */
void LIN_Parse(uint8 *LIN_data)
{
    uint8 i, Startdata;
    uint8 Startbyte = SBYTE; /* 起始字节 */

    Startdata = LIN_data[0 + Startbyte];

    switch (Startdata & 0xf0)
    {

        /***********0x00单包数据***********/
    case 0x00:
        UDS.PCI = Startdata;                     /* 数据长度 包含数据服务*/
        UDS.SID = LIN_data[1 + Startbyte];       /* 服务id */
        UDS.DID = LIN_data[2 + Startbyte];       /* 数据id */
        UDS.N_Data[0] = LIN_data[3 + Startbyte]; /* 数据id */
        UDS.N_Data[1] = LIN_data[4 + Startbyte]; /* 数据id */
        UDS.N_Data[2] = LIN_data[5 + Startbyte]; /* 数据id */
        UDS.N_Data[3] = LIN_data[6 + Startbyte]; /* 数据id */
                                                 //        for (i = 0; i < (UDS.PCI - (2 + Startbyte)); i++) /* 数据内容存放-数据服务*/
                                                 //        {
                                                 //            UDS.N_Data[i] = LIN_data[2 + i + Startbyte];
                                                 //        }
        Service_Inquiry();                       /* 进入uds服务查询函数 */
        UDS.Flow_Mode = 0;                       /* 多包数据流未开始 */
        UDS.Flowlost_flag = 0;                   /* 清除丢包标志位 */
        break;

        /***********0x10多包首帧  实际这里只有2个多包的服务分别为 34和36***********/
    case 0x10:
        UDS.PCI = LIN_data[1 + Startbyte]; /* 数据长度 包含数据服务 */
        UDS.SID = LIN_data[2 + Startbyte]; /* 服务id */
        UDS.DID = LIN_data[3 + Startbyte]; /* 数据id */
        UDS.N_Data[0] = LIN_data[5];
        UDS.N_Data[1] = LIN_data[6];
        UDS.N_Data[2] = LIN_data[7];
        UDS.N_DataFlow = 0;    /* 设置当前的数据流编号 */
        UDS.N_DataCount = 5;   /* 这里的SID算一个字节所有从1开始计数 */
        UDS.Flow_Mode = 1;     /* 多包数据流开始标志位 */
        UDS.Flowlost_flag = 0; /* 清除丢包标志位 */
        Service_Inquiry();     /* 首包也解析一下，防止后面数据丢失导致回复参数错误 */
        break;

        /***********0x20连续帧数据流 20-2F  多包首帧之后跟随的是21***********/
    case 0x20:
        /* 添加数据流监控信号保证数据流正确 ，不然中间丢包数据流就不对了 */
        UDS.N_DataFlow++;
        if (UDS.N_DataFlow > 0x0f)
        {
            UDS.N_DataFlow = 0;
        }
        if ((UDS.N_DataFlow) != (Startdata & 0x0f)) /* 数据流正确 */
        {
            UDS.Flowlost_flag = 1; /* 数据流丢包标志 */
        }

        /* 读取有效数据 */
        for (i = 0; i < 6; i++) /* 连续帧每包会有6个数据  */
        {
            UDS.N_Data[UDS.N_DataCount - 2] = LIN_data[2 + i]; /* 采集数据 */
            UDS.N_DataCount++;                                 /* 数据记录位置+1  */
        }

        /* 数据采集完成开始处理数据 */
        if (UDS.PCI <= (UDS.N_DataCount)) /* 接收数据长度满足数据长度要求 */
        {
            if (UDS.Flow_Mode == 1) /* 数据包没有丢失 且接收完成 */
            {
                UDS.Flow_Mode = 2; /* 数据流接收状态为接收完成 */
                Service_Inquiry(); /* 进入uds服务查询函数 */
            }
            else /* 数据包丢失 丢失首帧 */
            {
                UDS.N_DataCount = 0;
                UDS.Flow_Mode = 0; /* 数据流接收状态为接收未完成，返回否定发响应 */
                Service_Inquiry(); /* 进入uds服务查询函数 */
            }
        }
        break;

    case 0x30: /* 流控帧  lin不支持流控帧  */

        break;

    default:
        break;
    }
}

/* -------------------------------------------------------------------------------------------------
    Function Name  :  Service_Inquiry(void)
    Description    :  lin服务查询
    Date           : 2022-02-28
------------------------------------------------------------------------------------------------- */
void Service_Inquiry(void)
{
    switch (UDS.SID) /* 查询服务函数*/
    {

    case 0x10: /* 进入编程模式之后回复数据使用*/
        UDS_Srv10_Resp();
        break;

    case 0x27: /* 进入安全访问模式解锁控制器 */
        UDS_Srv27_Resp();
        break;

    case 0x31: /* 1.删除app的程序代码   2.检查数据的有效性，如CRC校验 */
        UDS_Srv31_Resp();
        break;

    case 0x34: /* 设置下载起始地址和宽度，并回传36服务一次发送最大数据宽度 */
        UDS_Srv34_Resp();
        break;

    case 0x36: /* 设置下载数据 多包连续发送  */
        UDS_Srv36_Resp();
        break;

    case 0x37: /* 设置下载完成退出下载  */
        UDS_Srv37_Resp();
        break;

    case 0x11: /* 控制器复位 */
        UDS_Srv11_Resp();
        break;

    case 0x19: /* 控制器查询 */
        UDS_Srv19_Resp();
        break;
    default:
        break;
    }
}
/* 服务函数 */

/*
*************************************************************************************************************************************************************************************
@   Brief    ：Service $10 -- 进入扩展模式(10服务)
@   Param    ：3个子功能，01 Default默认会话，02 Programming编程会话，03 Extended扩展会话，ECU上电时，进入的是默认会话（Default）
@    Date    ：2022 - 02 - 11
**************************************************************************************************************************************************************************************
*/
uint8 UDS_Srv10_Resp(void)
{

    LINReply.PCI = 0x02;           /* 回复数据长度 */
    LINReply.NAD = MNAD;           /* 回复数据设置 */
    LINReply.SID = UDS.SID + 0x40; /* 回复数据ID */
    LINReply.DATA[0] = UDS.DID;    /* P2Server_max:高字节value: 50 ms */
    LINReply.NRSID = 0x00;         /* 回复数据包数 */
    switch (UDS.DID)
    {
    case 0x01:           /* Default默认会话 */
        USER.P_flag = 0; /* 编程标志禁止 */
        break;

    case 0x02: /* Programming编程会话 */
        if (USER.Reset_flag == 0)
            USER.P_flag = 1; /* 编程标志允许 */
        break;

    case 0x03:           /* Extended扩展会话  boot不会使用此模式*/
        USER.P_flag = 0; /* 编程标志禁止 */
        break;

    default:
        LINReply.PCI = 0x03;          /* 回复数据长度 */
        LINReply.NRSID = 0x7f;        /* 回复数据长度 */
        LINReply.SID = 0x10;          /* 回复数据ID */
        LINReply.ErrorCode = UDS.DID; /* 不支持的服务 */
        break;
    }
    return 0;
}

/*
*************************************************************************************************************************************************************************************
@   Brief    ：Service $11 -- ECU复位 (11服务)
@   Param    ：3个子功能，01 hardReset 硬复位，02 softReset，
@    Date    ：2022 - 02 - 11
**************************************************************************************************************************************************************************************
*/
uint8 UDS_Srv11_Resp(void)
{

    LINReply.PCI = 0x02;           /* 回复数据长度 */
    LINReply.NAD = MNAD;           /* 回复数据设置 */
    LINReply.SID = UDS.SID + 0x40; /* 回复数据ID */
    LINReply.DATA[0] = UDS.DID;    /* P2Server_max:高字节value: 50 ms */
    LINReply.NRSID = 0x00;         /* 回复故障清空 */

    switch (UDS.DID)
    {
    case 0x01:               /* hardReset 硬复位*/
        USER.Reset_flag = 1; /* 复位 */
        break;

    case 0x02:               /* softReset */
        USER.Reset_flag = 1; /* 复位 */
        break;

    default:
        LINReply.PCI = 0x03;          /* 回复数据长度 */
        LINReply.NRSID = 0x7f;        /* 回复数据长度 */
        LINReply.SID = UDS.SID;       /* 回复数据ID */
        LINReply.ErrorCode = UDS.DID; /* 不支持的服务 */
        break;
    }
    return 0;
}

/*
*************************************************************************************************************************************************************************************
@   Brief    ：Service $27 -- ECU解锁  (27服务)
@   Param    ：UDS.DID都是奇数，对应不同的权限 。 回复的did为现有did+1
@    Date    ：2022 - 02 - 11
**************************************************************************************************************************************************************************************
*/
uint8 UDS_Srv27_Resp(void)
{

    LINReply.PCI = 0x06;            /* 回复数据长度 */
    LINReply.NAD = MNAD;            /* 回复数据设置 */
    LINReply.SID = UDS.SID + 0x40;  /* 回复数据ID */
    LINReply.DATA[0] = UDS.DID + 1; /* 回复数据DID */
    LINReply.NRSID = 0x00;          /* 回复故障清空 */

    switch (UDS.DID)
    {
    case 0x01: /* 编程只要解锁01 就可以*/
        if ((UDS.N_Data[0] == PARA1) && (UDS.N_Data[1] == PARA2) && (UDS.N_Data[2] == PARA3) && (UDS.N_Data[3] == PARA4) && (USER.P_flag == 1))
        {
            LINReply.DATA[1] = KEY1; /*  */
            LINReply.DATA[2] = KEY2; /*  */
            LINReply.DATA[3] = KEY3; /*  */
            LINReply.DATA[4] = KEY4; /*  */
            USER.P_flag = 2;         /* 解锁成功 */
        }
        else
        {
            LINReply.PCI = 0x03;          /* 回复数据长度 */
            LINReply.NRSID = 0x7f;        /* 回复数据长度 */
            LINReply.SID = UDS.SID;       /* 回复数据ID */
            LINReply.ErrorCode = UDS.DID; /* 否定回复 */
        }
        break;

    case 0x03: /* softReset */
        if ((UDS.N_Data[0] == 0X44) && (UDS.N_Data[1] == 0X55) && (UDS.N_Data[2] == 0X66))
        {
            USER.KEY_flag = 1;       /* 解锁成功 */
            LINReply.DATA[1] = KEY1; /*  */
            LINReply.DATA[2] = KEY2; /*  */
            LINReply.DATA[3] = KEY3; /*  */
            LINReply.DATA[4] = KEY4; /*  */
        }
        else
        {
            LINReply.PCI = 0x03;          /* 回复数据长度 */
            LINReply.NRSID = 0x7f;        /* 回复数据长度 */
            LINReply.SID = UDS.SID;       /* 回复数据ID */
            LINReply.ErrorCode = UDS.DID; /* 否定回复 */
        }
        break;

    default:
        LINReply.PCI = 0x03;          /* 回复数据长度 */
        LINReply.NRSID = 0x7f;        /* 回复数据长度 */
        LINReply.SID = UDS.SID;       /* 回复数据ID */
        LINReply.ErrorCode = UDS.DID; /* 否定回复 */
        break;
    }
    return 0;
}

/*
*************************************************************************************************************************************************************************************
@   Brief    ：Service $31 -- 删除存储空间 ,和 CRC校验
@   Param    ：单包数据， 退出数据服务后就开始计算crc
@    Date    ：2022 - 02 - 11
**************************************************************************************************************************************************************************************
*/
uint8 UDS_Srv31_Resp(void)
{

    uint16 CRC16;
    LINReply.PCI = 0x04;           /* 回复数据长度 */
    LINReply.NAD = MNAD;           /* 回复数据设置 */
    LINReply.SID = UDS.SID + 0x40; /* 回复数据ID */
    LINReply.DATA[0] = UDS.DID;    /* 回复数据DID */
    LINReply.DATA[1] = UDS.N_Data[0];
    LINReply.DATA[2] = UDS.N_Data[1];
    LINReply.NRSID = 0x00; /* 回复故障清空 */

    switch (UDS.DID)
    {
    case 0x01:
        if ((UDS.N_Data[0] == RIEFM1) && (UDS.N_Data[1] == RIEFM2) && (USER.P_flag == 2)) /* 对比routineIdentifier */
        {
            USER.Erase_flag = 1; /* 擦除flash 标志位置一 */
            USER.P_flag = 3;
        }
        else if ((UDS.N_Data[0] == RICRC1) && (UDS.N_Data[1] == RICRC2)) /* 对比routineIdentifier */
        {
            Flash_CRC_APP(USER.MemoryAddress,USER.MemorySize); /* CRC 校验 */
            if ((UDS.N_Data[2] == CRCL) && (UDS.N_Data[3] == CRCH))
            {
                LINReply.PCI = 0x06; /* 回复数据长度 */
                LINReply.DATA[3] = CRCL;
                LINReply.DATA[4] = CRCH;
                Flash_Sector_Write(GOTO_MAIN+3, CRCL); //写入CRC
                Flash_Sector_Write(GOTO_MAIN+4, CRCH); //写入CRC
                Flash_Sector_Write(GOTO_MAIN, 0xaa); //boot完成标记
					
            }
            else
            {
                LINReply.PCI = 0x03;          /* 回复数据长度 */
                LINReply.NRSID = 0x7f;        /* 回复数据长度 */
                LINReply.SID = UDS.SID;       /* 回复数据ID */
                LINReply.ErrorCode = UDS.DID; /* 否定回复 */
            }
        }
        else
        {
            LINReply.PCI = 0x03;          /* 回复数据长度 */
            LINReply.NRSID = 0x7f;        /* 回复数据长度 */
            LINReply.SID = UDS.SID;       /* 回复数据ID */
            LINReply.ErrorCode = UDS.DID; /* 否定回复 */
        }
        break;

    case 0x02: /* 停止程序*/

        break;

    case 0x03: /* 停止程序*/

        break;

    default:                          /* NCR */
        LINReply.PCI = 0x03;          /* 回复数据长度 */
        LINReply.NRSID = 0x7f;        /* 回复数据长度 */
        LINReply.SID = UDS.SID;       /* 回复数据ID */
        LINReply.ErrorCode = UDS.DID; /* 否定回复 */
        break;
    }
    return 0;
}

/*
*************************************************************************************************************************************************************************************
@   Brief    ：Service $34 -- 下载请求  (34服务)
@   Param    ：此处是多包数据，需要注意
@    Date    ：2022 - 01 - 11
**************************************************************************************************************************************************************************************
*/
uint8 UDS_Srv34_Resp(void)
{
    uint8 len, i;
    if ((UDS.Flow_Mode == 2) && (USER.P_flag == 3)) /* 数据接收完成 */
    {
        LINReply.PCI = 0x03;           /* 回复数据长度 */
        LINReply.NAD = MNAD;           /* 回复数据设置 */
        LINReply.SID = UDS.SID + 0x40; /* 回复数据ID */
        LINReply.DATA[0] = LFID;       /* 回复数据DID */
        LINReply.DATA[1] = Block_Len;  /* 回复数据DID */
        LINReply.NRSID = 0x00;         /* 回复故障清空 */

        /* if (UDS.N_Data[0] == 0)  加密压缩信息,未使用
         {
             ;
         }*/
        USER.MemoryAddress = 0; /* 清空地址和数据长度 */
        USER.MemorySize = 0;    /* 清空地址和数据长度 */
        USER.WriteAddress = 0;
        len = UDS.N_Data[0] & 0x0f; /* MemoryAddress 所占字节的长度*/
        for (i = 0; i < len; i++)
        {
            USER.MemoryAddress = USER.MemoryAddress + (uint32)(UDS.N_Data[i + 1] << ((len - 1 - i) * 8));
        }
        len = (UDS.N_Data[0] >> 4) & 0xf; /* memorySize 所占字节的长度*/
        for (i = 0; i < len; i++)
        {
            USER.MemorySize = USER.MemorySize + (uint32)(UDS.N_Data[i + 5] << ((len - 1 - i) * 8));
        }
        LINReply.BSC = 0;     /* 重置36的包序号 */
        UDS.BSC_ERR_flag = 0; /* 重置36错误标志位 */
        USER.P_flag = 4;      /* 进入下一步骤，后面如果判断的*/
    }
    else
    {
        LINReply.PCI = 0x03;       /* 回复数据长度 */
        LINReply.NRSID = 0x7f;     /* 回复数据长度 */
        LINReply.SID = UDS.SID;    /* 回复数据ID */
        LINReply.ErrorCode = 0x13; /* 数据长度不一致出现丢包 */
    }

    if (UDS.Flowlost_flag || (FLAG_BASE > USER.MemoryAddress) || (FLAG_SIZE < USER.MemorySize)) /* 出现数据丢失现象 */
    {
        LINReply.PCI = 0x03;    /* 回复数据长度 */
        LINReply.NRSID = 0x7f;  /* 回复数据长度 */
        LINReply.SID = UDS.SID; /* 回复数据ID */
        if (UDS.Flowlost_flag)
        {
            LINReply.ErrorCode = 0x13; /* 否定回复 长度错误 */
        }
        else
        {
            LINReply.ErrorCode = 0x22; /* 否定回复 执行条件不满足 */
            USER.P_flag = 3;
        }
    }
    return 0;
}
/*
*************************************************************************************************************************************************************************************
@   Brief    ：Service $36 -- ECU数据传输  (36服务)
@   Param    ：此处是多包数据，需要注意
@    Date    ：2022 - 02 - 11
**************************************************************************************************************************************************************************************
*/
uint8 UDS_Srv36_Resp(void)
{
    if (UDS.Flow_Mode == 2) /* 数据接收完成 */
    {
        LINReply.NAD = MNAD;           /* 回复数据设置 */
        LINReply.PCI = 0x02;           /* 回复数据长度 */
        LINReply.SID = UDS.SID + 0x40; /* 回复数据ID */
        LINReply.DATA[0] = UDS.DID;    /* 回复数据BSC 首包计数器  */
        LINReply.NRSID = 0x00;         /* 回复故障清空 */

        if (((LINReply.BSC + 1) & 0Xff) != LINReply.DATA[0])
        {
            UDS.BSC_ERR_flag = 1; /* BSC数据块错误 */
        }
        else
        {
            LINReply.BSC = LINReply.DATA[0]; /* 赋值bsc计数器 用于下次计算*/
        }

        if ((!UDS.BSC_ERR_flag) && (!UDS.Flowlost_flag) && (USER.P_flag == 4)) /* BSC数据块没有错误,多包数据没有丢失 */
        {
            Write64Byte64Flash(USER.MemoryAddress + USER.WriteAddress, UDS.N_Data, (UDS.PCI - 2)); /* 向flash写入64字节数据 */
            USER.WriteAddress = USER.WriteAddress + (UDS.PCI - 2);                                 /* 计数地址向后增加 */
        }
    }

    /**********************否定回复 ********************* */
    if ((UDS.Flowlost_flag) || (UDS.BSC_ERR_flag)) /* 出现数据丢失现象 */
    {
        LINReply.PCI = 0x03;          /* 回复数据长度 */
        LINReply.NRSID = 0x7f;        /* 回复数据长度 */
        LINReply.SID = UDS.SID;       /* 回复数据ID */
        LINReply.ErrorCode = UDS.DID; /* 否定回复 */
        if (UDS.BSC_ERR_flag)
        {
            LINReply.ErrorCode = 0x73;
        }
    }
    return 0;
}
/*
*************************************************************************************************************************************************************************************
@   Brief    ：Service $37 -- 结束数据传输 (37服务)
@   Param    ：单包数据，
@    Date    ：2022 - 01 - 11
**************************************************************************************************************************************************************************************
*/
uint8 UDS_Srv37_Resp(void)
{
    LINReply.NAD = MNAD;           /* 回复数据设置 */
    LINReply.PCI = 0x01;           /* 回复数据长度 */
    LINReply.SID = UDS.SID + 0x40; /* 回复数据ID */
    LINReply.NRSID = 0x00;         /* 回复故障清空 */

    return 0;
}
/*
*************************************************************************************************************************************************************************************
@   Brief    ：Service $19 -- 读取ecu故障总数 (19服务)
@   Param    ：单包数据， 退出数据服务后就开始计算crc
@    Date    ：2022 - 01 - 11
**************************************************************************************************************************************************************************************
*/
uint8 UDS_Srv19_Resp(void)
{
    uint16 ERRcont = 0;
    uint8 i = 0;
    LINReply.NAD = MNAD;           /* 回复数据设置 */
    LINReply.PCI = 0x06;           /* 回复数据长度 */
    LINReply.SID = UDS.SID + 0x40; /* 回复数据ID */
    LINReply.DATA[0] = UDS.DID;    /* 回复数据DID */

    LINReply.DATA[1] = 0xff; /* 可用状态掩码 */
    LINReply.DATA[2] = 0x01; /* 14229-1 */

    /* 从指定的flash中读取故障数量 倒序查询
    for (i = 128; i == 0; i--)
    {
        LINReply.DATA[3] = *(uint8 code *)(ERRADDR + ((i - 1) * 2));
        LINReply.DATA[4] = *(uint8 code *)(ERRADDR + ((i - 1) * 2) + 1);
        if (LINReply.DATA[3] || LINReply.DATA[4])//检测到非零数值退出
        {
            i = 0;
        }
    }*/
    LINReply.DATA[3] = 0X01; /* 故障数高位 */
    LINReply.DATA[4] = 0X02; /* 故障数低位 */
    /* 从指定的flash中读取故障数量 */

    if ((UDS.N_Data[0] != 0x01) || (UDS.DID != 0x01))
    {
        LINReply.NRSID = 0x00;     /* 回复故障清空 */
        LINReply.PCI = 0x03;       /* 回复数据长度 */
        LINReply.NRSID = 0x7f;     /* 回复数据长度 */
        LINReply.SID = UDS.SID;    /* 回复数据ID */
        LINReply.ErrorCode = 0x01; /* 数据长度不一致出现丢包 */
    }
}
/*  -------------------------------------------------------------------------------------------------
    Function Name  : LIN_send
    Description    : lin数据发送
    Date           :
                   : [输入]
    ------------------------------------------------------------------------------------------------- */
void LIN_Resp(void)
{
    if ((Lin.Send_flag) && (!Lin.Sleep_flag)) /* lin发送时间较长4.5ms，不放入中断 */
    {
        Lin.Check = LinCheck(1, Lin.PID, Lin.Leng, &Lin.SData); /* 计算需要发送的校验和 */
        LIN_Send(&Lin.SData, Lin.Check, Lin.Leng);              /* 发送数据 */
        Cutover_IO();                                           /* 发送完成数据 开始返回到端口检测 */
        Lin.Send_flag = 0;                                      /* 发送标志位置 */
        if ((Lin.SData[2] == 0X71) && (USER.Erase_flag == 1))
        {
            Flash_Erase_APP(); /* 回复完成后擦除空间 */
            USER.Erase_flag = 0;
        }

        if ((Lin.SData[2] == 0X51) && (USER.Reset_flag))
        {
			Flash_CRC_APP(PADDR,CRCSize); /* CRC 校验 */
			Flash_Sector_Write(CRCL_ADD, CRCL); //写入CRC
			Flash_Sector_Write(CRCH_ADD, CRCH); //写入CRC
            (*(void (*)())PADDR)();  /* 回复完成跳转主程序 */
            USER.Reset_flag=0;
        }
    }
}
/*-------------------------------------------------------------------------------------------------
    Function Name :	void LIN_list(uint8 PID)
    Description   :	总线收发列表 ，对接收的数据设置长度和检验类型 。对发送的数据设置
    Input         :	要发送的字符串的地址
    Output		  :	无
-------------------------------------------------------------------------------------------------*/
void LIN_list(uint8 PID)
{
    static uint16 addr = 0x1000;

    uint8 i = 0;

    uint8 LIN_ID1 = 0;
    Lin.Count = 0; /* 清空计数 */

    switch (PID)
    {
    case 0x3C:
        Lin.RUN_flag = 13; /* 根据数据头判断是接收数据  LIN_flag = 3 ，发送数据 ，不需要回应数据 */
        Lin.Leng = 8;      /* 填写需要接收的数据长度 */
        addr = 0x1000;
        break;

    case 0x7d:
        Lin.RUN_flag = 23; /* 根据数据头判断是接收数据  LIN_flag = 3 ，发送数据 ，不需要回应数据 */
        Lin.Leng = 8;      /* 填写需要接收的数据长度 */
        addr = 0x1100;
        for (i = 0; i < 8; i++)
        {
            Lin.SData[i] = Unusedbyte; /* 清空数组 */
        }

        Lin.SData[0] = LINReply.NAD;
        Lin.SData[1] = LINReply.PCI;
        if (LINReply.NRSID) /* 否定响应 */
        {
            Lin.SData[2] = LINReply.NRSID;
            Lin.SData[3] = LINReply.SID;
            Lin.SData[4] = LINReply.ErrorCode;
        }
        else
        {
            Lin.SData[2] = LINReply.SID;
            if (LINReply.PCI >= 2)
                Lin.SData[3] = LINReply.DATA[0];
            if (LINReply.PCI >= 3)
                Lin.SData[4] = LINReply.DATA[1];
            if (LINReply.PCI >= 4)
                Lin.SData[5] = LINReply.DATA[2];
            if (LINReply.PCI >= 5)
                Lin.SData[6] = LINReply.DATA[3];
            if (LINReply.PCI >= 6)
                Lin.SData[7] = LINReply.DATA[4];
        }
        Lin.Send_flag = 1;
        break;

    case 0xFB:             /* 可以删除优化空间 */
        Lin.RUN_flag = 23; /* 根据数据头判断是接收数据  LIN_flag = 3 ，发送数据 ，不需要回应数据 */
        Lin.Leng = 8;      /* 填写需要接收的数据长度 */
        for (i = 0; i < 8; i++)
        {
            Lin.SData[i] = *(uint8 code *)(addr);
            addr++;
        }
        Lin.Send_flag = 1;
        break;
		
//	case 0xf0:             /* 可以删除优化空间 */
//        Lin.RUN_flag = 23; /* 根据数据头判断是接收数据  LIN_flag = 3 ，发送数据 ，不需要回应数据 */
//        Lin.Leng = 8;      /* 填写需要接收的数据长度 */
//        for (i = 0; i < 8; i++)
//        {
//            Lin.SData[0] = 8;
//			Lin.SData[1] = CRCH;
//			Lin.SData[2] = CRCL;
//        }
//        Lin.Send_flag = 1;
//        break;

    default:
        Cutover_IO();
        break;
    }
}
void main(void)
{
    unsigned char ucDelay;
    for (ucDelay = 0; ucDelay < 200; ucDelay--)
    {
        ;
    }   
    /* 
    GOTO_MAIN是BOOT写完数据并校验后写入的标志， 
    MAIN_NO_ENPTY是app运行后写入的标志 ，（MAIN_NO_ENPTY建议放到中断中，保证跳转后中断正常）
    */
	Flash_CRC_APP(PADDR,CRCSize); /*  app CRC 校验 */
   /* 调试上位机禁止跳转,调试可以关闭跳转 */
    if (*(unsigned char code *)GOTO_MAIN && *(unsigned char code *)MAIN_NO_ENPTY)
    {
		if((CRCH==*(unsigned char code *)CRCH_ADD)&&((CRCL==*(unsigned char code *)CRCL_ADD)))/*  app代码校验匹配 */
		{
		(*(void (*)())PADDR)(); /* 跳转函数  */ 
		}
        
    }
    MNAD = *(uint8 code *)(NAD_ADD);
    ;
    WDT_ARR = ((uint16)(65532-(uint32)WDTtime*32768/1000) >> 8);   /* 初始化看门狗 */
    MODE_IO();
    TIM3_Init();
	
	SetBit(P3_OE, PIN4); /*输入模式*/
	ClrBit(P3_PU, PIN4); /*上拉禁止*/
    GP34 = 1;            /*lin芯片唤醒管教*/
	//  不同板子唤醒管脚可能不同	
	SetBit(P1_OE, PIN0); /*输入模式*/
	ClrBit(P1_PU, PIN0); /*上拉禁止*/
    GP10 = 1;            /*lin芯片唤醒管教*/
	
	
    while (1)
    {
        SetBit(WDT_CR, WDTRF);		 /* 喂狗 */
        LIN_detect();
        LIN_Resp(); /* 数据发送 */
    }
}
