#pragma once

/**
 * @brief initialize uart protocol
 */
void uart_init();

/**
 * @brief put a character to uart
 * @param[in] c character to put
 */
void uart_putc(char c);

/**
 * @brief get a character from uart
 * @return retrieved character
 */
char uart_getc();

/**
 * @brief prints a formatted string to uart
 * @param format formatted stirng
 * @param args argument to fill the formatted string
 */
void uart_printf(char* format, ...);

/**
 * @brief reads a line from uart
 * @param[out] str pointer to the buffer to write the received stirng
 * @return str
 */
char* uart_gets(char* str);
