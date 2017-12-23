#include "interCode.h"

char* codeFileName;

/* 生成代码节点的开始操作，得到一个空白节点，然后赋值 */
#define beginCode(inKind) \
	InterCodeChain codeNode = newCodeNode(); \
	InterCode code = codeNode->code; \
	code->kind = inKind; \
	code->opKind = VAL_OP;
/* 根据参数个数不同有不同的参数赋值操作 */
#define endCode() \
	return codeNode;

#define endCode1(op1) \
	code->op1 = op1 ; \
	return codeNode;

#define endCode2(op1,op2) \
	code->op1 = op1 ; \
	code->op2 = op2 ; \
	return codeNode;

#define endCode3(op1,op2,op3) \
	code->op1 = op1 ; \
	code->op2 = op2 ; \
	code->op3 = op3 ; \
	return codeNode;

#define endCode4(op1,op2,op3,op4) \
	code->op1 = op1 ; \
	code->op2 = op2 ; \
	code->op3 = op3 ; \
	code->op4 = op4 ; \
	return codeNode;

InterCodeChain labelCode(int labelNo){
	beginCode(LABEL_IC);
	endCode1(labelNo);
}

InterCodeChain funcCode(char* funcName){
	beginCode(FUNC_IC);
	endCode1(funcName);
}

InterCodeChain assignCode(Operand op1, Operand op2, int opKind){
	beginCode(ASSIGN_IC);
	endCode3(op1,op2,opKind);
}

InterCodeChain addCode(Operand op1, Operand op2, Operand op3,int opKind){
	beginCode(ARITH_IC);
	code->arithType = '+';
	endCode4(op1,op2,op3,opKind);
}

InterCodeChain subCode(Operand op1, Operand op2, Operand op3,int opKind){
	beginCode(ARITH_IC);
	code->arithType = '-';
	endCode4(op1,op2,op3,opKind);
}

InterCodeChain starCode(Operand op1, Operand op2, Operand op3,int opKind){
	beginCode(ARITH_IC);
	code->arithType = '*';
	endCode4(op1,op2,op3,opKind);
}

InterCodeChain divCode(Operand op1, Operand op2, Operand op3,int opKind){
	beginCode(ARITH_IC);
	code->arithType = '/';
	endCode4(op1,op2,op3,opKind);
}

InterCodeChain gotoCode(int labelNo){
	beginCode(GOTO_IC);
	endCode1(labelNo);
}

InterCodeChain relopCode(Operand op1, Operand op2, int labelNo, char* relopType){
	beginCode(RELOP_IC);
	endCode4(op1,op2,labelNo,relopType);
}

InterCodeChain returnCode(Operand op1){
	beginCode(RETURN_IC);
	endCode1(op1);
}

InterCodeChain decCode(Operand op1, int decSize){
	beginCode(DEC_IC);
	endCode2(op1,decSize);
}

InterCodeChain argCode(Operand op1, int opKind){
	beginCode(ARG_IC);
	endCode2(op1, opKind);
}

InterCodeChain callCode(Operand op1, char* funcName){
	beginCode(CALL_IC);
	endCode2(op1,funcName);
}

InterCodeChain paramCode(Operand op1){
	beginCode(PARAM_IC);
	endCode1(op1);
}

InterCodeChain readCode(Operand op1){
	beginCode(READ_IC);
	endCode1(op1);
}

InterCodeChain writeCode(Operand op1){
	beginCode(WRITE_IC);
	endCode1(op1);
}

/* 生成代码链的一个空白节点 */
InterCodeChain newCodeNode(){
	InterCodeChain codeNode = malloc(sizeof(struct InterCodeChain_));
	codeNode->pre = codeNode;
	codeNode->next = codeNode;
	codeNode->code = malloc(sizeof(struct InterCode_));
	return codeNode;
}

/* 生成常数运算符 */
Operand newConst(int value){
	Operand op = malloc(sizeof(struct Operand_));
	op->kind = CONST;
	op->value = value;
	op->isAddr = false;
	return op;
}

/* 生成变量运算符 */
Operand newVar(int value, int isAddr){
	Operand op = malloc(sizeof(struct Operand_));
	op->kind = VAR;
	op->value = value;
	op->isAddr = isAddr;
	return op;
}

int tempNo = 0;
int addrNo = 0;
int labelNo = 0;

/* 生成一个标签 */
int newLabel(){
	int ret = labelNo;
	labelNo++;
	return ret;
}

/* 生成一个临时变量运算符 */
Operand newTemp(bool isAddr){
	Operand op = malloc(sizeof(struct Operand_));
	op->kind = TEMP;
	op->value = tempNo;
	op->isAddr = isAddr;
	tempNo++;
	return op;
}


