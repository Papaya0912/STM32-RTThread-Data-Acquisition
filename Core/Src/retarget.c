#include "main.h"
#include <stdio.h>

extern UART_HandleTypeDef huart6;
extern UART_HandleTypeDef huart2;

// Biến toàn cục lưu UART đang được chọn (mặc định là UART1)
UART_HandleTypeDef *Target_UART = &huart2;

int __io_putchar(int ch)
{
    uint8_t c = (uint8_t)ch;
    // Chỉ in ra UART đang được chọn
    if (Target_UART != NULL) {
        HAL_UART_Transmit(Target_UART, &c, 1, HAL_MAX_DELAY);
    }
    return ch;
}

int __io_getchar(void)
{
    uint8_t c;
    if (Target_UART != NULL) {
        HAL_UART_Receive(Target_UART, &c, 1, HAL_MAX_DELAY);
    }
    return (int)c;
}
