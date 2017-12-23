#include "translate.h"


/* 函数声明  */
InterCodeChain translate_Program(TreeNode* program);
InterCodeChain translate_ExtDefList(TreeNode* extDefList);
InterCodeChain translate_ExtDef(TreeNode* extDef);
InterCodeChain translate_FunDec(TreeNode* funDec);
InterCodeChain translate_CompSt(TreeNode* compSt);
InterCodeChain translate_VarList(TreeNode* varList);
InterCodeChain translate_ParamDec(TreeNode* paramDec);
TreeNode* getIDFromVarDec(TreeNode* varDec);
InterCodeChain translate_DefList(TreeNode* DefList);
InterCodeChain translate_StmtList(TreeNode* StmtList);
InterCodeChain translate_Def(TreeNode* Def);
InterCodeChain translate_DecList(TreeNode* decList);
InterCodeChain translate_Dec(TreeNode* dec);
int typeSize(Type* type);
InterCodeChain translate_Exp(TreeNode* exp, Operand* place);
InterCodeChain translate_Stmt(TreeNode* stmt);
InterCodeChain translate_Cond(TreeNode* exp,int label_true,int label_false);
bool checkNode(TreeNode* head, int num_nexts, int i, int type);
InterCodeChain translate_Args(TreeNode* args, ArgList* argList);
int getOffsetFromStructure(Type* structure, char* name);

/* 翻译函数 */
void translate(TreeNode* root){
	printf("start!\n");
	InterCodeChain codes = translate_Program(root);
	printCodes(codes);
}

/* 翻译程序 */
InterCodeChain translate_Program(TreeNode* program){
	return translate_ExtDefList(program->next[0]);
}

/* ExtDefList */
InterCodeChain translate_ExtDefList(TreeNode* extDefList){
	if(extDefList->num_nexts == 0)
		return NULL;
	return linkCode2(translate_ExtDef(extDefList->next[0]),
			translate_ExtDefList(extDefList->next[1]));
}

/* ExtDef */
InterCodeChain translate_ExtDef(TreeNode* extDef){
	if(extDef->next[1]->type_syn == type_ExtDecList){
		printf("Global variable not handled\n");
		exit(0);
	}
	else if(extDef->num_nexts ==3 && 
			extDef->next[2]->type_syn == type_CompSt){
		return linkCode2(translate_FunDec(extDef->next[1]),
				translate_CompSt(extDef->next[2]));
	}
	else{
		return NULL;
	}
}

/* FunDec */
InterCodeChain translate_FunDec(TreeNode* funDec){
	InterCodeChain code1 = funcCode(funDec->next[0]->token.type_id);
	InterCodeChain code2 = NULL;
	if(funDec->next[2]->type_syn == type_VarList){
		code2 = translate_VarList(funDec->next[2]);
	}
	return linkCode2(code1, code2);
}

/* CompSt */
InterCodeChain translate_CompSt(TreeNode* compSt){
	return linkCode2(translate_DefList(compSt->next[1]),
			translate_StmtList(compSt->next[2]));
}

/* DefList */
InterCodeChain translate_DefList(TreeNode* DefList){
	if(DefList->num_nexts == 0)
		return NULL;
	return linkCode2(translate_Def(DefList->next[0]),
			translate_DefList(DefList->next[1]));
}

/* Def */
InterCodeChain translate_Def(TreeNode* Def){
	return translate_DecList(Def->next[1]);
}

/* DecList */
InterCodeChain translate_DecList(TreeNode* decList){
	InterCodeChain code1 = translate_Dec(decList->next[0]);
	if(decList->num_nexts == 3){
		return linkCode2(code1,translate_DecList(decList->next[2]));
	}
	return code1;
}

/* Dec */
InterCodeChain translate_Dec(TreeNode* dec){
	InterCodeChain code1 = NULL;
	InterCodeChain code2 = NULL;
	TreeNode* idNode = getIDFromVarDec(dec->next[0]);
	int idIndex = idNode->symbolTableIndex;
	Symbol* sym = getFromSymbolTable(idIndex);
	if(sym->type_var->kind != BASIC){
		Operand decOp = newVar(idIndex,true);
		code1 = decCode(decOp,typeSize(sym->type_var));
	}
	if(dec->num_nexts == 3){
		Operand t1;
		Operand var = newVar(idIndex, false);
		InterCodeChain exp = translate_Exp(dec->next[2],&t1);
		InterCodeChain assign = assignCode(var, t1, VAL_OP);
		code2 =  linkCode2(exp,assign);
	}
	return linkCode2(code1,code2);
}


