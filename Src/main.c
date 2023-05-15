/*******************************************************************************
 *
 * Description: Assignment1
 * Project name: TS_CURTAIN_MCU_V1.0.0
 *
 *
 * Last Changed By:  $Author: TrungNT $
 * Revision:         $Revision: 1.0 $
 * Last Changed:     $Date: 20/11/2021 $
 *
 ******************************************************************************/

/**Libraries******************************************************************/

#include <stdint.h>
#include <stm32f401re_gpio.h>
#include <stm32f401re_exti.h>
#include <stm32f401re_syscfg.h>
#include <stm32f401re_rcc.h>
#include <misc.h>
#include <timer.h>

/* Private define*********************************************************/
// Define Logic GPIO_PIN
#define GPIO_PIN_SET									1
#define GPIO_PIN_RESET									0
#define GPIO_PIN_LOW									0
#define GPIO_PIN_HIGH									1

// Define LED GPIO pin
// LED GREEN1 - GREEN2
#define LEDGREEN1_GPIO_PIN								GPIO_Pin_0
#define LEDGREEN1_GPIO_PORT								GPIOA

#define LEDGREEN2_GPIO_PIN								GPIO_Pin_11
#define LEDGREEN2_GPIO_PORT								GPIOA
//-----------------------------------------------------------------
// LED RED1 - RED2
#define LEDRED1_GPIO_PIN								GPIO_Pin_1
#define LEDRED1_GPIO_PORT								GPIOA

#define LEDRED2_GPIO_PIN								GPIO_Pin_13
#define LEDRED2_GPIO_PORT								GPIOB
//-----------------------------------------------------------------

// LED BLUE1 - BLUE2
#define LEDBLUE1_GPIO_PIN								GPIO_Pin_3
#define LEDBLUE1_GPIO_PORT								GPIOA

#define LEDBLUE2_GPIO_PIN								GPIO_Pin_10
#define LEDBLUE2_GPIO_PORT								GPIOA

//-----------------------------------------------------------------
// BUZZER
#define BUZZER_GPIO_PIN									GPIO_Pin_9
#define BUZZER_GPIO_PORT								GPIOC
//-----------------------------------------------------------------
// Button B2
#define BUTTONB2_GPIO_PIN								GPIO_Pin_3
#define BUTTONB2_GPIO_PORT								GPIOB
// Button B3
#define BUTTONB3_GPIO_PIN								GPIO_Pin_4
#define BUTTONB3_GPIO_PORT								GPIOA
// Button B4
#define BUTTONB4_GPIO_PIN								GPIO_Pin_0
#define BUTTONB4_GPIO_PORT								GPIOB

#define SYSFG_Clock										RCC_APB2Periph_SYSCFG

/*
 * Structure Button*********************************************************/

typedef enum {
	NO_CLICK = 0x00,
	BUTTON_PRESSED = 0x01,
	BUTTON_RELEASED = 0x02
} BUTTON_STATE;

typedef struct {
	BUTTON_STATE State;
	uint32_t timePress;
	uint32_t timeReleased;
	uint32_t Count;
} BUTTON_Name;
/* Private variables*******************************************************/
uint8_t Status = 0;
uint32_t startTime = 0;
uint32_t startTimerB3 = 0;
BUTTON_Name buttonB2;
BUTTON_Name buttonB4;
/* Function prototypes*****************************************************/
static void LedBuzz_Init(void);
static void InterruptPA4_Init(void);
static void InterruptPB0_Init(void);
static void InterruptPB3_Init(void);

static void LedControl_SetStatus(GPIO_TypeDef *GPIOx, uint16_t GPIO_PIN_ID, uint8_t Status);
static void Toggled_5times(void);
static void BuzzerControl_SetBeep(GPIO_TypeDef *GPIOx, uint16_t GPIO_PIN, uint32_t num);
static void Blinkled_StatusPower(GPIO_TypeDef *GPIOx1, uint16_t GPIO_PIN_ID1,
								 GPIO_TypeDef *GPIOx2, uint16_t GPIO_PIN_ID2, uint32_t num);
