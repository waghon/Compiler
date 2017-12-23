#ifndef INTERCODE_H
#define INTERCODE_H

#include "semanteme.h"
#include <stdio.h>



/* 操作数的结构 */
typedef struct Operand_* Operand;
struct Operand_{
	enum {VAR = 0,TEMP,CONST} kind;
	union{
		int value;
	};
	bool isAddr;
};

/* 中间代码的结构 */
typedef struct InterCode_* InterCode;
struct InterCode_{
	enum {LABEL_IC, FUNC_IC, ASSIGN_IC, ARITH_IC,  
		GOTO_IC, RELOP_IC, RETURN_IC, DEC_IC, ARG_IC, 
		CALL_IC, PARAM_IC, READ_IC, WRITE_IC} kind;
	Operand op1, op2, op3;
	int labelNo;
	union {
		int decSize;
		char *funcName;
		char *relopType;
		char arithType;
	};
	int opKind;
};

/* 中间代码链表 */
typedef struct InterCodeChain_* InterCodeChain;
struct InterCodeChain_{
	InterCode code;
	InterCodeChain pre, next;
};

/* 函数参数链表 */
typedef struct ArgList_* ArgList;
struct ArgList_{
	Operand op;
	Type* opType;
	ArgList next;
};

enum { ADDR_OP, VAL_OP };

/* 对外的接口，主要用于生成中间代码，中间代码的拼接，中间代码的打印*/
InterCodeChain labelCode(int labelNo);
InterCodeChain funcCode(char* funcName);
InterCodeChain assignCode(Operand op1, Operand op2,int opKind);
InterCodeChain addCode(Operand op1, Operand op2, Operand op3,int opKind);
InterCodeChain subCode(Operand op1, Operand op2, Operand op3,int opKind);
InterCodeChain starCode(Operand op1, Operand op2, Operand op3,int opKind);
InterCodeChain divCode(Operand op1, Operand op2, Operand op3,int opKind);
InterCodeChain gotoCode(int labelNo);
InterCodeChain relopCode(Operand op1, Operand op2, int labelNo, char* relopType);
InterCodeChain returnCode(Operand op1);
InterCodeChain decCode(Operand op1, int decSize);
InterCodeChain argCode(Operand op1, int opKind);
InterCodeChain callCode(Operand op1, char* funcName);
InterCodeChain paramCode(Operand op1);
InterCodeChain readCode(Operand op1);
InterCodeChain writeCode(Operand op1);
InterCodeChain newCodeNode();
void printCodes(InterCodeChain head);
InterCodeChain linkCode2(InterCodeChain code1, InterCodeChain code2);
InterCodeChain linkCode3(InterCodeChain code1, InterCodeChain code2,
		InterCodeChain code3);
InterCodeChain linkCode4(InterCodeChain code1, InterCodeChain code2,
		InterCodeChain code3, InterCodeChain code4);
Operand newConst(int value);
Operand newTemp(bool isAddr);
Operand newVar(int value, bool isAddr);
int newLabel();
	
#endif
