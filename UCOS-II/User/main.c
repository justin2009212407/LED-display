/************* STM32f103RBT6 ****************
* Filename   : main.c
* Author     : hurongbo(hurongbo@foxmail.com)
* Date       : 2012/12/8
********************************************/
#include "includes.h"
#include "led.h"
#include <stdio.h>
#include <string.h>
struct floor_msg
{
	uint8_t elevator[3];
	uint8_t floor_seg[3];
	uint8_t warn_flag[3];
};

#define MAX_MSG_ITEM_SIZE	6
#define MAX_MSG_CONTENT_SIZE	16
static OS_STK startup_task_stk[STARTUP_TASK_STK_SIZE];
static OS_STK rs485_tx_task_stk[RS485_TX_TASK_STK_SIZE];
static OS_STK rs485_rx_task_stk[RS485_RX_TASK_STK_SIZE];
static OS_STK scan_task_stk[SCAN_TASK_STK_SIZE];
// message queue
void *MsgGrp[MAX_MSG_ITEM_SIZE];
OS_EVENT *msg_handler;
uint8_t online_Cnt[3];


const char  *floor_table[] =
{
	" ",	// 00
	"1",	// 01
	"2",	// 02
	"3",	// 03
	"4",	// 04
	"5",	// 05
	"6",	// 06
	"7",	// 07
	"8",	// 08
	"9",	// 09
	"10",	// 0A
	"11",	// 0B
	"12",	// 0C
	"13",	// 0D
	"14",	// 0E
	"15",	// 0F
	"16",	// 10
	"17",	// 11
	"18",	// 12
	"19",	// 13
	"20",	// 14
	"21",	// 15
	"22",	// 16
	"23",	// 17
	"24",	// 18
	"25",	// 19
	"26",	// 1A
	"27",	// 1B
	"28",	// 1C
	"29",	// 1D
	"30",	// 1E
	"31",	// 1F
	"32",	// 20
	"33",	// 21
	"34",	// 22
	"35",	// 23
	"36",	// 24
	"37",	// 25
	"38",	// 26
	"39",	// 27
	"40",	// 28
	"41",	// 29
	"42",	// 2A
	"43",	// 2B
	"44",	// 2C
	"45",	// 2D
	"46",	// 2E
	"47",	// 2F
	"48",	// 30
	"0",	// 31
	"-1",	// 32
	"-2",	// 33
	"-3",	// 34
	"-4",	// 35
	"-5",	// 36
	"1B",	// 37
	"2B",	// 38
	"3B",	// 39
	"4B",	// 3A
	"5B",	// 3B
	"A",	// 3C
	"B",	// 3D
	"B1",	// 3E
	"B2",	// 3F
	"B3",	// 40
	"B4",	// 41
	"B5",	// 42
	"B6",	// 43
	"C",	// 44
	"D",	// 45
	"E",	// 46
	"G",	// 47
	"G1",	// 48
	"G2",	// 49
	"G3",	// 4A
	"GF",	// 4B
	"H",	// 4C
	"K",	// 4D
	"L",	// 4E
	"L1",	// 4F
	"L2",	// 50
	"L3",	// 51
	"LB",	// 52
	"LG",	// 53
	"M",	// 54
	"M1",	// 55
	"M2",	// 56
	"M3",	// 57
	"M4",	// 58
	"M5",	// 59
	"M6",	// 5A
	"MB",	// 5B
	"P",	// 5C
	"P0",	// 5D
	"P1",	// 5E
	"P2",	// 5F
	"P3",	// 60
	"P4",	// 61
	"P5",	// 62
	"PB",	// 63
	"PH",	// 64
	"PL",	// 65
	"PP",	// 66
	"R",	// 67
	"R1",	// 68
	"R2",	// 69
};
static const uint16_t ccitt_table[256] =
{
    0x0, 0x1021, 0x2042, 0x3063,
    0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b,
    0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x210, 0x3273, 0x2252,
    0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a,
    0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x420, 0x1401,
    0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509,
    0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x630,
    0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738,
    0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7,
    0x840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af,
    0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96,
    0x1a71, 0xa50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e,
    0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5,
    0x2c22, 0x3c03, 0xc60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd,
    0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4,
    0x3e13, 0x2e32, 0x1e51, 0xe70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc,
    0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb,
    0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0xa1, 0x30c2, 0x20e3,
    0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da,
    0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x2b1, 0x1290, 0x22f3, 0x32d2,
    0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589,
    0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x481,
    0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8,
    0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x691, 0x16b0,
    0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f,
    0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827,
    0x18c0, 0x8e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e,
    0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16,
    0xaf1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d,
    0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45,
    0x3ca2, 0x2c83, 0x1ce0, 0xcc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c,
    0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74,
    0x2e93, 0x3eb2, 0xed1, 0x1ef0,
};
uint16_t my_crc_ccitt(uint8_t *q, uint32_t len)
{
    uint16_t crc = 0;
    uint16_t b;
	int i;
    while (len-- > 0)
    {
        b = ccitt_table[(crc >> 8) & 0xff];
        crc = ((crc << 8) + *q++) ^ b;
    }
    for (i = 0; i<2; i++)
    {
        b = ccitt_table[(crc >> 8) & 0xff];
        crc = (crc << 8) ^ b;
    }
    return crc;
}
static void SysTick_Init()
{
	//System clock:9M,time 1ms
	SysTick_Config(9000);
	//Set AHB clock as SysTick clock source
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
}

