#pragma once

int readhex(const char * mcsfile,unsigned char *buf,size_t bufsize);
unsigned int getaddr(void);
char gethex(void);
unsigned char char2date(char ascii);
