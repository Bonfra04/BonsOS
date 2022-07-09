#pragma once

/**
 * @brief initializes uart protocol
 */
void uart_init();

/**
 * @brief puts a character to uart
 * @param[in] c character to put
 */
void uart_putc(char c);

/**
 * @brief puts a character from uart
 * @param[in] str string to put
 */
void uart_puts(const char* str);

/**
 * @brief gets a character from uart
 * @return Retrieved character
 */
char uart_getc();

/**
 * @brief reads a line from uart
 * @param[out] str pointer to the buffer to write the received stirng
 * @return str
 */
char* uart_gets(char* str);
