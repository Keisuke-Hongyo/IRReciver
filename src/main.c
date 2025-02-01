#include "ch32v003fun.h"
#include <stdio.h>
#include <stdlib.h>

#define AEHA_T		 	 425
#define AEHA_MARGIN		  75
#define AEHA_BIT_SIZE	  48	
#define AEHA_DATA_SIZE	   6

#define NEC_T		 	 562
#define NEC_MARGIN		  40
#define NEC_BIT_SIZE	  32	
#define NEC_DATA_SIZE	   4

#define SONY_T		 	 600
#define SONY_MARGIN		  40
#define SONY_BIT_SIZE	 20
#define SONY_DATA_SIZE	  4	

// リモコン用
typedef enum {
	recv_none,
	recv_ok,
	recv_err,
	recv_repeat
} RcvCode;

// リモコンフォーマットタイプ
typedef enum {
	NEC,
	AEHA,
	SONY
} IrFormat;

// Sony用赤外線リモコン受信データ構造体
struct nec_ir_data {
	unsigned char	customercode;		// カスタマコード
	unsigned short	data;				// データ
};

// Sony用赤外線リモコン受信データ構造体
struct sony_ir_data {
	unsigned char	data;
	unsigned short	address;
};

// 初期化関数初期化関数
void init()
{
	// Enable GPIOA,TIM2
	RCC->APB2PCENR |= (RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC );
	RCC->APB1PCENR |= (RCC_APB1Periph_TIM2);

	// PA1 is 10MHz
	funPinMode(PA2, FUN_OUTPUT);
	funPinMode(PC1, FUN_INPUT);

	funDigitalWrite(PA2, FUN_HIGH);

	//TIM2 フリーカウンタ
    TIM2->PSC = 48 - 1;			//TIM2プリスケーラ　 1count/us
    TIM2->CTLR1 |= TIM_CEN;		//TIM2カウント有効(CEN)
}

// NECフォーマット
RcvCode NecIrChek(struct nec_ir_data *irData)
{
	unsigned int t;
	unsigned char j;
	unsigned char bit;
	unsigned char data[NEC_BIT_SIZE];
	unsigned char rdata[NEC_DATA_SIZE]; 
	
	/* リーダ部チェック */
	if(funDigitalRead(PC1) == FUN_LOW) {
			return recv_none;
	} 

	/* リーダ部確認 */
	/* High部分を確認 */
	TIM2->CNT = 0;
	while(funDigitalRead(PC1) == FUN_HIGH);
	t = TIM2->CNT;
	TIM2->CNT = 0;
	if((t < (16 * (NEC_T - NEC_MARGIN))) || (t >  (16 * (NEC_T + NEC_MARGIN)))){
		return recv_err;
	}
	
	/* LOW部分を確認 */
	while(funDigitalRead(PC1) == FUN_LOW);
	t = TIM2->CNT;
	TIM2->CNT = 0;
	if((t >= (8 * (AEHA_T - AEHA_MARGIN))) ) {
		if (t >=  (8 * (AEHA_T - AEHA_MARGIN)) && (t <= (8 * (AEHA_T + AEHA_MARGIN)))) {
			return recv_repeat;
		}
	} else {
		return recv_err;
	}
	
	//データ格納
	for(bit = 0; bit < NEC_BIT_SIZE ;bit++){
		while(funDigitalRead(PC1) == FUN_HIGH);
		while(funDigitalRead(PC1) == FUN_LOW);
		t = TIM2->CNT;
		TIM2->CNT = 0;
		if((t < (2 * (NEC_T - NEC_MARGIN))) && (t > (4 * (NEC_T + NEC_MARGIN)))){
			return recv_err;
		} else {
			if((t >= (2 * (NEC_T - AEHA_MARGIN))) && (t <=  (2 * (NEC_T + AEHA_MARGIN)))){
				data[bit] = 0x00;
			} else {
				data[bit] = 0x01;
			}
		}
	}
	
	// データ復元
	for(j = 0; j < NEC_DATA_SIZE; j++){
		rdata[j] = 0x00;
		for(bit = 0; bit < 8; bit++){
			if(data[bit+(j * 8)]) {
				rdata[j] |= (1<<bit);
			} else {
				rdata[j] &= ~(1<<bit);
			}
		}
	}	

	if((rdata[0] & rdata[1]) == 0x00) {
		irData->customercode = rdata[0];
	}

	if((rdata[2] & rdata[3]) == 0x00) {
		irData->data = rdata[2];
	}

	return recv_ok;
}

