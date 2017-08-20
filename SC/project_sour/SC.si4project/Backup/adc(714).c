
#include "platform.h"

#define ADC1_DR_Address	0x40012440
__IO uint16_t RegularConvData_Tab;

int adc_init(void){
	ADC_InitTypeDef     ADC_InitStruct;
	DMA_InitTypeDef     DMA_InitStruct;
	GPIO_InitTypeDef    GPIO_InitStruct;
	/* ADC1 DeInit */  
	ADC_DeInit(ADC1);

	/* Enable  GPIOA clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	/* ADC1 Periph clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	/* DMA1 clock enable */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);

	/* Configure PA.01  as analog input */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOA, &GPIO_InitStruct);				// PA0,输入时不用设置速率


	/* DMA1 Channel1 Config */
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_Address;
	DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)&RegularConvData_Tab;
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStruct.DMA_BufferSize =4;
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStruct.DMA_Priority = DMA_Priority_High;
	DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStruct);

	/* DMA1 Channel1 enable */
	DMA_Cmd(DMA1_Channel1, ENABLE);

	//   /* ADC DMA request in circular mode */
	ADC_DMARequestModeConfig(ADC1, ADC_DMAMode_Circular);

	/* Enable ADC_DMA */
	ADC_DMACmd(ADC1, ENABLE);  

	/* Initialize ADC structure */
	ADC_StructInit(&ADC_InitStruct);

	/* Configure the ADC1 in continous mode withe a resolutuion equal to 12 bits  */
	ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStruct.ADC_ContinuousConvMode = ENABLE; 
	ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStruct.ADC_ScanDirection = ADC_ScanDirection_Backward;
	ADC_Init(ADC1, &ADC_InitStruct); 

	/* Convert the ADC1 Vref  with 55.5 Cycles as sampling time */ 
	ADC_ChannelConfig(ADC1, ADC_Channel_0  , ADC_SampleTime_55_5Cycles); 
	//   ADC_VrefintCmd(ENABLE);

	/* ADC Calibration */
	ADC_GetCalibrationFactor(ADC1);
	ADC_DMACmd(ADC1, ENABLE);
	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);     

	/* Wait the ADCEN falg */
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADEN)); 

	/* ADC1 regular Software Start Conv */ 
	ADC_StartOfConversion(ADC1);
}


void adc_init2(void) {
	//时钟配置	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);	

	//ADC IO配置，此处定义PA0口为ADC端口	
	GPIO_InitTypeDef PORT_ADC;  
	PORT_ADC.GPIO_Pin=GPIO_Pin_0;  
	PORT_ADC.GPIO_Mode=GPIO_Mode_AN;  
	PORT_ADC.GPIO_PuPd=GPIO_PuPd_NOPULL;  
	GPIO_Init(GPIOA,&PORT_ADC);	

	//ADC 参数配置  
	ADC_InitTypeDef ADC_InitStuctrue;  
	ADC_InitStuctrue.ADC_Resolution=ADC_Resolution_12b;//12位精度  
	ADC_InitStuctrue.ADC_ContinuousConvMode=DISABLE;//单次ADC	
	ADC_InitStuctrue.ADC_ExternalTrigConvEdge=ADC_ExternalTrigConvEdge_None;  
	ADC_InitStuctrue.ADC_DataAlign=ADC_DataAlign_Right;//数据右对齐  
	ADC_InitStuctrue.ADC_ScanDirection=ADC_ScanDirection_Backward;//数据覆盖  
	ADC_Init(ADC1,&ADC_InitStuctrue);  

	//配置ADC采样的通道和采样周期  
	//PA0对应ADC通道0  
	//注意，采集的数据是否准确与采样时间有关系  
	ADC_ChannelConfig(ADC1,ADC_Channel_0,ADC_SampleTime_239_5Cycles);  
	//如果采集系统内部温度，则通道为16，同时要使能温度传感器  
	//ADC_ChannelConfig(ADC1,ADC_Channel_16,ADC_SampleTime_239_5Cycles);  
	//ADC_TempSensorCmd(ENABLE);  

	//校准  
	ADC_GetCalibrationFactor(ADC1);	
	//使能  
	ADC_Cmd(ADC1,ENABLE);  
}

unsigned short adc_data;
void adc_read(void) {
	 //等待ADC准备  
    while(ADC_GetFlagStatus(ADC1,ADC_FLAG_ADEN)==RESET);  
    //软件启动ADC转换  
    ADC_StartOfConversion(ADC1);  
    //等待ADC完成  
    while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC)==RESET);  
    //所得数据即为ADC数据  
    adc_data=ADC_GetConversionValue(ADC1);  
  
    /* 
    //如果采集的是16通道，即芯片温度，则温度的值如下 
    //下面这段代价，参考STM32F030数据手册（寄存器版）A.7.16 
    //出厂校准数据所存储的地址，30°C和110°C的ADC值 
    #define TEMP110_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FFFF7C2)) 
    #define TEMP30_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FFFF7B8)) 
    #define VDD_CALIB ((uint16_t) (330)) 
    #define VDD_APPLI ((uint16_t) (300)) 
    int32_t temperature; 
    temperature = (((int32_t) ADC1->DR * VDD_APPLI / VDD_CALIB)- (int32_t) *TEMP30_CAL_ADDR ); 
    temperature = temperature * (int32_t)(110 - 30); 
    temperature = temperature / (int32_t)(*TEMP110_CAL_ADDR- *TEMP30_CAL_ADDR); 
    temperature = temperature + 30; 
    */  
}