/* count the size of fieldList */
int  fieldListSize(FieldList* s){
	if(s == NULL)
		return 0;
	return typeSize(s->type) + fieldListSize(s->tail);
}

/* count the size of some type */
int typeSize(Type* type){
	if(type->kind == BASIC)
		return 4;
	else if(type->kind == ARRAY){
		return type->array.size * typeSize(type->array.elem);
	}
	else{
		return fieldListSize(type->structure);
	}
}



/* StmtList */
InterCodeChain translate_StmtList(TreeNode* stmtList){
	if(stmtList->num_nexts == 0)
		return NULL;
	return linkCode2(translate_Stmt(stmtList->next[0]),
			translate_StmtList(stmtList->next[1]));
}

/* Stmt */
InterCodeChain translate_Stmt(TreeNode* stmt){
	if(stmt->next[0]->type_syn == type_Exp){
		return translate_Exp(stmt->next[0],NULL);
	}
	else if(stmt->next[0]->type_syn == type_CompSt){
		return translate_CompSt(stmt->next[0]);
	}
	else if(stmt->next[0]->type_syn == type_RETURN){
		Operand t1;
		InterCodeChain code1 = translate_Exp(stmt->next[1], &t1);
		InterCodeChain code2 = returnCode(t1);
		return linkCode2(code1, code2);
	}
	else if(stmt->next[0]->type_syn == type_IF && 
			stmt->num_nexts == 5){
		int label1 = newLabel();
		int label2 = newLabel();
		return linkCode4(
				translate_Cond(stmt->next[2], label1, label2),
				labelCode(label1),
				translate_Stmt(stmt->next[4]),
				labelCode(label2));
	}
	else if(stmt->next[0]->type_syn == type_IF){
		int label1 = newLabel();
		int label2 = newLabel();
		int label3 = newLabel();
		InterCodeChain code1 = linkCode4(
				translate_Cond(stmt->next[2], label1, label2),
				labelCode(label1),
				translate_Stmt(stmt->next[4]),
				gotoCode(label3));
		InterCodeChain code2 = linkCode3(
				labelCode(label2),
				translate_Stmt(stmt->next[6]),
				labelCode(label3));
		return linkCode2(code1, code2);
	}
	else{
		int label1 = newLabel();
		int label2 = newLabel();
		int label3 = newLabel();
		InterCodeChain code1 = linkCode3(
				labelCode(label1),
				translate_Cond(stmt->next[2], label2, label3),
				labelCode(label2)
				);
		InterCodeChain code2 = linkCode3(
				translate_Stmt(stmt->next[4]),
				gotoCode(label1),
				labelCode(label3)
				);
		return linkCode2(code1, code2);
	}
}

/* check the current Tree Node */
bool checkNode(TreeNode* head, int num_nexts, int i, int type){
	return head->num_nexts == num_nexts && 
		head->next[i]->type_syn == type;
}

/* translate Condition */
InterCodeChain translate_Cond(TreeNode* exp,int label_true,int label_false){
	if(checkNode(exp,3,1,type_RELOP) == true){
		Operand t1;
		Operand t2;
		InterCodeChain code1 = translate_Exp(exp->next[0], &t1);
		InterCodeChain code2 = translate_Exp(exp->next[2], &t2);
		TreeNode* relop = exp->next[1];
		InterCodeChain code3 = relopCode(t1, t2, label_true, relop->token.type_relop);
		InterCodeChain code4 = gotoCode(label_false);
		return linkCode4(code1, code2, code3, code4);
	}
	else if(checkNode(exp, 2, 1, type_NOT) == true){
		return translate_Cond(exp->next[1], label_false, label_true);
	}
	else if(checkNode(exp, 3, 1, type_AND) == true){
		int label1 = newLabel();
		InterCodeChain code1 = translate_Cond(exp->next[0], label1, label_false);
		InterCodeChain code2 = labelCode(label1);
		InterCodeChain code3 = translate_Cond(exp->next[2], label_true,label_false);
		return linkCode3(code1, code2, code3);
	}
	else if(checkNode(exp, 3, 1, type_OR) == true){
		int label1 = newLabel();
		InterCodeChain code1 = translate_Cond(exp->next[0], label_true, label1);
		InterCodeChain code2 = labelCode(label1);
		InterCodeChain code3 = translate_Cond(exp->next[2], label_true,label_false);
		return linkCode3(code1, code2, code3);
	}
	else{
		Operand t1;
		Operand c1 = newConst(0);
		InterCodeChain code1 = translate_Exp(exp, &t1);
		return linkCode3(
				code1,
				relopCode(t1, c1, label_true, "!="),
				gotoCode(label_false)
				);
	}
}

