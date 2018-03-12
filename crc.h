#pragma once

void create_crc_table();
unsigned int CRC32_4(const unsigned char* data, unsigned int reg, int len);