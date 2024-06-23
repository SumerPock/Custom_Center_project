#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

void bsp_InitGpio(void);

void bsp_GpioToggle(unsigned char _no);
void Pules_InitGpio(void);
void bsp_GpioOff(unsigned char _no);
void bsp_GpioOn(unsigned char _no);
#endif