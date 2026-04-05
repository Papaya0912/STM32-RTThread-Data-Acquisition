/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include "rtthread.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/*
 * ADS1115 dùng địa chỉ 7-bit, HAL STM32 yêu cầu "địa chỉ 8-bit" (7-bit << 1).
 * - ADS1_ADDR: ADS1115 #1 (ADDR = GND => 0x48)
 * - ADS2_ADDR: ADS1115 #2 (ADDR = VDD => 0x49)
 */
#define ADS1_ADDR   (0x48 << 1) // Địa chỉ I2C của ADS1115 1
#define ADS2_ADDR   (0x49 << 1) // Địa chỉ I2C của ADS1115 2
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
DMA_HandleTypeDef hdma_i2c1_rx;
DMA_HandleTypeDef hdma_i2c1_tx;
DMA_HandleTypeDef hdma_i2c2_rx;
DMA_HandleTypeDef hdma_i2c2_tx;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */
extern UART_HandleTypeDef *Target_UART;
// BIẾN CHO ADS1 (I2C1, địa chỉ ADS1_ADDR)
static struct rt_semaphore ads1_sem;
static uint8_t ads1_tx_buf[4];      // Buffer TX DMA (gửi thanh ghi + data)
static uint8_t ads1_rx_buf[4];      // Buffer RX DMA (nhận dữ liệu 2 byte)
static int16_t ads1_raw_val[4];     // Giá trị ADC thô (signed 16-bit)
static float ads1_vol_val[4];       // Giá trị quy đổi điện áp

// BIẾN CHO ADS2 (I2C2, địa chỉ ADS2_ADDR)
static struct rt_semaphore ads2_sem;
static uint8_t ads2_tx_buf[4];      // Buffer TX DMA
static uint8_t ads2_rx_buf[4];      // Buffer RX DMA
static int16_t ads2_raw_val[4];     // Giá trị ADC thô
static float ads2_vol_val[4];       // Giá trị quy đổi điện áp

// THREAD: mỗi ADS 1 thread + stack riêng
static struct rt_thread thread1;
static uint8_t stack1[1024];        // Stack cho thread ADS1

static struct rt_thread thread2;
static uint8_t stack2[1024];        // Stack cho thread ADS2
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART6_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// CALLBACK DMA CHO I2C
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    // DMA transmit hoàn tất trên I2C nào thì release semaphore của I2C đó
    if (hi2c->Instance == I2C1) rt_sem_release(&ads1_sem);
    if (hi2c->Instance == I2C2) rt_sem_release(&ads2_sem);
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    // DMA receive hoàn tất
    if (hi2c->Instance == I2C1) rt_sem_release(&ads1_sem);
    if (hi2c->Instance == I2C2) rt_sem_release(&ads2_sem);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    // Có lỗi I2C: cũng release để thread không bị kẹt chờ semaphore
    if (hi2c->Instance == I2C1) rt_sem_release(&ads1_sem);
    if (hi2c->Instance == I2C2) rt_sem_release(&ads2_sem);
}

// HÀM GIAO TIẾP I2C + DMA CHO ADS1
static rt_err_t ads1_write_reg(uint8_t *data, uint16_t len)
{
    // Copy sang buffer riêng để DMA dùng (tránh data stack bị thay đổi)
    memcpy(ads1_tx_buf, data, len);

    // Gửi dữ liệu bằng DMA (non-blocking)
    if (HAL_I2C_Master_Transmit_DMA(&hi2c1, ADS1_ADDR,
                                    ads1_tx_buf, len) != HAL_OK)
        return RT_ERROR;

    // Chờ semaphore được release trong callback (timeout 100ms)
    return rt_sem_take(&ads1_sem, rt_tick_from_millisecond(100));
}

