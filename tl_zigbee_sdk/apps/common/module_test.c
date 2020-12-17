/********************************************************************************************************
 * @file     module_test.c
 *
 * @brief    Modules function test
 *
 * @author
 * @date     Feb. 1, 2017
 *
 * @par      Copyright (c) 2016, Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *
 *			 The information contained herein is confidential and proprietary property of Telink
 * 		     Semiconductor (Shanghai) Co., Ltd. and is available under the terms
 *			 of Commercial License Agreement between Telink Semiconductor (Shanghai)
 *			 Co., Ltd. and the licensee in separate contract or the terms described here-in.
 *           This heading MUST NOT be removed from this file.
 *
 * 			 Licensees are granted free, non-transferable use of the information in this
 *			 file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided.
 *
 *******************************************************************************************************/
#include "zb_common.h"


#define MODULE_TEST_ADC			0
#define MODULE_TEST_UART		0

#if MODULE_TEST_UART

#if 0
//8278
//#define UART_PIN_CFG				uart_gpio_set(UART_TX_PD0, UART_RX_PD6);// uart tx/rx pin set
//8258 48 pin dongle
#define UART_TX_PIN         		GPIO_PD7
#define UART_RX_PIN         		GPIO_PA0
#define UART_PIN_CFG				uart_gpio_set(UART_TX_PD7, UART_RX_PA0);// uart tx/rx pin set
#else
#define UART_PIN_CFG				do{											\
										gpio_set_func(GPIO_PC2, AS_UART);		\
										gpio_set_func(GPIO_PC3, AS_UART);		\
										gpio_set_output_en(GPIO_PC2, 1);		\
										gpio_set_input_en(GPIO_PC2, 0);			\
										gpio_set_output_en(GPIO_PC2, 0);		\
										gpio_set_input_en(GPIO_PC3, 1);			\
										UART_GPIO_CFG_PC2_PC3();				\
									}while(0)
#endif

typedef struct{
	u32 dataLen;
	u8 dataPayload[1];
}uart_rxData_t;
__attribute__((aligned(4))) u8 moduleTest_uartTxBuf[4] = {0};
__attribute__((aligned(4))) u8 moduleTest_uartRxBuf[32] = {0};
volatile u8  T_uartPktSentSeqNo = 0;
volatile u32 T_uartPktRecvSeqNo = 0;
volatile u32 T_uartPktRecvLen = 0;
volatile u32 T_uartPktSentExcept = 0;
#include "../../proj/drivers/drv_uart.h"

void module_test_uartRcvHandler(void){
	/*
	 * the format of the uart rx data: length(4 Bytes) + payload
	 *
	 * */
	uart_rxData_t *rxData = (uart_rxData_t *)moduleTest_uartRxBuf;
	T_uartPktRecvLen = rxData->dataLen;
	T_uartPktRecvSeqNo = rxData->dataPayload[0];

#if 0//only for 8258/8278
	if(T_uartPktRecvSeqNo == 0xBB){
		drv_adc_enable(1);
	}else{
		drv_adc_enable(0);
	}
#endif
}


void moduleTest_forUart(void){
	UART_PIN_CFG;

	drv_uart_init(115200, moduleTest_uartRxBuf, sizeof(moduleTest_uartRxBuf)/sizeof(u8), module_test_uartRcvHandler);

	drv_adc_init();
#if 0//only for 8258
	drv_adc_mode_pin_set(Drv_ADC_VBAT_MODE, GPIO_PC5);
#endif

#if 0//only for 8278
	//drv_adc_mode_pin_set(Drv_ADC_BASE_MODE, GPIO_PB3);
	drv_adc_mode_pin_set(Drv_ADC_VBAT_MODE, GPIO_PB3);
#endif

	irq_enable();

	for(int i = 0; i < sizeof(moduleTest_uartTxBuf)/sizeof(u8); i++){
		moduleTest_uartTxBuf[i] = i;
	}

	while(1)
	{
		if(T_uartPktRecvSeqNo == 0xAA){
			moduleTest_uartTxBuf[0] = T_uartPktSentSeqNo++;
			if(uart_tx_start(moduleTest_uartTxBuf, sizeof(moduleTest_uartTxBuf)/sizeof(u8)) == 1){
				WaitMs(1000);
			}else{
				T_uartPktSentExcept++;
				while(1);
			}
		}else if(T_uartPktRecvSeqNo == 0xBB){
			u16 vBat = drv_get_adc_data();

			moduleTest_uartTxBuf[0] = HI_UINT16(vBat);
			moduleTest_uartTxBuf[1] = LO_UINT16(vBat);

			if(uart_tx_start(moduleTest_uartTxBuf, 2) == 1){
				WaitMs(1000);
			}else{
				T_uartPktSentExcept++;
				while(1);
			}
		}else if(T_uartPktRecvSeqNo == 0xCC){
			u16 random = zb_random();

			moduleTest_uartTxBuf[0] = HI_UINT16(random);
			moduleTest_uartTxBuf[1] = LO_UINT16(random);

			if(uart_tx_start(moduleTest_uartTxBuf, 2) == 1){
				WaitMs(1000);
			}else{
				T_uartPktSentExcept++;
				while(1);
			}
		}
	}
}