uint32_t CalculatorTime(uint32_t dwTimeInit, uint32_t dwTimeCurrent);
void LedControl_TimPressRealease(void);
void delay_ms(uint32_t ms);
//--------------------------------------------------------------------------------------------
int main(void) {
	buttonB2.Count = 0;
	SystemCoreClockUpdate();
	LedBuzz_Init();
	TimerInit();
	InterruptPA4_Init();
	InterruptPB3_Init();
	InterruptPB0_Init();
	Blinkled_StatusPower(LEDGREEN1_GPIO_PORT, LEDGREEN1_GPIO_PIN,
	LEDGREEN2_GPIO_PORT, LEDGREEN2_GPIO_PIN, 4);
	while (1) {
		Toggled_5times();

		LedControl_TimPressRealease();

	}
}

/**
 * @func   delay_ms
 * @brief  delay milisecon
 * @param  None
 * @retval None
 */
void delay_ms(uint32_t ms) {

	uint32_t startTime = GetMilSecTick(); //Lưu lại thời điểm hiện tại.
	while (CalculatorTime(startTime, GetMilSecTick()) <= ms)
		;

//	Đợi cho đến khi hết khoảng time cài đặt

}

/**
 * @func   LedBuzz_Init
 * @brief  Init Buuzzer and Led
 * @param  None
 * @retval None
 */
static void LedBuzz_Init(void) {
	//Declare type variable GPIO Struct------------------------------------------------
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(
			RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC,
			ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;

//	Port A
	GPIO_InitStructure.GPIO_Pin = LEDGREEN1_GPIO_PIN | LEDGREEN2_GPIO_PIN
			| LEDRED1_GPIO_PIN | LEDBLUE1_GPIO_PIN | LEDBLUE2_GPIO_PIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

//	Port B
	GPIO_InitStructure.GPIO_Pin = LEDRED2_GPIO_PIN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

//  Port C
	GPIO_InitStructure.GPIO_Pin = BUZZER_GPIO_PIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

/**
 * @func   InterruptPA4_Init
 * @brief  Init Interrupt
 * @param  None
 * @retval None
 */
static void InterruptPA4_Init(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

// Enable Clock Port A;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin = BUTTONB3_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

	GPIO_Init(BUTTONB3_GPIO_PORT, &GPIO_InitStructure);

//  Enable Clock Syscfg, Connect EXTI Line 4 to PA4 pin

	RCC_APB2PeriphClockCmd(SYSFG_Clock, ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource4);

//	Configuration Interrupt

	EXTI_InitStructure.EXTI_Line = EXTI_Line4;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

// Configuration NVIC

	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);
}

/**
 * @func   InterruptPB0_Init
 * @brief  Init Interrupt
 * @param  None
 * @retval None
 */
static void InterruptPB0_Init(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

// Enable Clock Port B;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin = BUTTONB4_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

	GPIO_Init(BUTTONB4_GPIO_PORT, &GPIO_InitStructure);

//  Enable Clock Syscfg, Connect EXTI Line 0 to PB0 pin

	RCC_APB2PeriphClockCmd(SYSFG_Clock, ENABLE);

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource0);

//	Configuration Interrupt

	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

// Configuration NVIC

	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);
}

/**
 * @func   InterruptPB3_Init
 * @brief  Init Interrupt
 * @param  None
 * @retval None
 */
static void InterruptPB3_Init(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

// Enable Clock Port C;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin = BUTTONB2_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

	GPIO_Init(BUTTONB2_GPIO_PORT, &GPIO_InitStructure);

//  Enable Clock Syscfg, Connect EXTI Line 3 to PB3 pin

	RCC_APB2PeriphClockCmd(SYSFG_Clock, ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource3);

//	Configuration Interrupt

	EXTI_InitStructure.EXTI_Line = EXTI_Line3;

	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

// Configuration NVIC

	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);
}

/**
 * @func   EXTI4_IRQHandler
 * @brief  Interrupt line 4
 * @param  None
 * @retval None
 */
void EXTI4_IRQHandler(void) {
	if (EXTI_GetFlagStatus(EXTI_Line4) == SET) {
		if (GPIO_ReadInputDataBit(BUTTONB3_GPIO_PORT,BUTTONB3_GPIO_PIN)== GPIO_PIN_RESET) {
			startTimerB3 = GetMilSecTick();	// Lưu thời gian nhấn nút
		}
		else
		{
			Status++;						// Khi nhả nút thì biến tăng
		}
	}
	//xóa cờ ngắt sau khi thực hiện xong chương trình ngắt.
	EXTI_ClearITPendingBit(EXTI_Line4);
}

