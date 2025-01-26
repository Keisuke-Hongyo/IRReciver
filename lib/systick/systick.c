#include "ch32v003fun.h"
#include <stdio.h>
#include "systick.h"


void systick_init()
{
	// 設定初期化
	SysTick->CTLR = 0x0000;
	

	// 周期カウンタ０クリア
	SysTick->CNT = 0x00000000;
	
    // SysTick設定
	SysTick->CTLR |= SYSTICK_CTLR_STE   |  // カウント許可
	                 SYSTICK_CTLR_STCLK ;  // カウント周期 HCLK/1 に設定
	
}

/*
 * SysTick ISR - must be lightweight to prevent the CPU from bogging down.
 * Increments Compare Register and systick_millis when triggered (every 1ms)
 * NOTE: the `__attribute__((interrupt))` attribute is very important
 */
void SysTick_Handler(void) __attribute__((interrupt));
void SysTick_Handler(void)
{
}