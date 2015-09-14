/*
file   : uart.cpp
author : shentq
version: V1.0
date   : 2015/7/5

Copyright (c) 2015, eBox by shentq. All Rights Reserved.

Copyright Notice
No part of this software may be used for any commercial activities by any form or means, without the prior written consent of shentq.

Disclaimer
This specification is preliminary and is subject to change at any time without notice. shentq assumes no responsibility for any errors contained herein.
*/

#include "uart.h"




callbackFun UARTCallbackTable[3];//֧�ִ��ڵ�rx�ж�


USART::USART(USART_TypeDef * USARTx,GPIO* txPin,GPIO* rxPin)
{
	_USARTx = USARTx;
	txPin->mode(AF_PP);
	rxPin->mode(INPUT);
}


void USART::begin(uint32_t BaudRate)
{
   USART_InitTypeDef USART_InitStructure;
	 NVIC_InitTypeDef NVIC_InitStructure;

	#if (USE_DMA == 1)
			RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//ʹ��DMAʱ��
		#endif

	
		switch((uint32_t)_USARTx)
		{
			case (uint32_t)USART1:
				RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
				_DMA1_Channelx = DMA1_Channel4;
			
				NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
				NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
				NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
				NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
				NVIC_Init(&NVIC_InitStructure);

				break;

			case (uint32_t)USART2:
				RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
				_DMA1_Channelx = DMA1_Channel7;
			
				NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
				NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
				NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
				NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
				NVIC_Init(&NVIC_InitStructure);

				break;

			case (uint32_t)USART3:
				RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
				_DMA1_Channelx = DMA1_Channel2;
			
				NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
				NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
				NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
				NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
				NVIC_Init(&NVIC_InitStructure);

				break;
		}

    USART_InitStructure.USART_BaudRate = BaudRate;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(_USARTx, &USART_InitStructure);
	
				
		USART_DMACmd(_USARTx,USART_DMAReq_Tx,ENABLE);    
    USART_Cmd(_USARTx, ENABLE);
}
void USART::interrupt(FunctionalState x)
{
	USART_ITConfig(_USARTx,USART_IT_RXNE,x);
}
void USART::attachInterrupt(void (*callbackFun)(void))
{
	switch((uint32_t)_USARTx)
	{
		case (uint32_t)USART1:
			UARTCallbackTable[0] = callbackFun;break;
		case (uint32_t)USART2:
			UARTCallbackTable[1] = callbackFun;break;
		case (uint32_t)USART3:
			UARTCallbackTable[2] = callbackFun;break;
	}
}
uint16_t USART::receiveData()
{
	return (uint16_t)(_USARTx->DR & (uint16_t)0x01FF);
}

int USART::putChar(char ch)
{
	USART_SendData(_USARTx,ch);
	while(USART_GetFlagStatus(_USARTx, USART_FLAG_TC) == RESET);
	return ch;
}
void USART::putString(const char *str)
{
	while(*str!='\0')
	{
		USART_SendData(_USARTx,*str++);
		while(USART_GetFlagStatus(_USARTx, USART_FLAG_TC) == RESET);
	}
}	
void USART::putString(const char *str,uint16_t length)
{
	#if (USE_DMA == 1)
	  while(USART_GetFlagStatus(_USARTx, USART_FLAG_TC) == RESET);
	  DMA_DeInit(_DMA1_Channelx);   //��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ
		_DMA1_Channelx->CPAR = (u32)&_USARTx->DR; //�����ַ
    _DMA1_Channelx->CMAR = (u32) str; //mem��ַ
    _DMA1_Channelx->CNDTR = length ; //���䳤��
    _DMA1_Channelx->CCR = (0 << 14) | // �Ǵ洢�����洢��ģʽ
            (2 << 12) | // ͨ�����ȼ���
            (0 << 11) | // �洢�����ݿ���8bit
            (0 << 10) | // �洢�����ݿ���8bit
            (0 <<  9) | // �������ݿ���8bit
            (0 <<  8) | // �������ݿ���8bit
            (1 <<  7) | // �洢����ַ����ģʽ
            (0 <<  6) | // �����ַ����ģʽ(����)
            (0 <<  5) | // ��ѭ��ģʽ
            (1 <<  4) | // �Ӵ洢����
            (1 <<  3) | // �Ƿ�������������ж�
            (0 <<  2) | // �Ƿ������봫���ж�
            (0 <<  1) | // �Ƿ�������������ж�
            (1);        // ͨ������
	#else 
	while(length--)
	{
		USART_SendData(_USARTx,*str++);
		while(USART_GetFlagStatus(_USARTx, USART_FLAG_TC) == RESET);
	}
	#endif
}	