#define FRAME_HEAD	0xFA
#define FRAME_TAIL	0xFE
#define MAST_ADDR	0xFF
static void rs485_tx_task(void *p_arg)
{
#if 0
	uint8_t tx_buf[4];
	
	tx_buf[0] = FRAME_HEAD;
	tx_buf[1] = MAST_ADDR;
	tx_buf[2] = 0x00;	//	dst addr
	tx_buf[3] = FRAME_TAIL;
#endif 
 	while(1)
 	{

//		RS485_send_bytes(tx_buf, 4);
//		if (++tx_buf[2] >= 3)
//		{
//			tx_buf[2] = 0;
//		}
		IWDG_ReloadCounter();
		OSTimeDlyHMSM(0,0,0,50);

 	}
}



static void rs485_rx_task(void *p_arg)
{
	uint8_t rx_buf[16];
	struct floor_msg msg;	
	uint16_t readLen;
	uint16_t crc16;
	USART3_Config();

	while(1)
	{
		// recieve head
		readLen = RS485_recieve_bytes(rx_buf, 1);
		if (readLen != 1) continue;
		
		if (rx_buf[0] != FRAME_HEAD) continue;

		// recieve remain 5 bytes
		readLen = RS485_recieve_bytes(rx_buf, 8);
		if (readLen != 8) continue;
		
		// crc16 check
		crc16 = my_crc_ccitt(rx_buf, 6);
		if (((crc16 >> 8) & 0xff) != rx_buf[6])
			continue;
		if ((crc16  & 0xff) != rx_buf[7])
			continue;
		
		// Toggle LED
		if (GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_12))
		{
			GPIO_ResetBits(GPIOB, GPIO_Pin_12);
		}
		else
		{
			GPIO_SetBits(GPIOB, GPIO_Pin_12);
		}

		
		msg.floor_seg[0] = rx_buf[0] & 0x7f;
		msg.warn_flag[0] = rx_buf[1] & 0x1f;
		msg.elevator[0] = 0x00;
		
		msg.floor_seg[1] = rx_buf[2] & 0x7f;
		msg.warn_flag[1] = rx_buf[3] & 0x1f;
		msg.elevator[1] = 0x01;
		
		msg.floor_seg[2] = rx_buf[4] & 0x7f;
		msg.warn_flag[2] = rx_buf[5] & 0x1f;
		msg.elevator[2] = 0x02;
		OSQPost(msg_handler, &msg);		
	}
}
const char *elevator_name_str[] = 
{
	"12#",
	"10#",
	"11#"
};
static void dis_floor(const char *name, uint8_t ele, uint8_t flag)
{
	short x, y;
	uint8_t x_size;
	short id_x, id_y;
	uint8_t id_x_size;
	uint8_t dir;
	const uint8_t *arrow_base = NULL;
	led_clear_area(1 + ele *42,0,42, 32);
	x_size = strlen(name) * EN_WIDTH;
	id_x_size = 3*6;
	x = (42 - x_size ) / 2 + ele *42;	//	ele = 0, 1, 2
	y = (32 - EN_HIGHT);
	id_x = (42-id_x_size) / 2 + ele*42;
	id_y = 0;
	dir = flag & 0x03;
	if (dir == 0x01)	// up run
	{
		arrow_base = arrow;

	}
	else if (dir == 0x02)	// down run
	{
		arrow_base = arrow + 40;

	}
	else
	{
		arrow_base = NULL;
	}
	if (x_size > 0)
	{
		led_little_print(id_x, id_y, elevator_name_str[ele]);
	}
	
	if ((flag & 0x18) == 0x10)	// ¼ìÐÞ
	{
		led_set_color(DOT_RED);
		led_picture(1+ele*42, y, CH_WIDTH, CH_HIGHT, hanzi_font + 60 *3);
		led_picture(1+ele*42 + 20, y, CH_WIDTH, CH_HIGHT, hanzi_font + 60 *4);
	}
	else if ((flag & 0x18) == 0x08)	// ¹ÊÕÏ
	{
		led_set_color(DOT_RED);
		led_picture(1+ele*42, y, CH_WIDTH, CH_HIGHT, hanzi_font + 60 *1);
		led_picture(1+ele*42 + 20, y, CH_WIDTH, CH_HIGHT, hanzi_font + 60 *2);

	}
	else
	{
		if (arrow_base != NULL)
		{
			led_set_color(DOT_RED);
			x = (42 - x_size - EN_WIDTH) / 2 + ele *42;	//	ele = 0, 1, 2
			led_print(x + EN_WIDTH,y,name);
			led_picture(x,y, EN_WIDTH, EN_HIGHT, arrow_base);
		}
		else
		{
			led_set_color(DOT_RED);
			x = (42 - x_size) / 2 + ele *42; //	ele = 0, 1, 2
			led_print(x,y,name);

		}
	}




}
extern const uint8_t hanzi_font[];
static void scan_task(void *p_arg)
{
	uint8_t err;
	struct floor_msg *pMsg = NULL;

	disp_clear();
	disbuffer_exchange();
	led_set_color(DOT_RED);
//	led_picture(1, 2, 20, 20, hanzi_font + 60);
//	disbuffer_exchange();
//	disp_full(DOT_RED);
//	disbuffer_exchange();
//	OSTimeDlyHMSM(0,0,1,0);

//	led_clear_area(1,1,60,16);
//	disbuffer_exchange();
//	OSTimeDlyHMSM(0,0,2,0);

//	while(1)
//	{
//		dis_floor(floor_table[18], 0, 0x01);
//		dis_floor(floor_table[19], 1, 0x02);
//		dis_floor(floor_table[20], 2, 0x01);
//		disbuffer_exchange();
//		OSTimeDlyHMSM(0,0,2,0);
//	}
	while(1)
	{
		pMsg = OSQPend(msg_handler, 3000, &err);
		if ((err == OS_ERR_NONE) && (pMsg != NULL))
		{
			if((pMsg->floor_seg[0] != 0) || (pMsg->warn_flag[0] != 0))
			{
				online_Cnt[pMsg->elevator[0]] = 0;
				dis_floor(floor_table[pMsg->floor_seg[0]], pMsg->elevator[0], pMsg->warn_flag[0]);
			}
			if((pMsg->floor_seg[1] != 0) || (pMsg->warn_flag[1] != 0))
			{
				online_Cnt[pMsg->elevator[1]] = 0;
				dis_floor(floor_table[pMsg->floor_seg[1]], pMsg->elevator[1], pMsg->warn_flag[1]);
			}
			if((pMsg->floor_seg[2] != 0) || (pMsg->warn_flag[2] != 0))
			{
				online_Cnt[pMsg->elevator[2]] = 0;
				dis_floor(floor_table[pMsg->floor_seg[2]], pMsg->elevator[2], pMsg->warn_flag[2]);
			}
								
			disbuffer_exchange();
		}
		else if (err == OS_ERR_TIMEOUT)
		{
			disp_clear();
			disbuffer_exchange();
		}

	}
}
static void startup_task(void *p_arg)
{

	SysTick_Init();

	#if (OS_TASK_STAT_EN>0)
		OSStatInit();
	#endif

	//	create msessage queue
	msg_handler = OSQCreate(MsgGrp, MAX_MSG_CONTENT_SIZE);


	OSTaskCreate(rs485_tx_task,(void *)0,
				&rs485_tx_task_stk[RS485_TX_TASK_STK_SIZE-1],
				RS485_TX_TASK_PRIO);

	OSTaskCreate(rs485_rx_task,(void *)0,
				&rs485_rx_task_stk[RS485_RX_TASK_STK_SIZE-1],
				RS485_RX_TASK_PRIO);

	OSTaskCreate(scan_task,(void *)0,
				&scan_task_stk[SCAN_TASK_STK_SIZE-1],
				SCAN_TASK_PRIO);
	while(1)
	{
		if (online_Cnt[0] < 5)
		{
			online_Cnt[0]++;
		}
		else
		{
			dis_floor("", 0, 0x00);
			disbuffer_exchange();
		}

		if (online_Cnt[1] < 5)
		{
			online_Cnt[1]++;
		}
		else
		{
			dis_floor("", 1, 0x00);
			disbuffer_exchange();
		}

		if (online_Cnt[2] < 5)
		{
			online_Cnt[2]++;
		}
		else
		{
			dis_floor("", 2, 0x00);
			disbuffer_exchange();
		}


		OSTimeDlyHMSM(0,0,1,0);

	}
}

int main()
{
	BSP_Init();
	OSInit();

	OSTaskCreate(startup_task,(void *)0,
				&startup_task_stk[STARTUP_TASK_STK_SIZE-1],
				STARTUP_TASK_PRIO);

	OSStart();
	return 0;
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif
