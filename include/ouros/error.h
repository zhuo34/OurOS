#ifndef _OS_ERROR_H_
#define _OS_ERROR_H_

#include <ouros/type.h>

// 返回的错误码必须小于0
// eg: error = -ERROR_NO_MEMORY;
#define NO_ERROR 				0
#define ERROR_END_OF_FILE 		1
#define ERROR_NO_MEMORY 		2
#define ERROR_READ_SDCARD		3
#define ERROR_UNKNOWN_FS 		4
#define ERROR_VFSMOUNT 			5
#define ERROR_NOT_DIR 			6
#define ERROR_FILE_NOTFOUND 	7
#define ERROR_CACHE_FULL 		8
#define ERROR_UNKNOWN_FORMAT 	9
#define ERROR_ACCESS_AUTHORITY 	10
#define ERROR_NOT_EMPTY 		11


#define MAX_ERRNO				4095
#define IS_ERR_VALUE(x) 		( (uint)(x) >= (uint)(-MAX_ERRNO) )

#define EOF 					-ERROR_END_OF_FILE

// 把错误码转化为错误指针
static inline void* ERR_PTR(int error) {
	return (void*)error;
}

// 把错误指针转化为错误码
static inline int PTR_ERR(const void *ptr) {
	return (int)ptr;
}

// 判断是否为错误指针
static inline bool IS_ERR_PTR(const void *ptr) {
	return IS_ERR_VALUE(ptr);
}

#endif