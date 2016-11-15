#ifndef __B64_H__
#define __B64_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static const uint8_t b64t[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

uint8_t *b64encode( uint8_t*in, uint32_t len);


#endif