#endif

#define MODULE_TEST_NV		0

#if MODULE_TEST_NV
#include "../../zigbee/zbapi/zb_api.h"
volatile u8 T_nwkFrmCntError = 0;
volatile u8 T_nwkFrmCntReadErr = 0;
volatile u32 T_frameCnt = 0;
volatile u32 T_readFrm = 0;
volatile u8 T_bufCheck[256] = {0};
void moduleTest_NV(void){
	extern zb_info_t g_zbInfo;
	u8 *pData = (u8*)&g_zbInfo;

	while(0){
		T_readFrm = 0;
		nv_nwkFrameCountSaveToFlash(T_frameCnt);
		T_nwkFrmCntReadErr = nv_nwkFrameCountFromFlash((u32*)&T_readFrm);

		if(T_readFrm != T_frameCnt){
			T_nwkFrmCntError = 1;
			while(1);
		}
		T_frameCnt += 1;
	}

	while(1){
		pData = (u8*)&g_zbInfo;
		for(s32 i = 0; i < sizeof(zb_info_t); i++){
			pData[i] = (i + T_frameCnt);
		}

		nv_flashWriteNew(1, NV_MODULE_ZB_INFO, NV_ITEM_ZB_INFO, sizeof(zb_info_t), (u8*)&g_zbInfo);
		nv_flashReadNew(1, NV_MODULE_ZB_INFO, NV_ITEM_ZB_INFO, sizeof(zb_info_t), (u8*)&T_bufCheck);
		for(s32 i = 0; i < sizeof(zb_info_t); i++){
			if(pData[i] != T_bufCheck[i]){
				while(1);
			}
		}

		nv_flashWriteNew(1, NV_MODULE_ZB_INFO, 2, sizeof(nwk_nib_t), (u8*)&g_zbInfo.nwkNib);
		nv_flashReadNew(1, NV_MODULE_ZB_INFO, 2, sizeof(nwk_nib_t), (u8*)&T_bufCheck);
		pData = (u8*)&g_zbInfo.nwkNib;
		for(s32 i = 0; i < sizeof(nwk_nib_t); i++){
			if(pData[i] != T_bufCheck[i]){
				while(1);
			}
		}
		T_frameCnt += 1;
	}
}
#endif

#define MODULE_TEST_PM 0

#if MODULE_TEST_PM

#if (__PROJECT_TL_SWITCH__ || __PROJECT_TL_SWITCH_8258__ || __PROJECT_TL_SWITCH_8278__)
#define PAD_WAKUPUP_TEST		1
#define TIMER_WAKUPUP_TEST		1
#define DEEP_SLEEP_TEST			1

#include "../../proj/drivers/drv_pm.h"
#include "./pm_interface.h"
extern pm_pinCfg_t g_switchPmCfg[];
extern void light_on(void);
extern void light_off(void);

unsigned char  txPktForPm[48] __attribute__ ((aligned (4))) = {0x09, 0x00, 0x00, 0x00, 0x0a, 0x03, 0x08, 0xd0,  0xff, 0xff, 0xff, 0xff, 0x07, 0x00};//{0x14,0x00,0x00,0x00,0x15, 0x00, 0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff, 0xee, 0xdd, 0xcc, 0xbb};

