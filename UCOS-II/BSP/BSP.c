#include "includes.h"
#include "stdio.h"
#include "stm32f10x_flash.h"
#include "led.h"
#include "timer3.h"


static void GPIO_Config(void);
static void NVIC_Configuration(void);
static void myWatchDog_Init(void);

struct serial_buffer_s rs485_buf;

void BSP_Init(void)
{

	NVIC_Configuration();
	GPIO_Config();
	myTime3_Init();
	HUB08_Init();
	myWatchDog_Init();
}

static void GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;


	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB ,ENABLE);

	/* green led */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;

	GPIO_Init(GPIOB, &GPIO_InitStructure);

}

/*
 * 函数名：USART3_Config
 * 描述  ：USART3 GPIO 配置,工作模式配置
 * 输入  ：无
 * 输出  : 无
 * 调用  ：外部调用
 */
void USART3_Config(void)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* config USART3 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);//RCC_APB2Periph_USART3 |
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE );
	GPIO_PinRemapConfig(GPIO_PartialRemap_USART3, ENABLE);

	/* USART3 GPIO config */
	/* Configure USART3 Tx (PA.09) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Configure USART3 Rx (PA.10) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOC, &GPIO_InitStructure);


	/* USART3 mode config */
	USART_InitStructure.USART_BaudRate = 9600; //19200; //115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_9b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_Even ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

	USART_Cmd(USART3, ENABLE);

	rs485_buf.xSemaphore = OSSemCreate(0);


}

static void myWatchDog_Init(void)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);//取消寄存器写保护
	IWDG_SetPrescaler(IWDG_Prescaler_64);//设置看门狗时钟预分频值
	IWDG_SetReload(625);//设置重载值
	IWDG_ReloadCounter();
	IWDG_Enable();//使能看门狗
}
static void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	/* Configure the NVIC Preemption Priority Bits */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

	/* Enable the USARTy Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}



/*
 * 函数名：fputc
 * 描述  ：重定向c库函数printf到USART3
 * 输入  ：无
 * 输出  ：无
 * 调用  ：由printf调用
 */
//int fputc(int ch, FILE *f)
//{
//	/* 将Printf内容发往串口 */
//	USART_SendData(USART3, (unsigned char) ch);
//	while (!(USART3->SR & USART_FLAG_TXE));

//	return (ch);
//}
 void RS485_send_bytes(uint8_t *buf, uint16_t size)
{
	USART3->CR1 &= ~USART_Mode_Rx;	//	Disable rx
	while(size--)
	{
		USART_SendData(USART3, (unsigned char) *buf++);
		while (!(USART3->SR & USART_FLAG_TXE));
	}
	while (!(USART3->SR & USART_FLAG_TC));
	USART3->CR1 |= USART_Mode_Rx;	//	Enable rx
}

void _read_data(uint8_t *buf, uint16_t size)
{
	uint16_t i;
	for (i = 0; i< size; i++)
	{
		*buf++ = rs485_buf.buf[rs485_buf.getIndex++];
		if (rs485_buf.getIndex == BUFFER_SIZE)
		{
			rs485_buf.getIndex = 0;
		}
		rs485_buf.length --;
		if (rs485_buf.length == 0)
		{
			break;
		}
	}
}
uint16_t RS485_recieve_bytes(uint8_t *buf, uint16_t size)
{
	INT8U err;
	if (rs485_buf.length >= size)
	{
		_read_data(buf, size);
		return size;
	}
	rs485_buf.wanted_num = size;
	OSSemPend( rs485_buf.xSemaphore, 0, &err );
	if (err == OS_ERR_NONE)
	{
		/* The event has occurred, process it here. */
		rs485_buf.wanted_num = 0;
		_read_data(buf, size);
		return size;

		/* Processing is complete, return to wait for the next event. */
	}
	return 0;

}


void USART3_IRQHandler(void)
{
	uint8_t Res;
	unsigned char i=0;
  if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
  {
    /* Clear the USART3 Receive interrupt */
    USART_ClearITPendingBit(USART3, USART_IT_RXNE);

    /* Read one byte from the receive data register */
    Res = USART_ReceiveData(USART3) & 0xFF;
	if (rs485_buf.length < BUFFER_SIZE)
	{
		rs485_buf.buf[rs485_buf.putIndex++] = Res;
		rs485_buf.length++;
		if (rs485_buf.putIndex == BUFFER_SIZE)
		{
			rs485_buf.putIndex = 0;
		}

	}
	if ((rs485_buf.length >= rs485_buf.wanted_num) && (rs485_buf.wanted_num))
	{
		/* The event has occurred, use the semaphore to unblock the task so the task
		can process the event. */
		OSSemPost(rs485_buf.xSemaphore);
	}

  }

  if(USART_GetITStatus(USART3, USART_IT_TXE) != RESET)
  {
    /* Write one byte to the transmit data register */


    /* Clear the USART1 transmit interrupt */
    USART_ClearITPendingBit(USART3, USART_IT_TXE);

  }
}
/******************* (C) COPYRIGHT 2012 WildFire Team *****END OF FILE************/
