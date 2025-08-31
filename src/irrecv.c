#include "irrecv.h"

// NECフォーマット
RcvCode NecIrChek(struct nec_ir_data *irData)
{
	unsigned int t;
	unsigned char j;
	unsigned char bit;
	unsigned char data[NEC_BIT_SIZE];
	unsigned char rdata[NEC_DATA_SIZE]; 
	
	/* リーダ部チェック */
	if(funDigitalRead(IR_INPUT_PIN) == FUN_LOW) {
			return recv_none;
	} 

	/* リーダ部確認 */
	/* High部分を確認 */
	TIM2->CNT = 0;
	while(funDigitalRead(IR_INPUT_PIN) == FUN_HIGH);
	t = TIM2->CNT;
	TIM2->CNT = 0;
	if((t < (16 * (NEC_T - NEC_MARGIN))) || (t >  (16 * (NEC_T + NEC_MARGIN)))){
		return recv_err;
	}
	
	/* LOW部分を確認 */
	while(funDigitalRead(IR_INPUT_PIN) == FUN_LOW);
	t = TIM2->CNT;
	TIM2->CNT = 0;
	if((t >= (8 * (NEC_T - NEC_MARGIN))) ) {
		if (t >=  (4 * (NEC_T - NEC_MARGIN)) && (t <= (4 * (NEC_T + NEC_MARGIN)))) {
			return recv_repeat;
		}
	} else {
		return recv_err;
	}
	
	//データ格納
	for(bit = 0; bit < NEC_BIT_SIZE ;bit++){
		while(funDigitalRead(IR_INPUT_PIN) == FUN_HIGH);
		while(funDigitalRead(IR_INPUT_PIN) == FUN_LOW);
		t = TIM2->CNT;
		TIM2->CNT = 0;
		if((t < (2 * (NEC_T - NEC_MARGIN))) && (t > (4 * (NEC_T + NEC_MARGIN)))){
			return recv_err;
		} else {
			if((t >= (2 * (NEC_T - NEC_MARGIN))) && (t <=  (2 * (NEC_T + NEC_MARGIN)))){
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
	if(funDigitalRead(IR_INPUT_PIN) == FUN_LOW) {
			return recv_none;
	} 

	/* リーダ部確認 */
	/* High部分を確認 */
	TIM2->CNT = 0;
	while(funDigitalRead(IR_INPUT_PIN) == FUN_HIGH);
	t = TIM2->CNT;
	TIM2->CNT = 0;
	if((t < (4 * (SONY_T - SONY_MARGIN))) || (t >  (4 * (SONY_T + SONY_MARGIN)))){
		return recv_err;
	}

	//データ格納
	for(bit = 0; bit < SONY_BIT_SIZE ;bit++){
		while(funDigitalRead(IR_INPUT_PIN) == FUN_LOW);
		if(TIM2->CNT > 900) break;
		while(funDigitalRead(IR_INPUT_PIN) == FUN_HIGH);
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
	if(funDigitalRead(IR_INPUT_PIN) == FUN_LOW) {
			return recv_none;
	} 

	/* リーダ部確認 */
	/* High部分を確認 */
	TIM2->CNT = 0;
	while(funDigitalRead(IR_INPUT_PIN) == FUN_HIGH);
	t = TIM2->CNT;
	TIM2->CNT = 0;
	if((t < (8 * (AEHA_T - AEHA_MARGIN))) || (t >  (8 * (AEHA_T + AEHA_MARGIN)))){
		return recv_err;
	}
	
	/* LOW部分を確認 */
	while(funDigitalRead(IR_INPUT_PIN) == FUN_LOW);
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
		while(funDigitalRead(IR_INPUT_PIN) == FUN_HIGH);
		while(funDigitalRead(IR_INPUT_PIN) == FUN_LOW);
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

