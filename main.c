/**
  ******************************************************************************
  * @file    Project/STM32F4xx_StdPeriph_Templates/main.c 
  * @author  MCD Application Team
  * @version V1.2.0RC2
  * @date    20-February-2013
  * @brief   Main program body
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"

#define CANARD_SPIN_PERIOD   1000



static void getNodeInfoHandleCanard(CanardRxTransfer* transfer);
static uint16_t makeNodeInfoMessage(uint8_t buffer[UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE]);
static void makeNodeStatusMessage(uint8_t buffer[UAVCAN_NODE_STATUS_MESSAGE_SIZE]);
static uint32_t getUptime(void);
static bool shouldAcceptTransfer(const CanardInstance* ins,
                                 uint64_t* out_data_type_signature,
                                 uint16_t data_type_id,
                                 CanardTransferType transfer_type,
                                 uint8_t source_node_id);
void systickIsr(void);

/////////////////////////////////////////////////////////////////////


static CanardInstance g_canard;             // The library instance
static uint8_t g_canard_memory_pool[1024];  // Arena for memory allocation, used by the library
static RCC_ClocksTypeDef g_RCC_Clocks;

static uint32_t  g_uptime = 0;




/*--------------------------------------------------------------------------------*/

bool shouldAcceptTransfer(const CanardInstance* ins,
                          uint64_t* out_data_type_signature,
                          uint16_t data_type_id,
                          CanardTransferType transfer_type,
                          uint8_t source_node_id)
{
    if ((transfer_type == CanardTransferTypeRequest) &&
        (data_type_id == UAVCAN_GET_NODE_INFO_DATA_TYPE_ID))
    {
        *out_data_type_signature = UAVCAN_GET_NODE_INFO_DATA_TYPE_SIGNATURE;
        return true;
    }

    return false;
}

static void onTransferReceived(CanardInstance* ins, CanardRxTransfer* transfer)
{
    if ((transfer->transfer_type == CanardTransferTypeRequest) &&
    (transfer->data_type_id == UAVCAN_GET_NODE_INFO_DATA_TYPE_ID))
    {
    	getNodeInfoHandleCanard(transfer);
    }
}

static void swInit(void)
{
    CanardSTM32CANTimings timings;
    int result = canardSTM32ComputeCANTimings(g_RCC_Clocks.PCLK1_Frequency, 1000000, &timings);
    if (result)
    {
        __ASM volatile("BKPT #01");
    }
    canardSTM32Init(&timings, CanardSTM32IfaceModeNormal);
    result = canardSTM32Init(&timings, CanardSTM32IfaceModeNormal);
    if (result)
    {
        __ASM volatile("BKPT #01");
    }

    canardInit(&g_canard,                         // Uninitialized library instance
               g_canard_memory_pool,              // Raw memory chunk used for dynamic allocation
               sizeof(g_canard_memory_pool),      // Size of the above, in bytes
               onTransferReceived,                // Callback, see CanardOnTransferReception
               shouldAcceptTransfer,              // Callback, see CanardShouldAcceptTransfer
               NULL);

    canardSetLocalNodeID(&g_canard, 100);
}




static void spinCanard(void)
{
    static uint32_t spin_time = 0;
    if (getUptime() < spin_time + CANARD_SPIN_PERIOD) { return; }  // rate limiting
    spin_time = getUptime();
    gpioToggle(GPIOD, GPIO_Pin_12);                           // some indication

    uint8_t buffer[UAVCAN_NODE_STATUS_MESSAGE_SIZE];
    static uint8_t transfer_id = 0;                          // This variable MUST BE STATIC; refer to the libcanard documentation for the background

    makeNodeStatusMessage(buffer);

    canardBroadcast(&g_canard,
                    UAVCAN_NODE_STATUS_DATA_TYPE_SIGNATURE,
                    UAVCAN_NODE_STATUS_DATA_TYPE_ID,
                    &transfer_id,
                    CANARD_TRANSFER_PRIORITY_LOW,
                    buffer,
                    UAVCAN_NODE_STATUS_MESSAGE_SIZE);
}



