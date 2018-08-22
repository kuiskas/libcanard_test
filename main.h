/**
  ******************************************************************************
  * @file    Project/STM32F4xx_StdPeriph_Templates/main.h 
  * @author  MCD Application Team
  * @version V1.2.0RC2
  * @date    20-February-2013
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#include <stdio.h>
#include <stdint.h>
#include "stm32f4xx_conf.h"
#include "canard.h"
#include "canard_stm32.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define APP_VERSION_MAJOR                                           99
#define APP_VERSION_MINOR                                           99
#define APP_NODE_NAME                                               "com.zubax.babel.demo"
#define GIT_HASH                                                    0xBADC0FFE

#define UAVCAN_GET_NODE_INFO_DATA_TYPE_SIGNATURE                    0xee468a8121c46a9e
#define UAVCAN_GET_NODE_INFO_DATA_TYPE_ID                           1
#define UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE                      ((3015 + 7) / 8)

#define UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_ID                          1030
#define UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_SIGNATURE                   0x217f5c87d7ec951d
#define UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_MAX_VALUE                   8192

#define UNIQUE_ID_LENGTH_BYTES                                      16

#define UAVCAN_NODE_HEALTH_OK                                       0
#define UAVCAN_NODE_HEALTH_WARNING                                  1
#define UAVCAN_NODE_HEALTH_ERROR                                    2
#define UAVCAN_NODE_HEALTH_CRITICAL                                 3

#define UAVCAN_NODE_MODE_OPERATIONAL                                0
#define UAVCAN_NODE_MODE_INITIALIZATION                             1

#define UAVCAN_NODE_STATUS_MESSAGE_SIZE                             7

#define UAVCAN_NODE_STATUS_MESSAGE_SIZE                             7
#define UAVCAN_NODE_STATUS_DATA_TYPE_ID                             341
#define UAVCAN_NODE_STATUS_DATA_TYPE_SIGNATURE                      0x0f0868d0c1a7c6f1

#define UAVCAN_PROTOCOL_DEBUG_KEYVALUE_ID                           16370
#define UAVCAN_PROTOCOL_DEBUG_KEYVALUE_SIGNATURE                    0xe02f25d6e0c98ae0
#define UAVCAN_PROTOCOL_DEBUG_KEYVALUE_MESSAGE_SIZE                 62
#define UNIQUE_ID_LENGTH_BYTES                                      16

#define UAVCAN_PROTOCOL_PARAM_GETSET_ID                             11
#define UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE                      0xa7b622f939d1a4d5

#define CAN_RX_PIN                 GPIO_Pin_0
#define CAN_TX_PIN                 GPIO_Pin_1
#define CAN_GPIO_PORT              GPIOD
#define CAN_AF_PORT                GPIO_AF_CAN1
#define CAN_RX_SOURCE              GPIO_PinSource0
#define CAN_TX_SOURCE              GPIO_PinSource1

#define LED1_PIN                   GPIO_Pin_12
#define LED2_PIN                   GPIO_Pin_13
#define LED_GPIO_PORT              GPIOD








static void hwInit(void)
{
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
  

  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_PinAFConfig(CAN_GPIO_PORT, CAN_RX_SOURCE, CAN_AF_PORT);
  GPIO_PinAFConfig(CAN_GPIO_PORT, CAN_TX_SOURCE, CAN_AF_PORT);


  /*CAN*/
  GPIO_InitStructure.GPIO_Pin = CAN_RX_PIN | CAN_TX_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(CAN_GPIO_PORT, &GPIO_InitStructure);

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
//  /**timers*/
//  GPIO_InitStructure.GPIO_PuPd  =   GPIO_PuPd_NOPULL;
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
//  GPIO_Init(GPIOA, &GPIO_InitStructure);
//
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_6 | GPIO_Pin_7;
//  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /*leds*/
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // GPIO_OType_PP
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = LED1_PIN | LED2_PIN;
  GPIO_Init(LED_GPIO_PORT, &GPIO_InitStructure);

  GPIO_SetBits(GPIOD,GPIO_Pin_12|GPIO_Pin_13);





}



static void readUniqueID(uint8_t* out_uid);

static inline void gpioToggle(GPIO_TypeDef * port, uint16_t pin)
{
	GPIO_ToggleBits(port,pin);

}

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