void moduleTest_PM(void){
	platform_wakeup_e wakeupSrc = 0;
	platform_mode_e mode = 0;

	irq_disable();

	ZB_RADIO_INIT();
	ZB_RADIO_TRX_SWITCH(RF_MODE_TX, 50);

#if PAD_WAKUPUP_TEST
	s32 pinNum = 2;
	for(s32 i = 0; i < pinNum; i++){
		platform_wakeup_pad_cfg(g_switchPmCfg[i].wakeupPin, g_switchPmCfg[i].wakeLevel, 1);
	}
	wakeupSrc |= PLATFORM_WAKEUP_PAD;
#endif

	u32 interval = 0;
#if TIMER_WAKUPUP_TEST
	interval = 1000;
	wakeupSrc |= PLATFORM_WAKEUP_TIMER;
#endif

#if DEEP_SLEEP_TEST
#if (__PROJECT_TL_SWITCH__)
	mode = PLATFORM_MODE_DEEPSLEEP;
#elif (__PROJECT_TL_SWITCH_8258__ || __PROJECT_TL_SWITCH_8278__)
	mode = PLATFORM_MODE_DEEP_WITH_RETENTION;
#endif
#else
	mode = PLATFORM_MODE_SUSPEND;
#endif

	if(wakeupSrc == (PLATFORM_WAKEUP_PAD|PLATFORM_WAKEUP_TIMER)){
		interval = 10000;
	}

	WaitUs(1000*1000);

	while(1){
#if DEEP_SLEEP_TEST
		ZB_RADIO_TX_START(txPktForPm);

		for(u32 i = 0; i < 2; i++){
			light_on();
			WaitUs(100*1000);
			light_off();
			WaitUs(100*1000);
		}
#endif

		platform_lowpower_enter(mode, wakeupSrc, interval);
		//tx_packet[8]++;

		ZB_RADIO_TRX_SWITCH(RF_MODE_TX, 50);
		ZB_RADIO_TX_START(txPktForPm);

		for(u32 i = 0; i < 2; i++){
			light_on();
			WaitUs(100*1000);
			light_off();
			WaitUs(100*1000);
		}
	}
}
#else
	#error please compile the project of "sampleSwith"
#endif
#endif

#define MODULE_TEST_RF 0

#if MODULE_TEST_RF

#define TX 1

unsigned char  rx_packet[128]  __attribute__ ((aligned (4)));

unsigned char  tx_packet[48] __attribute__ ((aligned (4))) = {0x09, 0x00, 0x00, 0x00, 0x0a, 0x03, 0x08, 0xd0,  0xff, 0xff, 0xff, 0xff, 0x07, 0x00};//{0x14,0x00,0x00,0x00,0x15, 0x00, 0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff, 0xee, 0xdd, 0xcc, 0xbb};

unsigned int rx_cnt;
void moduleTest_RF(){
    
	//RF_Init(RF_MODE_ZIGBEE_250K);///cystal is 12Mhz.

	//RF_PowerLevelSet(RF_POWER_7P9dBm);
	//RF_RxBufferSet(rx_packet,128,0);

	//rf_set_power_level_index(0) ;//RF_POWER_P10p46dBm, RF_POWER_P5p65dBm, RF_POWER_P3p01dBm);

    #if TX
    	while(1)
    	{
    		ZB_RADIO_TRX_SWITCH(RF_MODE_TX, 50);
			WaitUs(150);
			//tx_packet[8]++;
			ZB_RADIO_TX_START(tx_packet);
			WaitMs(50);
    	}
	#else
    	ZB_RADIO_TRX_SWITCH(RF_MODE_RX, 50);
    	while(1);
	#endif
}
#endif

#define MODULE_TEST_MMOHASH 	0
#if MODULE_TEST_MMOHASH
u8 T_DBG_installCode[18] = {0x83,0xfe,0xd3,0x40,0x7a,0x93,0x97,0x23,0xa5,0xc6,0x39,0xb2,0x69,0x16,0xd5,0x05};
u16 T_DBG_insCodeCRC;
u8 T_DBG_hashOut[16] = {0};
void moudleTest_hash(void)
{
	T_DBG_insCodeCRC = tl_bdbInstallCodeCRC16(T_DBG_installCode, 16);
	T_DBG_installCode[16] = (u8)(T_DBG_insCodeCRC & 0xff);
	T_DBG_installCode[17] = (u8)(T_DBG_insCodeCRC >> 8);
	ss_mmoHash(T_DBG_installCode, 18, T_DBG_hashOut);
	while(1);
}
#endif