/**
 * @func   EXTI3_IRQHandler
 * @brief  Interrupt line 3
 * @param  None
 * @retval None
 */
void EXTI3_IRQHandler(void) {
	if (EXTI_GetFlagStatus(EXTI_Line3) == SET) {

		if (GPIO_ReadInputDataBit(BUTTONB2_GPIO_PORT,BUTTONB2_GPIO_PIN)== GPIO_PIN_RESET) {
			buttonB2.State = BUTTON_PRESSED;
			buttonB2.timePress = GetMilSecTick();
			buttonB2.Count++;
		} else {

			buttonB2.timeReleased = GetMilSecTick();
			buttonB2.State = BUTTON_RELEASED;
		}
	}
	//xóa cờ ngắt sau khi thực hiện xong chương trình ngắt.
	EXTI_ClearITPendingBit(EXTI_Line3);
}

/**
 * @func   EXTI0_IRQHandler
 * @brief  Interrupt line 0
 * @param  None
 * @retval None
 */
void EXTI0_IRQHandler(void) {
	if (EXTI_GetFlagStatus(EXTI_Line0) == SET) {
		if (GPIO_ReadInputDataBit(BUTTONB4_GPIO_PORT,BUTTONB4_GPIO_PIN)== GPIO_PIN_RESET) {
			buttonB4.State = BUTTON_PRESSED;
			buttonB4.timePress = GetMilSecTick();
			buttonB4.Count++;
		} else {
			buttonB4.timeReleased = GetMilSecTick();
			buttonB4.State = BUTTON_RELEASED;
		}
	}
	//xóa cờ ngắt sau khi thực hiện xong chương trình ngắt.
	EXTI_ClearITPendingBit(EXTI_Line0);
}

/**
 * @func   LedControl_SetStatus
 * @brief  set status LED
 * @param  None
 * @retval None
 */
static void LedControl_SetStatus(GPIO_TypeDef *GPIOx, uint16_t GPIO_PIN,
		uint8_t Status) {
	// SET bit in BSRR Registers

	if (Status == GPIO_PIN_SET) {
		GPIOx->BSRRL = GPIO_PIN;
	}
	if (Status == GPIO_PIN_RESET) {
		GPIOx->BSRRH = GPIO_PIN;
	}
}

/**
 * @func   BuzzerControl_SetBeep
 * @brief  set status buzzer
 * @param  None
 * @retval None
 */
static void BuzzerControl_SetBeep(GPIO_TypeDef *GPIOx, uint16_t GPIO_PIN,
		uint32_t num) {
	for (uint32_t i = 0; i < num; i++) {
		GPIO_SetBits(GPIOx, GPIO_PIN);
		delay_ms(200);
		GPIO_ResetBits(GPIOx, GPIO_PIN);
		delay_ms(200);
	}
}

/**
 * @func   CalculatorTime
 * @brief  Caculator time button
 * @param  None
 * @retval None
 */
uint32_t CalculatorTime(uint32_t dwTimeInit, uint32_t dwTimeCurrent) {
	uint32_t dwTimeTotal;
	if (dwTimeCurrent >= dwTimeInit) {
		dwTimeTotal = dwTimeCurrent - dwTimeInit;
	} else {
		dwTimeTotal = 0xFFFFFFFFU + dwTimeCurrent - dwTimeInit;
	}
	return dwTimeTotal;

}

/**
 * @func   Toggled_5times
 * @brief  Toggle led number times
 * @param  None
 * @retval None
 */
static void Toggled_5times(void) {
	if (Status == 5) {
		delay_ms(200);
		Blinkled_StatusPower(LEDGREEN1_GPIO_PORT, LEDGREEN1_GPIO_PIN,
		LEDGREEN2_GPIO_PORT, LEDGREEN2_GPIO_PIN, 5);
		BuzzerControl_SetBeep(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN, 2);
		Status = 0;
	}
	else
	{
		if(CalculatorTime(startTimerB3, GetMilSecTick()) > 500) // Kiểm tra khoảng thời gian giữa các lần nhấn nút
		{
			Status = 0;
		}
	}
}

