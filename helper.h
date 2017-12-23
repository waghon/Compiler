#ifndef HELPER_H
#define HELPER_H

#include<stdio.h>
#include<string.h>

//#define DEBUG_

typedef int bool;
#define true 1
#define false 0

/* 用来获取当前行的信息 */
extern FILE* fileForLine;

/* 打印错误信息相关 */
#define MAXLINESIZE 200
void printError(int type,int line, int column, char* msg);
void printPreDec(int line, int column,char* name);
void debug(char* s);
char* myStrcat(char* msg,char* name);

/* 两个串拼接 */
#define concat(a,b) a##b

/* hash散列函数 */
unsigned int hash_pjw(char* name);

#endif