#define MODULE_TEST_CCM	0
#if MODULE_TEST_CCM
//u8 aes_ccmDecTran(u8 micLen, u8 *key, u8 *iv, u8 *mStr, u16 mStrLen, u8 *aStr, u8 aStrLen, u8 *mic)
#if 0
u8 key[] = {0xC0, 0xC1 , 0xC2 , 0xC3 , 0xC4 , 0xC5 , 0xC6 , 0xC7 , 0xC8 , 0xC9 , 0xCa, 0xCb , 0xCc , 0xCd , 0xCe , 0xCf};
u8 nonce[] = {0x21, 0x43, 0x65, 0x87, 0x21, 0x43, 0x65, 0x87, 0x02, 0x00, 0x00, 0x00, 0x05};//0x02, 0x00, 0x00, 0x00, 0x05};
u8 astring[] = {0x8c, 0x10, 0x21, 0x43, 0x65, 0x87, 0x02, 0x00, 0x00, 0x00, 0x20}; //0x02, 0x00, 0x00, 0x00, 0x20};
u8 astrLen = 11;//10;//11;
u8 mstring[] = {0x83};//, 0x20}; //0x02, 0x00, 0x00, 0x00, 0x20};
u8 mstrLen = 0;//1;//11;
u8 mic[] = {0xCF, 0x78, 0x7E, 0x72};// {0xCA, 0x43, 0x24, 0xDD};//{0xF6, 0x36, 0x78, 0x9E}; //{0xCF, 0x78, 0x7E, 0x72};
#else
u8 key[] = {0xC0, 0xC1 , 0xC2 , 0xC3 , 0xC4 , 0xC5 , 0xC6 , 0xC7 , 0xC8 , 0xC9 , 0xCa, 0xCb , 0xCc , 0xCd , 0xCe , 0xCf};
u8 nonce[] = {0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x99, 0x00, 0x00, 0x00, 0x05};
u8 astring[] = {0x8c, 0x12, 0x01, 0x00, 0x00, 0x00, 0x22};
u8 astrLen = 7;
u8 mstring[] = {0x67};
u8 mstrLen = 0;
u8 mic[] = {0x51, 0x3B, 0x81,0x3F}; // {0x1C, 0xF9, 0x06, 0xC0};  //{0xf3, 0xeb, 0x95,0xb1}; //{0x1C, 0xF9, 0x06, 0xC0};
#endif


void moduleTest_ccm(void){
	//aes_ccmAuthTran(4, key, nonce, NULL, mstrLen, astring, astrLen, mic);
	//aes_ccmEncTran(4, key, nonce, NULL, mstrLen, astring, astrLen, mic);

	//while(1);

	aes_ccmDecTran(4, key, nonce, mstring, mstrLen, astring, astrLen, mic);
	//nonce[12] = 0xc5;
	u8 ret = aes_ccmDecAuthTran(4, key, nonce, mstring, mstrLen, astring, astrLen, mic);

	while(1);
}
#endif

#if MODULE_TEST_ADC

#define ADC_SAMPLE_TIME		16
s16 BUFF_ADC_SAMPLE[ADC_SAMPLE_TIME] = {0};
volatile s32 Cvoltage_value = 0;
volatile s32 Ctemp_value = 0;

void moduleTest_adc(u8 mode){
	s32 sum = 0;

	drv_adc_init();

#if defined(MCU_CORE_8258) || defined(MCU_CORE_8278)
	drv_adc_mode_pin_set(Drv_ADC_BASE_MODE, GPIO_PB3);
	drv_adc_enable(1);
#elif MCU_CORE_826x
	drv_ADC_ParamSetting(Drv_ADC_MISC_CHN,Drv_SINGLE_ENDED_MODE,B4,B4,S_3,RV_AVDD,RES14);
#else

#endif

	while(1)
	{
#if defined(MCU_CORE_8258) || defined(MCU_CORE_8278)
		Cvoltage_value = drv_get_adc_data();
#elif MCU_CORE_826x
		for(s32 i=0;i<ADC_SAMPLE_TIME;i++){
			BUFF_ADC_SAMPLE[i] = drv_get_adc_data();
		}
		for(s32 i=0;i<ADC_SAMPLE_TIME;i++){ //-2; remove max value and min value.
			sum += BUFF_ADC_SAMPLE[i];
		}
		sum /= (ADC_SAMPLE_TIME);//get the average value

		Cvoltage_value = 3300*(sum - 128)/(16384 - 256);//ref_vol=RV_AVDD=3300mv,(mv) = Vref*(x-128)/(2^14-2^8)
#else
#endif
	}
}
#endif

