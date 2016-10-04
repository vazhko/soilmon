/* 
 * File:   spi.h
 * Author: Vlad
 *
 * Created on 30 Θώνό 2013 γ., 12:39
 */

#ifndef SPI_H
#define	SPI_H

#include "HardwareProfile.h"
#include "GenericTypeDefs.h"

void spi1_init(void);
void spi1_close(void);
WORD MCP3204_read(BYTE ch);
double MCP3204_read_i(WORD count, BYTE ch);


#endif	/* SPI_H */

