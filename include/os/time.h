#ifndef OS_TIME_H
#define OS_TIME_H

// Put current time into buffer, at least 8 char size
void get_time(char* buf, int len);

void init_time();

#endif // OS_TIME_H