#ifndef _LED_H
#define _LED_H
#include "stm32f10x_conf.h"


#define HUB08_CLK_PIN 		GPIO_Pin_7
#define HUB08_CLK_PORT 		GPIOC

#define HUB08_R1_PIN 		GPIO_Pin_0
#define HUB08_R1_PORT 		GPIOC

#define HUB08_G1_PIN 	 	GPIO_Pin_1
#define HUB08_G1_PORT 		GPIOC

#define HUB08_UP_PORT		GPIOC
#define HUB08_UP_PIN_MASK	0x03

#define HUB08_R2_PIN 		GPIO_Pin_2
#define HUB08_R2_PORT 		GPIOC

#define HUB08_G2_PIN 	 	GPIO_Pin_3
#define HUB08_G2_PORT 		GPIOC

#define HUB08_DN_PORT		GPIOC
#define HUB08_DN_PIN_MASK	0x0C


#define HUB08_LAT_PIN 		GPIO_Pin_10
#define HUB08_LAT_PORT 		GPIOB

#define HUB08_OE_PIN 		GPIO_Pin_11
#define HUB08_OE_PORT 		GPIOB
/* HUB line select GPIOB pin7~10 */
#define HUB_LINE_PORT		GPIOA
#define HUB_LINE_MASK_SHIFT	0
#define HUB_LINE_MASK 		(0x000F << HUB_LINE_MASK_SHIFT)


#define HUB08_A_PIN 		GPIO_Pin_0
#define HUB08_A_PORT 		GPIOA

#define HUB08_B_PIN 		GPIO_Pin_1
#define HUB08_B_PORT 		GPIOA

#define HUB08_C_PIN 		GPIO_Pin_2
#define HUB08_C_PORT 		GPIOA

#define HUB08_D_PIN 		GPIO_Pin_3
#define HUB08_D_PORT 		GPIOA


#define LED_HORISON_SIZE	128
#define LED_VEHICLE_SIZE	32
#define LED_SCAN_PERCENT	16
#define LED_MAX_DOT			(LED_HORISON_SIZE * LED_VEHICLE_SIZE)
#define LED_MAX_BYTES		(LED_HORISON_SIZE * LED_VEHICLE_SIZE)

#define DOT_CLEAR	0x00
#define DOT_RED		0x01
#define DOT_GREEN	0x02
#define DOT_YELLOW	0x03

#define EN_HIGHT	20
#define EN_WIDTH	10
#define CH_HIGHT	20
#define CH_WIDTH	20


extern const uint8_t arrow[];
extern const uint8_t hanzi_font[];
void HUB08_Init(void);
void HUB08_scan(void);
void HUB08_selectLine(uint8_t line);
void disbuffer_exchange(void);
void disp_clear(void);
void disp_full(uint8_t color);

void disp_bytes(uint8_t bytes);
__inline  void led_set_pix(short x, short y, uint8_t color);
__inline  void led_clear_pix(short x, short y);
void led_clear_area(short x,short y,short width, short height);

void led_picture(short x,short y,
                short width, short height, const uint8_t *pic);
void led_set_color(uint8_t color);


void led_print(short x, short y, const char *str);
void led_little_print(short x, short y, const char *str);

#endif

