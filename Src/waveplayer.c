/**
  ******************************************************************************
  * @file    Audio_playback_and_record/Src/waveplayer.c 
  * @author  MCD Application Team
  * @version V1.2.2
  * @date    25-May-2015
  * @brief   I2S Audio player program. 
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
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
#include "waveplayer.h"
#include "arm_math.h"

/** @addtogroup STM32F4-Discovery_Audio_Player_Recorder
* @{
*/ 

/* Private typedef -----------------------------------------------------------*/
typedef enum
{
  BUFFER_OFFSET_NONE = 0,  
  BUFFER_OFFSET_HALF,  
  BUFFER_OFFSET_FULL,     
}BUFFER_StateTypeDef;

/* Private define ------------------------------------------------------------*/

#define AUDIO_BUFFER_STEREO_LENGTH        2048

#define T60SEG      60000    // Valor a ser utilizado para comparar con el valor 
                             // devuelto por la funcion HAL_GetTick ubicada en el
                             // archivo stm32f4xx_hal.c que cuenta ticks en milisegundos.
                             // El valor 60000 corresponderia a 60 segundos
#define T1SEG       1000     // Valor a ser utilizado para comparar con el valor 
                             // devuelto por la funcion HAL_GetTick ubicada en el
                             // archivo stm32f4xx_hal.c que cuenta ticks en milisegundos.
                             // El valor 1000 corresponderia a 1 segundo


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/


static uint32_t tickstart = 0;
static uint16_t firstfind = 0; 

/* Ping-Pong buffer used for audio play */
int16_t Audio_BufferStereo[AUDIO_BUFFER_STEREO_LENGTH];

/* Position in the audio play buffer */
__IO BUFFER_StateTypeDef BufferOffset = BUFFER_OFFSET_NONE;

/* Variable used by FatFs*/
FIL FileRead;

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

static void convertToStereo(int16_t *src, int16_t *dest, int32_t lengthSrc)
{
  int32_t i;
  
  for (i = lengthSrc-1 ; i >= 0 ; i--)
  {
    dest[i*2] = src[i];
    dest[i*2+1] = src[i];
  }
}


/* Exported functions ------------------------------------------------------- */

/*--------------------------------
Callbacks implementation:
The callbacks prototypes are defined in the stm32f4_discovery_audio_codec.h file
and their implementation should be done in the user code if they are needed.
Below some examples of callback implementations.
--------------------------------------------------------*/

/**
  * @brief  Manages the DMA Half Transfer complete interrupt.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{ 
  BufferOffset = BUFFER_OFFSET_HALF;
}

/**
* @brief  Calculates the remaining file size and new position of the pointer.
* @param  None
* @retval None
*/
void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
  BufferOffset = BUFFER_OFFSET_FULL;
  BSP_AUDIO_OUT_ChangeBuffer((uint16_t*)&Audio_BufferStereo[0], sizeof(Audio_BufferStereo) /2);
}

/**
* @brief  Manages the DMA FIFO error interrupt.
* @param  None
* @retval None
*/
void BSP_AUDIO_OUT_Error_CallBack(void)
{
  /* Stop the program with an infinite loop */
  while (1)
  {}
  
  /* Could also generate a system reset to recover from the error */
  /* .... */
}

/**
  * @brief  Starts Wave player.
  * @param  None
  * @retval None
  */