/* Exp */
InterCodeChain translate_Exp(TreeNode* exp, Operand* place){
	InterCodeChain ret = NULL;
	
	if(exp->num_nexts == 1 && 
			exp->next[0]->type_syn == type_ID){ //处理符号
		int idIndex = exp->next[0]->symbolTableIndex;
		Symbol* idSym = getFromSymbolTable(idIndex);
		bool isAddr = false;
		if(idSym->type_var->kind != BASIC && idSym->isParam)
			isAddr = true;
		if(place != NULL){
			*place = newVar(idIndex, isAddr);
		}
		return NULL;
	}
	else if(exp->num_nexts > 1 &&
			exp->next[0]->type_syn == type_ID){ //处理函数
		Operand ans = newTemp(false);
		char* funcName = exp->next[0]->token.type_id;
		if(exp->num_nexts == 3){
			InterCodeChain retCode = NULL;
			if(strcmp(funcName, "read") == 0){
				retCode = readCode(ans);
			}
			else{
				retCode = callCode(ans, funcName);
			}
			if(place != NULL)
				*place = ans;
			return retCode;
		}
		else{
			ArgList argList = NULL;
			InterCodeChain code1 = translate_Args(exp->next[2], &argList);
			if(strcmp(funcName, "write") == 0){
				return linkCode2(code1, writeCode(argList->op));
			}
			else{
				InterCodeChain code2 = NULL;
				while(argList != NULL){
					InterCodeChain arg;
					int opKind;
					if( argList->opType->kind != BASIC)
						opKind = ADDR_OP;
					else
						opKind = VAL_OP;
					arg = argCode(argList->op, opKind);
					code2 = linkCode2(code2, arg);
					argList = argList->next;
				}
				InterCodeChain code3 = callCode(ans, funcName);
				if(place != NULL)
					*place = ans;
				return linkCode3(code1, code2, code3);
			}
		}
	}
	else if(exp->next[0]->type_syn == type_INT){ // 处理整数
		if(place != NULL){
			*place = newConst(exp->next[0]->token.type_int);
			return NULL;
		}
	}
	else if(exp->next[0]->type_syn == type_FLOAT){ // 处理浮点数
		printf("float not handled yet\n");
		exit(0);
	}
	else if(exp->next[1]->type_syn == type_ASSIGNOP){ //处理等号
		Operand left, right;
		InterCodeChain code1 = translate_Exp(exp->next[2], &right);
		InterCodeChain code2 = translate_Exp(exp->next[0], &left);
		InterCodeChain code3 = assignCode(left, right, VAL_OP);
		return linkCode3(code1, code2, code3);
	}
	else if(exp->next[0]->type_syn == type_MINUS){ //处理取负操作
		Operand t1;
		InterCodeChain code1 = translate_Exp(exp->next[1], &t1);
		/* 针对常数的优化 */
		if(t1->kind == CONST){
			t1->value = -t1->value;
			if(place != NULL)
				*place = t1;
			return NULL;
		}
		*place = newTemp(false);
		Operand c0 = newConst(0);
		InterCodeChain code2 = subCode(*place, c0, t1, VAL_OP);
		return linkCode2(code1, code2);
	}
	else if(exp->next[0]->type_syn == type_NOT ||
			exp->next[1]->type_syn == type_AND ||
			exp->next[1]->type_syn == type_OR ||
			exp->next[1]->type_syn == type_RELOP ){ //处理逻辑运算 
		int label1 = newLabel();
		int label2 = newLabel();
		Operand ans = newTemp(false);
		Operand c0 = newConst(0);
		Operand c1 = newConst(1);
		InterCodeChain code0 = assignCode(ans, c0, VAL_OP);
		InterCodeChain code1 = translate_Cond(exp, label1, label2);
		InterCodeChain code2 = linkCode2(labelCode(label1),
				assignCode(ans, c1, VAL_OP));
		return linkCode4(code0, code1, code2, labelCode(label2));
	}
	else if(exp->next[1]->type_syn == type_PLUS ||
			exp->next[1]->type_syn == type_MINUS ||
			exp->next[1]->type_syn == type_STAR ||
			exp->next[1]->type_syn == type_DIV 
			){ //处理四则运算
		Operand t1, t2;
		InterCodeChain code1 = translate_Exp(exp->next[0], &t1);
		InterCodeChain code2 = translate_Exp(exp->next[2], &t2);
		InterCodeChain code3;
		*place = newTemp(false);
		switch(exp->next[1]->type_syn){
		case type_PLUS:
			code3 = addCode(*place, t1, t2, VAL_OP);
			break;
		case type_MINUS:
			code3 = subCode(*place, t1, t2, VAL_OP);
			break;
		case type_STAR:
			code3 = starCode(*place, t1, t2, VAL_OP);
			break;
		case type_DIV:
			code3 = divCode(*place, t1, t2, VAL_OP);
			break;
		}
		return linkCode3(code1, code2, code3);
	}
	else if(exp->next[0]->type_syn == type_LP){ //处理括号
		return translate_Exp(exp->next[1], place);
	}
	else if(exp->next[1]->type_syn == type_LB){ //处理数组操作
		Operand base;
		Operand num;
		InterCodeChain code1 = translate_Exp(exp->next[0], &base);
		InterCodeChain code2 = translate_Exp(exp->next[2], &num);
		int typeIndex = exp->next[0]->typeTableIndex;
		Type* elemType = getFromTypeTable(typeIndex)->array.elem;
		int elemSize = typeSize(elemType);
		Operand offset = newTemp(false); // save the offset
		InterCodeChain code3 = starCode(
				offset, newConst(elemSize), num, VAL_OP);
		Operand ans = newTemp(true);
		InterCodeChain code4 = addCode(ans, base, offset, ADDR_OP);
		if(place != NULL)
			*place = ans;
		return linkCode4(code1, code2, code3, code4);
	}
	else if(exp->next[1]->type_syn == type_DOT){ //处理结构体访问
		Operand base;
		InterCodeChain code1 = translate_Exp(exp->next[0], &base);
		int typeIndex = exp->next[0]->typeTableIndex;
		Type* type = getFromTypeTable(typeIndex);
		int offset = getOffsetFromStructure(type, 
				exp->next[2]->token.type_id);
		Operand ans = newTemp(true);
		InterCodeChain code2 = addCode(ans, base, 
				newConst(offset), ADDR_OP);
		if(place != NULL)
			*place = ans;
		return linkCode2(code1, code2);
	}
}