static rt_err_t ads1_read_reg(uint8_t reg, int16_t *val)
{
    // Chọn thanh ghi cần đọc
    ads1_tx_buf[0] = reg;

    if (HAL_I2C_Master_Transmit_DMA(&hi2c1, ADS1_ADDR,
                                    ads1_tx_buf, 1) != HAL_OK)
        return RT_ERROR;

    // Chờ TX xong (timeout 50ms)
    if (rt_sem_take(&ads1_sem, rt_tick_from_millisecond(50)) != RT_EOK)
        return RT_ETIMEOUT;

    // Đọc 2 byte dữ liệu từ thanh ghi vừa chọn
    if (HAL_I2C_Master_Receive_DMA(&hi2c1, ADS1_ADDR,
                                   ads1_rx_buf, 2) != HAL_OK)
        return RT_ERROR;

    // Chờ RX xong (timeout 50ms)
    if (rt_sem_take(&ads1_sem, rt_tick_from_millisecond(50)) != RT_EOK)
        return RT_ETIMEOUT;

    // Ghép MSB/LSB thành signed 16-bit (ADS1115 trả về big-endian)
    *val = (int16_t)((ads1_rx_buf[0] << 8) | ads1_rx_buf[1]);
    return RT_EOK;
}

// HÀM GIAO TIẾP I2C + DMA CHO ADS2
static rt_err_t ads2_write_reg(uint8_t *data, uint16_t len)
{
    // Copy sang buffer DMA riêng
    memcpy(ads2_tx_buf, data, len);

    // Gửi DMA
    if (HAL_I2C_Master_Transmit_DMA(&hi2c2, ADS2_ADDR,
                                    ads2_tx_buf, len) != HAL_OK)
        return RT_ERROR;

    // Chờ TX xong
    return rt_sem_take(&ads2_sem, rt_tick_from_millisecond(100));
}

static rt_err_t ads2_read_reg(uint8_t reg, int16_t *val)
{
    // Chọn thanh ghi cần đọc
    ads2_tx_buf[0] = reg;

    if (HAL_I2C_Master_Transmit_DMA(&hi2c2, ADS2_ADDR,
                                    ads2_tx_buf, 1) != HAL_OK)
        return RT_ERROR;

    // Chờ TX xong
    if (rt_sem_take(&ads2_sem, rt_tick_from_millisecond(50)) != RT_EOK)
        return RT_ETIMEOUT;

    // Đọc 2 byte
    if (HAL_I2C_Master_Receive_DMA(&hi2c2, ADS2_ADDR,
                                   ads2_rx_buf, 2) != HAL_OK)
        return RT_ERROR;

    // Chờ RX xong
    if (rt_sem_take(&ads2_sem, rt_tick_from_millisecond(50)) != RT_EOK)
        return RT_ETIMEOUT;

    // Ghép dữ liệu
    *val = (int16_t)((ads2_rx_buf[0] << 8) | ads2_rx_buf[1]);
    return RT_EOK;
}

// THREAD ENTRY: ADS1
static void ads1_thread_entry(void *parameter)
{
    (void)parameter;
    // MUX bits cho AIN0/AIN1/AIN2/AIN3 single-ended (datasheet ADS1115)
    uint16_t mux[4] = {0x4000, 0x5000, 0x6000, 0x7000};
    while (1)
    {
        for (int i = 0; i < 4; i++)
        {
            uint16_t config = 0x8383 | mux[i];
            uint8_t data[3];

            // Ghi Config Register (0x01)
            data[0] = 0x01;
            data[1] = config >> 8;
            data[2] = config & 0xFF;

            // Gửi cấu hình (I2C DMA) và chỉ đọc khi cấu hình ghi OK
            if (ads1_write_reg(data, 3) == RT_EOK)
            {
                // Chờ ADC chuyển đổi (10ms) - tuỳ data rate có thể điều chỉnh
                rt_thread_mdelay(10);

                int16_t raw;
                // Đọc Conversion Register (0x00)
                if (ads1_read_reg(0x00, &raw) == RT_EOK)
                {
                    // Lưu raw và quy đổi điện áp (hệ số phụ thuộc PGA/FSR)
                    ads1_raw_val[i] = raw;
                    ads1_vol_val[i] = raw * 0.000125f;
                }
            }
        }

        // In kết quả 2 kênh của ADS1

        Target_UART = &huart2;
        uint32_t current_tick = rt_tick_get();
		// In ra màn hình kèm theo giá trị Tick
		printf("[Tick: %lu] ADS1 Ch0: %d   Ch1: %d\r\n",
			   current_tick, ads1_raw_val[0], ads1_raw_val[1]);
		printf("[Tick: %lu] VOL1 Ch0: %f   Ch1: %f\r\n\n",
			   current_tick, ads1_vol_val[0], ads1_vol_val[1]);
        rt_thread_mdelay(2000);		// Tự ngủ 2 giây
    }
}