/* 根据操作数类型和表达式类型不同，返回相应的格式串 */
char* Op_s(Operand op,int opKind){
	char opKinds[3][20] = {"v", "t", "#" };
	char number[20];
	char* ret = malloc(20);
	sprintf(number,"%d",op->value);
	strcat(opKinds[op->kind],number);
	char* opName = opKinds[op->kind];
	char option1[20] = "*";
	char option2[20] = "&";
	if(opKind == ADDR_OP){
		if(op->kind == VAR && !op->isAddr){
			strcat(option2, opName);
			strcpy(ret, option2);
			return ret;
		}
	}
	else if(opKind == VAL_OP){
		if(op->isAddr){
			strcat(option1, opName);
			strcpy(ret, option1);
			return ret;
		}
	}
	else {
		printf("wrong opKind\n");
		exit(0);
	}
	strcpy(ret, opName);
	return ret;
}

/* 返回变量的名字 */
char* opName(Operand op){
	char opKinds[3][20] = {"v", "t", "#" };
	char number[20];
	char* ret = malloc(20);
	sprintf(number,"%d",op->value);
	strcat(opKinds[op->kind],number);
	strcpy(ret, opKinds[op->kind]);
	return ret;
}

/* 公共结构的打印 */
#define codeForm1(name) \
	case concat(name,_IC): \
		fprintf(file,#name); \
		fprintf(file," %s\n",Op_s(code->op1,code->opKind)); \
		break;

/* 根据代码类型不同，执行不同的打印 */
void printCode(FILE* file, InterCode code){
	switch(code->kind){
	case LABEL_IC:
		fprintf(file,"LABEL label%d :\n", code->labelNo);
		break;
	case FUNC_IC:
		fprintf(file,"FUNCTION %s :\n", code->funcName);
		break;
	case ASSIGN_IC:
		fprintf(file,"%s := %s\n",Op_s(code->op1,code->opKind),
				Op_s(code->op2,code->opKind));
		break;
	case ARITH_IC:
		fprintf(file,"%s := %s %c %s\n",Op_s(code->op1,code->opKind),
				Op_s(code->op2,code->opKind),
				code->arithType,Op_s(code->op3,VAL_OP));
		break;
	case RELOP_IC:
		fprintf(file,"IF %s %s %s ",Op_s(code->op1,code->opKind),
				code->relopType,Op_s(code->op2,code->opKind));
	case GOTO_IC:
		fprintf(file,"GOTO label%d\n",code->labelNo);
		break;
	case DEC_IC:
		fprintf(file,"DEC %s %d\n",Op_s(code->op1,code->opKind),
				code->decSize);
		break;
	case CALL_IC:
		fprintf(file,"%s := CALL %s\n",Op_s(code->op1,code->opKind),
				code->funcName);
		break;
	case PARAM_IC:
		fprintf(file,"PARAM %s\n", opName(code->op1));
		//fprintf(file,"PARAM %s\n", Op_s(code->op1, code->opKind));
		break;
	codeForm1(RETURN);
	codeForm1(ARG);
	codeForm1(READ);
	codeForm1(WRITE);
	}
}

/* 扫描，逐个打印到文件 */
void printCodes(InterCodeChain head){
	FILE* file = fopen(codeFileName,"w");
	InterCodeChain temp = head;
	if(head != NULL){
		printCode(file,head->code);
		InterCodeChain temp = head->next;;
		while(temp != head){
			printCode(file,temp->code);
			temp = temp->next;
		}
	}
}

/* 将两段代码连接起来,将新的头节点返回 */
InterCodeChain linkCode2(InterCodeChain code1, InterCodeChain code2){
	if(code1 == NULL)
		return code2;
	else if(code2 == NULL)
		return code1;
	else{
		InterCodeChain code1Tail = code1->pre;
		InterCodeChain code2Tail = code2->pre;
		code1->pre = code2Tail;
		code1Tail->next = code2;
		code2->pre = code1Tail;
		code2Tail->next = code1;
	}
	return code1;
}

/* 将三段代码连接起来,将新的头节点返回 */
InterCodeChain linkCode3(InterCodeChain code1, InterCodeChain code2,
		InterCodeChain code3){
	return linkCode2(linkCode2(code1,code2),code3);
}

/* 将四段代码连接起来,将新的头节点返回 */
InterCodeChain linkCode4(InterCodeChain code1, InterCodeChain code2,
		InterCodeChain code3, InterCodeChain code4){
	return linkCode2(linkCode2(code1,code2),linkCode2(code3,code4));
}
