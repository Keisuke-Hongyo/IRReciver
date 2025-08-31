#include "ch32v003fun.h"

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

#define IR_INPUT_PIN	PA2
#define CHK_OUT_PIN		PC1

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

// Function
RcvCode NecIrChek(struct nec_ir_data *);
RcvCode SonyIrChek(struct  sony_ir_data *);
RcvCode AehaIrChek(unsigned char *);
void nec_ir_recive();
void aeha_ir_recive();
void sony_ir_recive();