// THREAD ENTRY: ADS2

static void ads2_thread_entry(void *parameter)
{
    (void)parameter;
    // MUX bits cho 4 kênh single-ended
    uint16_t mux[4] = {0x4000, 0x5000, 0x6000, 0x7000};

    while (1)
    {
        for (int i = 0; i < 4; i++)
        {
            uint16_t config = 0x8383 | mux[i];
            uint8_t data[3];

            // Ghi Config Register (0x01)
            data[0] = 0x01;
            data[1] = config >> 8;
            data[2] = config & 0xFF;

            // Gửi cấu hình và đọc conversion
            if (ads2_write_reg(data, 3) == RT_EOK)
            {
                // Chờ ADC chuyển đổi
                rt_thread_mdelay(10);

                int16_t raw;
                // Đọc Conversion Register (0x00)
                if (ads2_read_reg(0x00, &raw) == RT_EOK)
                {
                    // Lưu raw và điện áp
                    ads2_raw_val[i] = raw;
                    ads2_vol_val[i] = raw * 0.000125f;
                }
            }
        }

        // In kết quả 4 kênh của ADS2
        Target_UART = &huart6;
        uint32_t current_tick = rt_tick_get();

		// In ra màn hình kèm theo giá trị Tick
		printf("[Tick: %lu] ADS2 Ch0: %d   Ch1: %d\r\n",
			   current_tick, ads2_raw_val[0], ads2_raw_val[1]);
		printf("[Tick: %lu] VOL2 Ch0: %f   Ch1: %f\r\n\n",
			   current_tick, ads2_vol_val[0], ads2_vol_val[1]);
        rt_thread_mdelay(2000);		// Tự ngủ 2 giây
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_USART2_UART_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
  // Thông báo boot
  printf("BOOT OK!!!\r\n");

  // Khởi tạo RT-Thread
  __disable_irq();
  rt_system_timer_init();       // Khởi tạo tick timer cho RT-Thread
  rt_system_scheduler_init();   // Khởi tạo scheduler
  rt_thread_idle_init();        // Khởi tạo idle thread
  __enable_irq();

  // IPC: semaphore cho DMA complete
  // ADS1
  rt_sem_init(&ads1_sem, "sem1", 0, RT_IPC_FLAG_FIFO);
  memset(ads1_raw_val, 0, sizeof(ads1_raw_val));
  memset(ads1_vol_val, 0, sizeof(ads1_vol_val));

  // ADS2
  rt_sem_init(&ads2_sem, "sem2", 0, RT_IPC_FLAG_FIFO);
  memset(ads2_raw_val, 0, sizeof(ads2_raw_val));
  memset(ads2_vol_val, 0, sizeof(ads2_vol_val));

  // Thread ADS1
  rt_thread_init(&thread1, "t1",
                 ads1_thread_entry, RT_NULL,
                 stack1, sizeof(stack1), 10, 10);
  rt_thread_startup(&thread1);

  // Thread ADS2
  rt_thread_init(&thread2, "t2",
                 ads2_thread_entry, RT_NULL,
                 stack2, sizeof(stack2), 10, 10);
  rt_thread_startup(&thread2);

  // Start scheduler
  rt_system_scheduler_start();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
  /* DMA1_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
  /* DMA1_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);
  /* DMA1_Stream7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream7_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream7_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
    // Lỗi nghiêm trọng: ở đây giữ CPU trong vòng lặp vô hạn để debug.
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
