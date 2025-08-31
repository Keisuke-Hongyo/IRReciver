#include "ch32v003fun.h"
#include <stdio.h>
#include <stdlib.h>
#include "irrecv.h"

// 初期化関数初期化関数
void init()
{
	// Enable GPIOA,TIM2
	RCC->APB2PCENR |= (RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC );
	RCC->APB1PCENR |= (RCC_APB1Periph_TIM2);

	// PA1 is 10MHz
	funPinMode(IR_INPUT_PIN, FUN_INPUT);
	funPinMode(CHK_OUT_PIN, FUN_OUTPUT);

	funDigitalWrite(CHK_OUT_PIN, FUN_HIGH);

	//TIM2 フリーカウンタ
    TIM2->PSC = 48 - 1;			//TIM2プリスケーラ　 1count/us
    TIM2->CTLR1 |= TIM_CEN;		//TIM2カウント有効(CEN)
}

// NEC 受信処理プログラム
void nec_ir_recive()
{
	RcvCode pat;
	//unsigned char nec_data[NEC_DATA_SIZE];
	//unsigned char 	i;
	struct  nec_ir_data irdata;

	pat = NecIrChek(&irdata);
	if(pat == recv_ok) {
		printf("RCV OK!\n");
		printf("Recive Data: Customer Code = 0x%02x Data = 0x%02x \n"
							,irdata.customercode,irdata.data);
		/* for(i=0;i<NEC_DATA_SIZE;i++){
			printf("0x%02x ",nec_data[i]);
		} 
		printf("\n");*/
		if (irdata.data == 0xf8) {
			funDigitalWrite(CHK_OUT_PIN, FUN_LOW);
		} else if (irdata.data == 0x78) {
			funDigitalWrite(CHK_OUT_PIN, FUN_HIGH);
		}
	} else if (pat == recv_repeat) {
		printf("RCV REPEAT \n");
	}
} 
// AEHA 受信処理プログラム
void aeha_ir_recive()
{
	RcvCode pat;
	unsigned char	ahea_data[AEHA_DATA_SIZE];
	unsigned char i;

	pat = AehaIrChek(ahea_data);
		if(pat == recv_ok) {
			printf("RCV OK!\n");
			printf("Recive Data: ");
			for(i=0;i<AEHA_DATA_SIZE;i++){
				printf("0x%02x ",ahea_data[i]);
			}
			printf("\n");
		} else if (pat == recv_repeat) {
			printf("RCV REPEAT \n");
		} 
		if (ahea_data[4] == 0x01) {
			funDigitalWrite(CHK_OUT_PIN, FUN_LOW);
		} else if (ahea_data[4] == 0x02) {
			funDigitalWrite(CHK_OUT_PIN, FUN_HIGH);
		}
}
// SONY 受信処理プログラム
void sony_ir_recive()
{
	RcvCode pat;
	struct  sony_ir_data irdata;
	pat = SonyIrChek(&irdata);
	if(pat == recv_ok) {
		printf("RCV OK!\n");
		printf("Data     Code = %0x\n",irdata.data);
		printf("Address  Code = %0x\n",irdata.address);
	} /* else {
		printf("RCV Error... \n");
	} */
}

// main Program 
int main(void) { 
	
	IrFormat format;

	// システム初期化 -> 必ずいる！
	SystemInit();

	printf("systemclock=%d \n",FUNCONF_SYSTEM_CORE_CLOCK);

  	// SysTick初期設定(1ms割込設定)
	init();

	printf("IR RECIVER TEST\n");
	
	format = NEC;
	
	while (1) {
		
		switch (format) {
			case NEC: 	nec_ir_recive(); break;
			case AEHA:	aeha_ir_recive(); break;
			case SONY:	sony_ir_recive(); break;
			
			default:	break;
		
		}
		
	}
}