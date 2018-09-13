/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2014 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stm32f4xx.h>
#include "diag/Trace.h"

// ----------------------------------------------------------------------------
//
// Standalone STM32F4 empty sample (trace via DEBUG).
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the DEBUG output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

#define getbit(n,b)   ((n & (1 << b)) >> b)

uint8_t spi_transfer_r(uint8_t,int);
void spi_transfer_w(uint8_t,uint8_t,int);

int
main(int argc, char* argv[])
{
	uint8_t msg  = 0x00;
	uint8_t addr = 0x00;

	GPIO_TypeDef *pc   = GPIOC;
	GPIO_TypeDef *pb   = GPIOB;
	SPI_TypeDef  *spi2 = SPI2;

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;

	pc->MODER |= GPIO_MODER_MODER9_0;
	pc->ODR   |= GPIO_ODR_ODR_9;

	pb->MODER |= GPIO_MODER_MODER13_1;
	pb->MODER |= GPIO_MODER_MODER14_1;
	pb->MODER |= GPIO_MODER_MODER15_1;

	pb->AFR[1] |= (5 << 20)|(5 << 24)|(5 << 28);

	spi2->CR1 |= SPI_CR1_MSTR;
	spi2->CR1 |= SPI_CR1_CPOL | SPI_CR1_CPHA;
	spi2->CR1 |= SPI_CR1_SSM  | SPI_CR1_SSI;
	spi2->CR1 |= SPI_CR1_SPE;

	addr = 0x2D;

	spi_transfer_w(addr,0x00,1);	//Measure Mode

	while(1)
	{
		trace_printf("X: %d   Y: %d  Z: %d\n",spi_transfer_r(0x32,1),spi_transfer_r(0x34,1),spi_transfer_r(0x36,1));
	}

}


uint8_t spi_transfer_r(uint8_t addr,int n)
{
	int i;

	GPIO_TypeDef *pc   =  GPIOC;
	SPI_TypeDef  *spi2 =  SPI2;

	pc->ODR ^= GPIO_ODR_ODR_9;

	addr |= (1 << 7);

	spi2->DR = addr;

	while(getbit(spi2->SR,7));

	for(i=0;i<n;i++)
	{
		spi2->DR = 0x00;
		while(getbit(spi2->SR,7));
	}

	while(!getbit(spi2->SR,0));

	pc->ODR ^= GPIO_ODR_ODR_9;

	return spi2->DR;
}

void spi_transfer_w(uint8_t addr,uint8_t data,int n)
{
	int i;

	GPIO_TypeDef *pc   =  GPIOC;
	SPI_TypeDef  *spi2 =  SPI2;

	pc->ODR ^= GPIO_ODR_ODR_9;

	spi2->DR = addr;

	while(getbit(spi2->SR,7));

	spi2->DR = data;

	while(getbit(spi2->SR,7));


	for(i=0;i<n;i++)
	{
		spi2->DR = data;
		while(getbit(spi2->SR,7));
	}

	trace_printf("Data Sent..\n");

	pc->ODR ^= GPIO_ODR_ODR_9;

}


#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