/* structrue offset */
int getOffsetFromStructure(Type* structure, char* name){
	FieldList* head = structure->structure;
	int offset = 0;
	while(strcmp(head->name, name) != 0){
		offset += typeSize(head->type);
		head = head->tail;
	}
	return offset;
}

/* Args */
InterCodeChain translate_Args(TreeNode* args, ArgList* argList){
	if(args->num_nexts == 1){
		Operand t1;
		InterCodeChain code1 = translate_Exp(args->next[0], &t1);
		ArgList arg1 = malloc(sizeof(struct ArgList_));
		arg1->op = t1;
		arg1->opType = getFromTypeTable(args->next[0]->typeTableIndex);
		arg1->next = *argList;
		*argList = arg1;
		return code1;
	}
	else{
		Operand t1;
		InterCodeChain code1 = translate_Exp(args->next[0], &t1);
		ArgList arg1 = malloc(sizeof(struct ArgList_));
		arg1->op = t1;
		arg1->opType = getFromTypeTable(args->next[0]->typeTableIndex);
		arg1->next = *argList;
		*argList = arg1;
		InterCodeChain code2 = translate_Args(args->next[2], argList);
		return linkCode2(code1, code2);
	}
}

/* VarList */
InterCodeChain translate_VarList(TreeNode* varList){
	InterCodeChain code1 = translate_ParamDec(varList->next[0]);
	if(varList->num_nexts == 1)
		return code1;
	else
		return linkCode2(code1,translate_VarList(varList->next[2]));
}

/* 从VarDec中获取标志符 */
TreeNode* getIDFromVarDec(TreeNode* varDec){
	while(varDec->next[0]->type_syn != type_ID){
		varDec = varDec->next[0];
	}
	return varDec->next[0];
}

/* ParamDec */
InterCodeChain translate_ParamDec(TreeNode* paramDec){
	TreeNode* id = getIDFromVarDec(paramDec->next[1]);
	int index = id->symbolTableIndex;
	Operand op = newVar(index, true);
	return paramCode(op);
}