#define MODULE_TEST_TIMER 0

#if MODULE_TEST_TIMER

volatile u8 T_timerCnt = 0;
volatile u8 T_timerDone = 0;
s32 moduleTestTimerCb(void *arg){
	T_timerCnt++;
	T_timerDone = 1;

	gpio_toggle(GPIO_PA3);

	return 0;
}

void moduleTest_timer(void){
	gpio_init();

	WaitMs(2000);

	gpio_set_func(GPIO_PA3 ,AS_GPIO);
	gpio_set_output_en(GPIO_PA3, 1); 		//enable output
	gpio_set_input_en(GPIO_PA3 ,0);			//disable input
	gpio_write(GPIO_PA3, 1);              	//LED On

	hwTmr_init(TIMER_IDX_1, TIMER_MODE_SCLK);

	hwTmr_set(TIMER_IDX_1, 500*1000, moduleTestTimerCb, NULL);

	T_timerDone = 0;

	while(1){

	}
}
#endif


#define MODULE_TEST_PWM		0
#if MODULE_TEST_PWM

#if __PROJECT_TL_DIMMABLE_LIGHT__
#include "../../proj/drivers/drv_pwm.h"
void moduleTest_pwm(void){
	PWM_W_CHANNEL_SET();
	drv_pwm_init();

	u8 duty = 5;
	u8 fullDuty = 100;
	u32 max_tick = CLOCK_SYS_CLOCK_HZ / 1000;
	u32 cmp_tick = (duty * max_tick) / fullDuty;

	drv_pwm_cfg(PWM_W_CHANNEL, (u16)cmp_tick, (u16)max_tick);
	drv_pwm_start(PWM_W_CHANNEL);

	while(1){
		if(cmp_tick <= max_tick){
			drv_pwm_cfg(PWM_W_CHANNEL, (u16)cmp_tick, (u16)max_tick);
			cmp_tick += 2400;
			WaitMs(500);
		}else{
			cmp_tick = 0;
		}
	}
}
#else
	#error please compile the project of "sampleLight"
#endif
#endif

#define MODULE_TEST_SPI 0

#if MODULE_TEST_SPI
#include "../../proj/drivers/drv_spi.h"

/**
 *  @brief  SPI interface
 *  -----------------------------------
 *   	 |	SDO  SDI  SCK  CN |
 *  826x |	A2   A3   A4   A5 |
 *  	 |	B5   B6   B7   B4 |
 *  -----------------------------------
 *  	 |	SDO  SDI  SCK  CN |
 *  8258 |	A2   A3   A4   D6 |
 *  	 |	B7   B6   D7   D2 |
 *  -----------------------------------
 *  	 |	SDO  SDI  SCK  CN |
 *  8278 |	A2   A3   A4   D6 |
 *  	 |	B7   B6   D7   D2 |
 *  -----------------------------------
 */

#define SPI_MASTER_MODE					1
#define SPI_SLAVE_MODE					2
/* Select test mode. */
#define SPI_MODE						1

/* SPI read/write command. */
#define SPI_READ_CMD  					0x80
#define SPI_WRITE_CMD 					0x00
#define CMD_BUF_LEN						4

/* 826x sram address 		-- 0x8000 ~ 0xffff
 * 8258/8278 sram address 	-- 0x40000 ~ 0x4ffff
 */
#if defined(MCU_CORE_8258) || defined(MCU_CORE_8278)
#define SPI_SLAVE_ADDR					0x48000
#define SPI_CS_PIN				        GPIO_PB2//SPI CS
#elif MCU_CORE_826x
#define SPI_SLAVE_ADDR					0x8800
#define SPI_CS_PIN				        GPIO_PB4//SPI CS
#else

#endif


#define DBG_DATA_LEN    				16

//write buff
volatile unsigned char spi_master_tx_buff[DBG_DATA_LEN] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
//read buff
volatile unsigned char spi_master_rx_buff[DBG_DATA_LEN] = {0x00};

unsigned char cmd_buf[CMD_BUF_LEN] = {0};