// Sonyフォーマット
RcvCode SonyIrChek(struct  sony_ir_data *irData)
{
	unsigned int t;
	unsigned char i;
	unsigned char bit;
	unsigned char data[SONY_BIT_SIZE];

	/* リーダ部チェック */
	if(funDigitalRead(PC1) == FUN_LOW) {
			return recv_none;
	} 

	/* リーダ部確認 */
	/* High部分を確認 */
	TIM2->CNT = 0;
	while(funDigitalRead(PC1) == FUN_HIGH);
	t = TIM2->CNT;
	TIM2->CNT = 0;
	if((t < (4 * (SONY_T - SONY_MARGIN))) || (t >  (4 * (SONY_T + SONY_MARGIN)))){
		return recv_err;
	}

	//データ格納
	for(bit = 0; bit < SONY_BIT_SIZE ;bit++){
		while(funDigitalRead(PC1) == FUN_LOW);
		if(TIM2->CNT > 900) break;
		while(funDigitalRead(PC1) == FUN_HIGH);
		t = TIM2->CNT;
		TIM2->CNT = 0;
		if((t >= (2 * (SONY_T - SONY_MARGIN))) && (t <=  (2 * (SONY_T + SONY_MARGIN)))){
			data[bit] = 0x00;
		} else {
			data[bit] = 0x01;
		}
	}
	//　データ復元
	// データ部
	irData->data = 0x00;
	for(i = 0;i < 7; i++){
		if(data[i]) {
			irData->data |= (1<<i);
		} else {
			irData->data &= ~(1<<i);
		}
	}
	// アドレス部
	irData->address = 0x0000;
	for(i = 7;i < bit; i++){
		if(data[i]) {
			irData->address |= (1<<(i-7));
		} else {
			irData->address &= ~(1<<(i-7));
		}
	}
	
	return recv_ok;
}

// 家電協会フォーマット
RcvCode AehaIrChek(unsigned char *irData)
{
	unsigned int t;
	unsigned char j;
	unsigned char bit;
	unsigned char data[AEHA_BIT_SIZE];
	
	/* リーダ部チェック */
	if(funDigitalRead(PC1) == FUN_LOW) {
			return recv_none;
	} 

	/* リーダ部確認 */
	/* High部分を確認 */
	TIM2->CNT = 0;
	while(funDigitalRead(PC1) == FUN_HIGH);
	t = TIM2->CNT;
	TIM2->CNT = 0;
	if((t < (8 * (AEHA_T - AEHA_MARGIN))) || (t >  (8 * (AEHA_T + AEHA_MARGIN)))){
		return recv_err;
	}
	
	/* LOW部分を確認 */
	while(funDigitalRead(PC1) == FUN_LOW);
	t = TIM2->CNT;
	TIM2->CNT = 0;
	if((t >= (4 * (AEHA_T - AEHA_MARGIN))) ) {
		if (t >=  (8 * (AEHA_T - AEHA_MARGIN)) && (t <= (8 * (AEHA_T + AEHA_MARGIN)))) {
			return recv_repeat;
		}
	} else {
		return recv_err;
	}
	
	//データ格納
	for(bit = 0; bit < AEHA_BIT_SIZE ;bit++){
		while(funDigitalRead(PC1) == FUN_HIGH);
		while(funDigitalRead(PC1) == FUN_LOW);
		t = TIM2->CNT;
		TIM2->CNT = 0;
		if((t < (2 * (AEHA_T - AEHA_MARGIN))) && (t > (4 * (AEHA_T + AEHA_MARGIN)))){
			return recv_err;
		} else {
			if((t >= (2 * (AEHA_T - AEHA_MARGIN))) && (t <=  (2 * (AEHA_T + AEHA_MARGIN)))){
				data[bit] = 0x00;
			} else {
				data[bit] = 0x01;
			}
		}
	}
	
	// データ復元
	for(j = 0; j < AEHA_DATA_SIZE; j++){
		*irData = 0x00;
		for(bit = 0; bit < 8; bit++){
			if(data[bit+(j * 8)]) {
				*irData |= (1<<bit);
			} else {
				*irData &= ~(1<<bit);
			}
		}
		irData++;
	}	
	
	return recv_ok;
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
			funDigitalWrite(PA2, FUN_LOW);
		} else if (irdata.data == 0x78) {
			funDigitalWrite(PA2, FUN_HIGH);
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
			funDigitalWrite(PA2, FUN_LOW);
		} else if (ahea_data[4] == 0x02) {
			funDigitalWrite(PA2, FUN_HIGH);
		}
}
// AEHA 受信処理プログラム
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