void WavePlayerStart(WAVE_FormatTypeDef waveformat, 
  WavePlayer_getDataCB_type getDataCB,
  uint8_t volume)
{
  int32_t bytesread = 0;
  int32_t length;
  int16_t *pBuf = NULL;
  uint32_t AudioRemSize = 0;
  uint32_t WaveDataLength = 0;  
  
	int16_t ValorEficaz = 0;
	int16_t Contador = 0;
	int16_t Bandera = 0;
	
	
  uint32_t tickcontar = 0;
	
  /* Set WaveDataLenght to the Speech Wave length */
  WaveDataLength = waveformat.FileSize;

  AudioRemSize = WaveDataLength - sizeof(WAVE_FormatTypeDef);
  
  /* Initialize Wave player (Codec, DMA, I2C) */
  if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, volume, waveformat.SampleRate) != 0)
  {
    Error_Handler();
  }
  
  /* Start playing Wave */
  BSP_AUDIO_OUT_Play((uint16_t*)&Audio_BufferStereo[0], sizeof(Audio_BufferStereo));
  LEDsState = LED6_TOGGLE;
  
  /* Toggling LED6 to signal Play */
  LEDsState = LED6_TOGGLE;
  
	
	LEDsState = LEDS_OFF;
	
  /* Resume playing Wave */
  BSP_AUDIO_OUT_Resume();
  
  if (CHANNEL_MONO == waveformat.NbrChannels)
  {
    length = AUDIO_BUFFER_STEREO_LENGTH/4;
  }
  else
  {
    length = AUDIO_BUFFER_STEREO_LENGTH/2;
  }
      
	while (AudioRemSize > 0)
  {
    if (BUFFER_OFFSET_HALF == BufferOffset)
    {
      pBuf = &Audio_BufferStereo[0];
    }
    
    if (BUFFER_OFFSET_FULL == BufferOffset)
    {
      pBuf = &Audio_BufferStereo[AUDIO_BUFFER_STEREO_LENGTH/2];
    }
    
    if (NULL != pBuf)
    {
      bytesread = getDataCB(pBuf, length);
      
			arm_rms_q15(pBuf, length, &ValorEficaz);
			
			// printf("Valor Eficaz = %d\r\n", ValorEficaz);
			
			// 0x03E8 Hexadecimal = 1000 Decimal
			// se comprobo con la salida printf("Valor Eficaz = %d\r\n", ValorEficaz);
			// que los valores de los tonos de 1 KHz tenian un valor eficaz superior 
			// a 1000.
			
			if (ValorEficaz > 0x03E8) {
				 Bandera = 1;
				 Contador = Contador + 1;
			   }
				else { if (Bandera == 1) {
					Bandera = 2;
				  }
			   }
			
			BSP_LED_Off(LED3);
			BSP_LED_Off(LED6);																	 
			 
			if (Bandera == 2) {

  				Bandera = 0;
				
			    if (Contador > 10) {
				               BSP_LED_On(LED3); 
				               Contador = 0;
                       if (firstfind == 0)
											 {
												 firstfind = 1;
                       }												 
			                  } else {
				               BSP_LED_On(LED6);
									     Contador = 0;		
															 
												       } 
			              	 }
		
			
      if (firstfind != 0 )
      {				
			tickcontar = HAL_GetTick();
			if (firstfind == 1)
			  {
				tickstart = tickcontar;
				firstfind = 2;
				}
		  if ((tickcontar - tickstart) > T60SEG)
			{
			tickstart=tickcontar;
			}

      if ((tickcontar - tickstart) <= T1SEG)			
			{
			  BSP_LED_On(LED5);
			}
      else				
			{
				BSP_LED_Off(LED5);
			}	
				
      }			
				

			
											 
      if (CHANNEL_MONO == waveformat.NbrChannels)
      {
        convertToStereo(pBuf, pBuf, length);
      }
      
      BufferOffset = BUFFER_OFFSET_NONE;
      
      pBuf = NULL;
      
      if (bytesread <= 0)
      {
        AudioRemSize = 0;
      }
      else
      {
        AudioRemSize -= bytesread;
        if (AudioRemSize < (length*2))
        {
          AudioRemSize = 0;
        }
      }
    }
  }
  
	BSP_LED_Off(LED3);
	BSP_LED_Off(LED6);
	
  // LEDsState = LEDS_OFF;
 

  BSP_AUDIO_OUT_Stop(CODEC_PDWN_HW);
}

/**
* @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