void spi_master_test_init(void)
{
	//spi clock 500K, only master need set spi clock
	//div_clock. spi_clk = sys_clk/((div_clk+1)*2),mode select
	drv_spi_master_init((unsigned char)(CLOCK_SYS_CLOCK_HZ/(2*500000)-1), SPI_MODE_0);

#if	defined(MCU_CORE_8278)
	drv_spi_master_pin_select(SPI_GPIO_SCL_A4, SPI_GPIO_CS_D6, SPI_GPIO_SDO_A2, SPI_GPIO_SDI_A3);
#else
	drv_spi_master_pin_select(SPI_PIN_GPIO1);
#endif

	//switch CS pin
	drv_spi_master_cspin_select(SPI_CS_PIN);
}

void spi_slave_test_init(void)
{
	drv_spi_slave_init((unsigned char)(CLOCK_SYS_CLOCK_HZ/(2*500000)-1), SPI_MODE_0);//slave mode init

#if	defined(MCU_CORE_8278)
	drv_spi_slave_pin_select(SPI_GPIO_SCL_A4, SPI_GPIO_CS_D6, SPI_GPIO_SDO_A2, SPI_GPIO_SDI_A3);
#else
	drv_spi_slave_pin_select(SPI_PIN_GPIO1);
#endif
}

void spi_master_mainloop(void)
{
	WaitMs(1000);   //1S

	spi_master_tx_buff[0] += 1;
	spi_master_tx_buff[0] &= 0xff;

	u8 idx = 0;
#if defined(MCU_CORE_8258) || defined(MCU_CORE_8278)
	cmd_buf[idx++] = (SPI_SLAVE_ADDR >> 16) & 0xff;
	cmd_buf[idx++] = (SPI_SLAVE_ADDR >> 8) & 0xff;
	cmd_buf[idx++] = SPI_SLAVE_ADDR & 0xff;
#elif MCU_CORE_826x
	cmd_buf[idx++] = (SPI_SLAVE_ADDR >> 8) & 0xff;
	cmd_buf[idx++] = SPI_SLAVE_ADDR & 0xff;
#endif
	cmd_buf[idx] = SPI_WRITE_CMD;

	drv_spi_write((unsigned char*)cmd_buf, idx + 1, (unsigned char*)spi_master_tx_buff, DBG_DATA_LEN, SPI_CS_PIN);

	WaitMs(1000);   //1S

	cmd_buf[idx] = SPI_READ_CMD;

	drv_spi_read((unsigned char*)cmd_buf, idx + 1, (unsigned char*)spi_master_rx_buff, DBG_DATA_LEN, SPI_CS_PIN);
}

void main_loop(void)
{
	WaitMs(1000);

#if(SPI_MODE == SPI_MASTER_MODE)
	spi_master_mainloop();
#else
	WaitMs(50);
#endif
}

void moduleTest_spi(void)
{
	WaitMs(2000);//leave enough time for SWS_reset when power on

#if(SPI_MODE == SPI_MASTER_MODE)
	 spi_master_test_init();
#else
	 spi_slave_test_init();
#endif

//	irq_enable();

	while(1){
#if(MODULE_WATCHDOG_ENABLE)
		wd_clear(); //clear watch dog
#endif
		main_loop();
	}
}
#endif


#define MODULE_TEST_GPIO_IRQ	0

#if MODULE_TEST_GPIO_IRQ

#if 1
//8258
#define TEST_LED1		GPIO_PA4
#define TEST_LED2		GPIO_PB1
#define TEST_SW1		GPIO_PD5
#define TEST_SW2		GPIO_PD6
#define TEST_GPIO		GPIO_PB2
#else
#if 0
#define TEST_LED1		GPIO_PC2
#define TEST_LED2		GPIO_PC3
#define TEST_SW1		GPIO_PD2
#define TEST_SW2		GPIO_PC5

#else //8278 dongle
#define TEST_LED1		GPIO_PA2
#define TEST_LED2		GPIO_PA3
#define TEST_SW1		GPIO_PD6
#define TEST_SW2		GPIO_PD5
#define TEST_GPIO		GPIO_PB2
#endif
#endif

volatile u8 T_DBG_gpioIrqCb1 = 0;
void moduleTest_gpioIrqCb1(void)
{
	T_DBG_gpioIrqCb1++;
	gpio_toggle(TEST_LED1);
}

volatile u8 T_DBG_gpioIrqCb2 = 0;
void moduleTest_gpioIrqCb2(void)
{
	T_DBG_gpioIrqCb2++;
	gpio_toggle(TEST_LED2);
}