/**
 * @func   Blinkled_StatusPower
 * @brief  Led blink
 * @param  None
 * @retval None
 */
static void Blinkled_StatusPower(GPIO_TypeDef *GPIOx1, uint16_t GPIO_PIN_ID1,
		GPIO_TypeDef *GPIOx2, uint16_t GPIO_PIN_ID2, uint32_t num) {
	for (uint32_t i = 0; i < num; i++) {
		LedControl_SetStatus(GPIOx1, GPIO_PIN_ID1, GPIO_PIN_HIGH);
		LedControl_SetStatus(GPIOx2, GPIO_PIN_ID2, GPIO_PIN_HIGH);
		delay_ms(100);
		LedControl_SetStatus(GPIOx1, GPIO_PIN_ID1, GPIO_PIN_LOW);
		LedControl_SetStatus(GPIOx2, GPIO_PIN_ID2, GPIO_PIN_LOW);
		delay_ms(100);
	}
}

/**
 * @func   LedControl_TimPress
 * @brief  Control led with state button
 * @param  None
 * @retval None
 */

void LedControl_TimPressRealease(void) {
//	BUTTON B2****************************************************************************************************
	if (buttonB2.State == BUTTON_PRESSED) {
		if (CalculatorTime(buttonB2.timePress, GetMilSecTick()) > 500) { // Kiểm tra xem nút có đang được giữ không
			buttonB2.Count = 0;
			LedControl_SetStatus(LEDBLUE2_GPIO_PORT, LEDBLUE2_GPIO_PIN,GPIO_PIN_HIGH);
		}
	}
	if (buttonB2.State == BUTTON_RELEASED) {

		LedControl_SetStatus(LEDBLUE2_GPIO_PORT, LEDBLUE2_GPIO_PIN, GPIO_PIN_LOW);
		if (buttonB2.Count == 1) {
				if (CalculatorTime(buttonB2.timeReleased, GetMilSecTick()) > 400) // Kiểm tra khoảng thời gian giữa 2 lần nhấn nút
					buttonB2.Count = 0;

		}
		if (buttonB2.Count == 2) {
			LedControl_SetStatus(LEDBLUE2_GPIO_PORT, LEDBLUE2_GPIO_PIN, GPIO_PIN_HIGH);
		}
		if (buttonB2.Count >= 3) {
			LedControl_SetStatus(LEDBLUE2_GPIO_PORT, LEDBLUE2_GPIO_PIN, GPIO_PIN_LOW);
			buttonB2.Count = 0;
		}
	}
	if (buttonB2.State == NO_CLICK) {

	}

//BUTTON B4****************************************************************************************************
	if (buttonB4.State == BUTTON_PRESSED) {
		if (CalculatorTime(buttonB4.timePress, GetMilSecTick()) > 500) {			// Kiểm tra xem nút có đang được giữ không
			buttonB4.Count = 0;
			LedControl_SetStatus(LEDRED2_GPIO_PORT, LEDRED2_GPIO_PIN,GPIO_PIN_HIGH);
		}
	}
	if (buttonB4.State == BUTTON_RELEASED) {

		LedControl_SetStatus(LEDRED2_GPIO_PORT, LEDRED2_GPIO_PIN, GPIO_PIN_LOW);
		if (buttonB4.Count == 1) {
				if (CalculatorTime(buttonB4.timeReleased, GetMilSecTick()) > 400)  // Kiểm tra khoảng thời gian giữa 2 lần nhấn nút
					buttonB4.Count = 0;
		}
		if (buttonB4.Count == 2) {
			LedControl_SetStatus(LEDRED2_GPIO_PORT, LEDRED2_GPIO_PIN, GPIO_PIN_HIGH);
		}
		if (buttonB4.Count >= 3) {
			LedControl_SetStatus(LEDRED2_GPIO_PORT, LEDRED2_GPIO_PIN, GPIO_PIN_LOW);
			buttonB4.Count = 0;
		}
	}
	if (buttonB4.State == NO_CLICK) {

	}
}