void USART::printfln(const char *str,uint16_t length)
{	
	#if (USE_DMA == 1)
		uint16_t i = 0;
		uint16_t tmpln;
		tmpln = length;

		while(USART_GetFlagStatus(_USARTx, USART_FLAG_TC) == RESET);
		while(tmpln--)
		{
			sendBuf[i++] = *str++;
		};

		DMA_DeInit(_DMA1_Channelx);   //��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ
		_DMA1_Channelx->CPAR = (u32)&_USARTx->DR; //�����ַ
    _DMA1_Channelx->CMAR = (u32) sendBuf; //mem��ַ
    _DMA1_Channelx->CNDTR = length ; //���䳤��
    _DMA1_Channelx->CCR = (0 << 14) | // �Ǵ洢�����洢��ģʽ
            (2 << 12) | // ͨ�����ȼ���
            (0 << 11) | // �洢�����ݿ���8bit
            (0 << 10) | // �洢�����ݿ���8bit
            (0 <<  9) | // �������ݿ���8bit
            (0 <<  8) | // �������ݿ���8bit
            (1 <<  7) | // �洢����ַ����ģʽ
            (0 <<  6) | // �����ַ����ģʽ(����)
            (0 <<  5) | // ��ѭ��ģʽ
            (1 <<  4) | // �Ӵ洢����
            (1 <<  3) | // �Ƿ�������������ж�
            (0 <<  2) | // �Ƿ������봫���ж�
            (0 <<  1) | // �Ƿ�������������ж�
            (1);        // ͨ������
	#else
		putString(str,length);
	#endif
}
void USART::printf(const char* fmt,...)
{
	 __IO uint16_t length = 0;
	while(USART_GetFlagStatus(_USARTx, USART_FLAG_TC) == RESET);
	va_list va_params;   
	va_start(va_params,fmt);   
	length = vsprintf(sendBuf,fmt,va_params);   
	va_end(va_params); 

	#if (USE_DMA == 1)
	if(length != 0)
	{
		DMA_DeInit(_DMA1_Channelx);   //��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ
		_DMA1_Channelx->CPAR = (u32)&_USARTx->DR; //�����ַ
    _DMA1_Channelx->CMAR = (u32) sendBuf; //mem��ַ
    _DMA1_Channelx->CNDTR = length ; //���䳤��
    _DMA1_Channelx->CCR = (0 << 14) | // �Ǵ洢�����洢��ģʽ
            (2 << 12) | // ͨ�����ȼ���
            (0 << 11) | // �洢�����ݿ���8bit
            (0 << 10) | // �洢�����ݿ���8bit
            (0 <<  9) | // �������ݿ���8bit
            (0 <<  8) | // �������ݿ���8bit
            (1 <<  7) | // �洢����ַ����ģʽ
            (0 <<  6) | // �����ַ����ģʽ(����)
            (0 <<  5) | // ��ѭ��ģʽ
            (1 <<  4) | // �Ӵ洢����
            (1 <<  3) | // �Ƿ�������������ж�
            (0 <<  2) | // �Ƿ������봫���ж�
            (0 <<  1) | // �Ƿ�������������ж�
            (1);        // ͨ������
//			DMA_InitStructure.DMA_PeripheralBaseAddr =  (u32)&_USARTx->DR;  //DMA����ADC����ַ
//			DMA_InitStructure.DMA_MemoryBaseAddr = (u32)sendBuf;  //DMA�ڴ����ַ
//			DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  //
//			DMA_InitStructure.DMA_BufferSize = 128;  //DMAͨ����DMA����Ĵ�С
//			DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //
//			DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //
//			DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //
//			DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte; //
//			DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //������ѭ������ģʽ
//			DMA_InitStructure.DMA_Priority = DMA_Priority_High; //
//			DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
//			DMA_Init(_DMA1_Channelx, &DMA_InitStructure);  //
//		
//		DMA_SetCurrDataCounter(_DMA1_Channelx,length);  
//		DMA_Cmd(_DMA1_Channelx,ENABLE);  
	}
	#else
		putString(sendBuf);
	#endif
	
}

void USART::waitBusy()
{
	while(USART_GetFlagStatus(_USARTx, USART_FLAG_TC) == RESET);
}


extern "C"{

void USART1_IRQHandler(void)
{

	if(USART_GetITStatus(USART1,USART_IT_RXNE)==SET)
	{
		UARTCallbackTable[0]();
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
	}
}
void USART2_IRQHandler(void)
{

	if(USART_GetITStatus(USART2,USART_IT_RXNE)==SET)
	{
		UARTCallbackTable[1]();
		USART_ClearITPendingBit(USART2,USART_IT_RXNE);
	}
}
void USART3_IRQHandler(void)
{

	if(USART_GetITStatus(USART3,USART_IT_RXNE)==SET)
	{
		UARTCallbackTable[2]();
		USART_ClearITPendingBit(USART3,USART_IT_RXNE);
	}
}

//void DMA1_Channel4_IRQHandler(void)  
//	{  

//		DMA_Cmd(DMA1_Channel4,DISABLE);  
//		DMA_ClearFlag(DMA1_FLAG_TC4); 

//	}	 
//	void DMA1_Channel7_IRQHandler(void)  
//	{  
//		DMA_Cmd(DMA1_Channel7,DISABLE);  
//		DMA_ClearFlag(DMA1_FLAG_TC7); 
//	}	 
//	void DMA1_Channel2_IRQHandler(void)  
//	{  
//		DMA_Cmd(DMA1_Channel2,DISABLE);  
//		DMA_ClearFlag(DMA1_FLAG_TC2); 
//	}
}