volatile u8 T_DBG_gpioIrqCb3 = 0;
void moduleTest_gpioIrqCb3(void)
{
	T_DBG_gpioIrqCb3++;
	gpio_toggle(TEST_LED1);

	if(gpio_read(TEST_GPIO)){
		gpio_setup_up_down_resistor(TEST_GPIO, PM_PIN_PULLUP_10K);
		drv_gpio_irq_conf(GPIO_IRQ_MODE, TEST_GPIO, poll_falling, moduleTest_gpioIrqCb3);
	}else{
		gpio_setup_up_down_resistor(TEST_GPIO, PM_PIN_PULLDOWN_100K);
		drv_gpio_irq_conf(GPIO_IRQ_MODE, TEST_GPIO, poll_rising, moduleTest_gpioIrqCb3);
	}
}

volatile u8 T_DBG_mainCnt = 0;
void moduleTest_gpioIrq(void)		//comment out user_init
{
	gpio_init();

	WaitMs(2000);
	//1.init the LED pin,for indication
	gpio_set_func(TEST_LED1 ,AS_GPIO);
	gpio_set_output_en(TEST_LED1, 1); 		//enable output
	gpio_set_input_en(TEST_LED1 ,0);			//disable input
	gpio_write(TEST_LED1, 1);              	//LED On

	gpio_set_func(TEST_LED2 ,AS_GPIO);
	gpio_set_output_en(TEST_LED2, 1); 		//enable output
	gpio_set_input_en(TEST_LED2 ,0);			//disable input
	gpio_write(TEST_LED2, 1);              	//LED On

#if 0
	gpio_set_func(TEST_SW1 ,AS_GPIO);
	gpio_set_output_en(TEST_SW1, 0); 			//enable output
	gpio_set_input_en(TEST_SW1 ,1);				//disable input
	gpio_setup_up_down_resistor(TEST_SW1, PM_PIN_PULLUP_10K);
	drv_gpio_irq_conf(GPIO_IRQ_MODE, TEST_SW1, poll_falling, moduleTest_gpioIrqCb1);
	drv_gpio_irq_en(TEST_SW1);
#endif

#if 0
	gpio_set_func(TEST_SW2 ,AS_GPIO);
	gpio_set_output_en(TEST_SW2, 0); 			//enable output
	gpio_set_input_en(TEST_SW2 ,1);				//disable input
	gpio_setup_up_down_resistor(TEST_SW2, PM_PIN_PULLUP_10K);
	drv_gpio_irq_conf(GPIO_IRQ_RISC0_MODE, TEST_SW2, poll_falling, moduleTest_gpioIrqCb2);
	drv_gpio_irq_risc0_en(TEST_SW2);
#endif

	gpio_set_func(TEST_GPIO ,AS_GPIO);
	gpio_set_output_en(TEST_GPIO, 0); 			//enable output
	gpio_set_input_en(TEST_GPIO ,1);				//disable input
	if(gpio_read(TEST_GPIO)){				//test edge trigger irq
		gpio_setup_up_down_resistor(TEST_GPIO, PM_PIN_PULLUP_10K);
		drv_gpio_irq_conf(GPIO_IRQ_MODE, TEST_GPIO, poll_falling, moduleTest_gpioIrqCb3);
	}else{
		gpio_setup_up_down_resistor(TEST_GPIO, PM_PIN_PULLDOWN_100K);
		drv_gpio_irq_conf(GPIO_IRQ_MODE, TEST_GPIO, poll_rising, moduleTest_gpioIrqCb3);
	}
	drv_gpio_irq_en(TEST_GPIO);

	while(1){
		T_DBG_mainCnt++;
	};
}
#endif


void moduleTest_start(void){
#if MODULE_TEST_MMOHASH
	moudleTest_hash();
#endif

#if MODULE_TEST_CCM
	moduleTest_ccm();
#endif

#if MODULE_TEST_PM
	moduleTest_PM();
#endif

#if MODULE_TEST_UART
	moduleTest_forUart();
#endif

#if MODULE_TEST_RF
	moduleTest_RF();
#endif

#if MODULE_TEST_ADC
	moduleTest_adc(0);
#endif

#if MODULE_TEST_TIMER
	moduleTest_timer();
#endif

#if MODULE_TEST_PWM
	moduleTest_pwm();
#endif

#if MODULE_TEST_SPI
	moduleTest_spi();
#endif

#if MODULE_TEST_NV
	moduleTest_NV();
#endif

#if MODULE_TEST_GPIO_IRQ
	moduleTest_gpioIrq();
#endif
}