static void makeNodeStatusMessage(uint8_t buffer[UAVCAN_NODE_STATUS_MESSAGE_SIZE])
{
    const uint8_t node_health = UAVCAN_NODE_HEALTH_OK;
    const uint8_t node_mode   = UAVCAN_NODE_MODE_OPERATIONAL;
    memset(buffer, 0, UAVCAN_NODE_STATUS_MESSAGE_SIZE);
    const uint32_t uptime_sec = getUptime() / 1000;
    canardEncodeScalar(buffer,  0, 32, &uptime_sec);
    canardEncodeScalar(buffer, 32,  2, &node_health);
    canardEncodeScalar(buffer, 34,  3, &node_mode);
}


static uint16_t makeNodeInfoMessage(uint8_t buffer[UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE])
{
    memset(buffer, 0, UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE);
    makeNodeStatusMessage(buffer);

    buffer[7] = APP_VERSION_MAJOR;
    buffer[8] = APP_VERSION_MINOR;
    buffer[9] = 1;  // Optional field flags, VCS commit is set
    const uint32_t git_hash = GIT_HASH;
    canardEncodeScalar(buffer, 80, 32, &git_hash);

    readUniqueID(&buffer[24]);
    const size_t name_len = strlen(APP_NODE_NAME);
    memcpy(&buffer[41], APP_NODE_NAME, name_len);
    return 41 + name_len ;
}


static void readUniqueID(uint8_t* out_uid)
{
    for (uint8_t i = 0; i < UNIQUE_ID_LENGTH_BYTES; i++)
    {
        out_uid[i] = i;
    }
}
static void getNodeInfoHandleCanard(CanardRxTransfer* transfer)
{
    uint8_t buffer[UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE];
    memset(buffer, 0, UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE);
    const uint16_t len = makeNodeInfoMessage(buffer);
    int result = canardRequestOrRespond(&g_canard,
                                        transfer->source_node_id,
                                        UAVCAN_GET_NODE_INFO_DATA_TYPE_SIGNATURE,
                                        UAVCAN_GET_NODE_INFO_DATA_TYPE_ID,
                                        &transfer->transfer_id,
                                        transfer->priority,
                                        CanardResponse,
                                        &buffer[0],
                                        (uint16_t)len);
    if (result < 0)
    {
        // TODO: handle the error
    }
}


static void sendCanard(void)
{
    const CanardCANFrame* txf = canardPeekTxQueue(&g_canard);
    while(txf)
    {
        const int tx_res = canardSTM32Transmit(txf);
        if (tx_res < 0)         // Failure - drop the frame and report
        {
            __ASM volatile("BKPT #01");   // TODO: handle the error properly
        }
        if(tx_res > 0)
        {
            canardPopTxQueue(&g_canard);
        }
        txf = canardPeekTxQueue(&g_canard);
    }
}

static void receiveCanard(void)
{
    CanardCANFrame rx_frame;
    int res = canardSTM32Receive(&rx_frame);
    if(res)
    {
        canardHandleRxFrame(&g_canard, &rx_frame, getUptime() * 1000);
    }
}


int main(void)
{
	//Initialize system for 180MHz core clock
	 //SystemInit();
	 RCC_GetClocksFreq(&g_RCC_Clocks);         // To make sure RCC is initialised properly
	 hwInit();
	 swInit();
	 SysTick_Config(SystemCoreClock / 1000); // To make systick event happen every 1 ms
	 while(1)
	 {
	    GPIO_ResetBits(GPIOD, GPIO_Pin_12);
	    sendCanard();
	    receiveCanard();
	    spinCanard();
	 }
}




static uint32_t getUptime(void)
{
    return g_uptime;     // Atomic read, locking is not needed
}

void systickIsr(void)
{
    g_uptime++;
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
