#include "semanteme.h"

/* 变量定义 */
static int semanError=0;//记录错误的个数
CrossChainNode* fieldFloor[MAX_FIELD_FLOOR]; //定义域层
CrossChainNode* symbolHashTable[MAX_HASH_TABLE]; // 符号的hash表 
int lastField = 0; // 最上层的field的下标
int inField = 0; //为了标志是否在struct内部

/* 将一些类型和符号保存下来,
 * 在语法树中存下索引号就可以了
 * 这里主要是为了生成代码准备，不需要再构建复杂的符号表
 * 在线性符号表中查找就可以了 */
#define MAX_TYPE_TABLE 5000
#define MAX_SYMBOL_TABLE 5000
Type* typeTable[MAX_TYPE_TABLE];
Symbol* symbolTable[MAX_SYMBOL_TABLE];
int typeTableTop = 0;
int symbolTableTop = 0;

/* 清理符号表，初始化一些参数 */
void clearSymbolTable(){
	int i;
	lastField = 0;
	inField = 0;
	typeTableTop=0;
	symbolTableTop=0;
	for(i = 0; i < MAX_FIELD_FLOOR; i++){
		fieldFloor[i] = NULL;
	}
	for(i=0; i < MAX_HASH_TABLE; i++){
		symbolHashTable[i] = NULL;
	}
}

/* 进行语义分析 */
bool semanteme(TreeNode* root){
	clearSymbolTable();
	putReadWriteFunction();
	synSemanteme(root,NULL);
	checkFuncDefined();
	if(semanError>0){
		printf("%d errors have been finded\n", semanError);
		return false;
	}
	return true;
}

void putReadFunction();
void putWriteFunction();

/* 为生成代码需要，加入读写函数 */
void putReadWriteFunction(){
	putReadFunction();
	putWriteFunction();
}
void putReadFunction(){
	Type* intType = generateIntType();
	Symbol* fundec = malloc(sizeof(Symbol));
	fundec->name = "read";
	fundec->line = 1;
	fundec->column = 1;
	fundec->kind = FUNCTION;
	fundec->isFuncDefined = true;
	fundec->type_func_ret = intType;
	fundec->type_func_varlist = NULL;
	fundec->func_varlist_size = 0;
	putASymbol(fundec);
}
void putWriteFunction(){
	Type* intType = generateIntType();
	Symbol* fundec = malloc(sizeof(Symbol));
	fundec->name = "write";
	fundec->line = 1;
	fundec->column = 1;
	fundec->kind = FUNCTION;
	fundec->isFuncDefined = true;
	fundec->type_func_ret = intType;
	TypeList* funcVarList = malloc(sizeof(TypeList));
	funcVarList->name = "unknow";
	funcVarList->type = intType;
	funcVarList->tail = NULL;
	fundec->type_func_varlist = funcVarList;
	fundec->func_varlist_size = 1;
	putASymbol(fundec);
}

/* 递归，对每个分支进行语义分析 */
void synSemanteme(TreeNode* root,Type* funcReturn){
	if(root->type_node==TYPETOKEN) // token不需要处理
		return ;
	switch(root->type_syn){
	case type_Def:
		doDef(root);
		break;
	case type_Exp:
		doExp(root);
		break;
	case type_ExtDef:
		doExtDef(root,funcReturn);
		break;
	case type_Stmt:
		doStmt(root,funcReturn);
		break;
	default:
		doBranchHandle(root,funcReturn);
		break;
	}
}
/* 处理Def分支 */
void doDef(TreeNode* root){
	debug(root->name);
	Type* type = doSpecifier(root->next[0]);
	doDecList(type,root->next[1]);
}
/* 处理外部Def分支 */
void doExtDef(TreeNode* root,Type* funcReturn){
	debug(root->name);
	Type* type = doSpecifier(root->next[0]);
	switch(root->next[1]->type_syn){
	case type_ExtDecList:
		doExtDecList(type,root->next[1]);
		break;
	case type_SEMI:
		break;
	case type_FunDec:
		if(root->next[2]->type_syn == type_CompSt){
			doFunDec(type,root->next[1],true);
			inCompSt();
			if(root->next[1]->num_nexts == 4){
				doVarList(root->next[1]->next[2],true);
			}
			doBranchHandle(root->next[2],type);
			outCompSt();
		}
		else{
			doFunDec(type,root->next[1],false);
		}
		break;
	}
}
/* 处理语句的分支 */
void doStmt(TreeNode* root,Type* funcReturn){
	debug(root->name);
	Type* temp;
	switch(root->next[0]->type_syn){
	case type_Exp:
		doExp(root->next[0]);
		break;
	case type_CompSt:
		inCompSt();
		doBranchHandle(root->next[0],funcReturn);
		outCompSt();
		break;
	case type_RETURN:
		debug("return handle");
		temp = doExp(root->next[1]);
		if(temp != NULL && typeCmp(temp,funcReturn) == false){
			printError(8,root->next[0]->token.line,root->next[0]->token.column,"Type mismatched for return.");
			semanError++;
		}
		break;
	case type_IF:
		temp = doExp(root->next[2]);
		if(temp != NULL &&
			   	!(temp->kind == BASIC && temp->basic == TYPEINT)){
			printError(20,root->next[2]->symbol.line,root->next[2]->symbol.column,"wrong type for 'if'");
			semanError++;
		}

		doStmt(root->next[4],funcReturn);
		if(root->num_nexts == 7){
			doStmt(root->next[6],funcReturn);
		}
		break;
	case type_WHILE:
		temp = doExp(root->next[2]);
		if(temp != NULL &&
			   	!(temp->kind == BASIC && temp->basic == TYPEINT)){
			printError(20,root->next[2]->symbol.line,root->next[2]->symbol.column,"wrong type for 'while'");
			semanError++;
		}
		doStmt(root->next[4],funcReturn);
		break;
	}
}

/* 处理表达式的分支 */
Type* doExp(TreeNode* root){
	debug(root->name);
	Type* ret;
	if(root->num_nexts == 1 && 
			root->next[0]->type_syn == type_ID){ //处理符号
		ret = doExpID(root);
	}
	else if(root->num_nexts > 1 &&
			root->next[0]->type_syn == type_ID){ //处理函数
		ret = doExpFunc(root);
	}
	else if(root->next[0]->type_syn == type_INT){ // 处理整数
		ret = doExpINT(root);
	}
	else if(root->next[0]->type_syn == type_FLOAT){ // 处理整数
		ret = doExpFLOAT(root);
	}
	else if(root->next[1]->type_syn == type_ASSIGNOP){ //处理等号
		ret = doExpAssignop(root);
	}
	else if(root->next[0]->type_syn == type_NOT){ //处理逻辑非
		ret = doExpNOT(root);
	}
	else if(root->next[0]->type_syn == type_MINUS){ //处理取负操作
		ret = doExpMinus(root);
	}
	else if(root->next[1]->type_syn == type_AND ||
			root->next[1]->type_syn == type_OR){ //处理逻辑操作
		ret = doExp_AND_OR(root);
	}
	else if(root->next[1]->type_syn == type_RELOP){ //处理比较操作
		ret = doExpRelop(root);
	}
	else if(root->next[1]->type_syn == type_PLUS ||
			root->next[1]->type_syn == type_MINUS ||
			root->next[1]->type_syn == type_STAR ||
			root->next[1]->type_syn == type_DIV 
			){ //处理四则运算
		ret = doExpFundOp(root);
	}
	else if(root->next[0]->type_syn == type_LP){ //处理括号
		ret = doExp(root->next[1]);
	}
	else if(root->next[1]->type_syn == type_LB){ //处理数组操作
		ret = doExpLB(root);
	}
	else if(root->next[1]->type_syn == type_DOT){ //处理结构体访问
	
		ret = doExpDOT(root);
	}
	root->typeTableIndex = putIntoTypeTable(ret);
	return ret;
}

/* 遍历处理根结点开始的分支 */
void doBranchHandle(TreeNode* root,Type* funcReturn){
	debug(root->name);
	int i;
	for(i=0;i<root->num_nexts;i++){
		synSemanteme(root->next[i],funcReturn);
	}
}

/* 从Specifier中获取类型 */
Type* doSpecifier(TreeNode* root){
	debug(root->name);
	Type* type=malloc(sizeof(Type));
	switch(root->next[0]->type_syn){
	case type_TYPE:
		type->kind=BASIC;
		type->basic=root->next[0]->token.type_int;
		break;
	case type_StructSpecifier:
		type = doStructSpecifier(root->next[0]);
		break;
	}

	return type;
}

/* 处理结构体的定义 */
Type* doStructSpecifier(TreeNode* root){
	if(root->num_nexts == 2){ //已有的结构体
		char* name = root->next[1]->next[0]->token.type_id;
		Symbol* symbol = getSymbol(name);
		if(!(symbol != NULL && symbol->kind == STRUCTURE_DEFINE)){
			char msg[50] = "Undefined structrue";
			printError(17,root->next[1]->symbol.line,
					root->next[1]->symbol.column,
					myStrcat(msg,name));
			semanError++;
			return NULL;
		}
		else{
			return symbol->type_structure;
		}
	}
	else{ //定义新的结构体
		Type* type = malloc(sizeof(Type));
		type->kind = STRUCTURE;
		inCompSt();
		inField++;
		type->structure = getFieldList(root->next[3]);
		inField--;
		outCompSt();
		if(root->next[1]->num_nexts > 0){ //放入结构名
			Symbol* structSym = malloc(sizeof(Symbol));
			structSym->name = root->next[1]->next[0]->token.type_id;
			structSym->line = root->symbol.line;
			structSym->column = root->symbol.column;
			structSym->kind = STRUCTURE_DEFINE;
			structSym->type_structure = type;
			putASymbol(structSym);
		}
	}
}

/* 处理结构体的域 */
FieldList* getFieldList(TreeNode* root){
	TreeNode* defList = root;
	while(defList->num_nexts > 0){ //将所有定义加入栈中
		doDef(defList->next[0]);
		defList = defList->next[1];
	}
	/* 从符号表中提取域符号链 */
	FieldList* retValue = NULL;
	CrossChainNode* symChain = fieldFloor[lastField];
	while(symChain != NULL){
		Symbol* symbol = symChain->symbol;
		if(symbol->kind == VARIABLE){
			FieldList* newNode = malloc(sizeof(FieldList));
			newNode->name = symbol->name;
			newNode->type = symbol->type_var;
			newNode->tail = retValue;
			retValue = newNode;
			if(symbol->initialized){
				printError(15,symbol->line,symbol->column,"A symbol initialized in structure field.");
				semanError++;
			}
		}
		symChain = symChain->underSide;
	}
	return retValue;
}

/* 处理ExtDecList */
void doExtDecList(Type* type,TreeNode* root){
	debug(root->name);
	doVarDec(type,root->next[0],false,true,false);
	if(root->num_nexts==3){
		doExtDecList(type,root->next[2]);
	}
}

/* 处理函数定义 */
void doFunDec(Type* type,TreeNode* root,bool isDefined){
	debug(root->name);
	Symbol* fundec = malloc(sizeof(Symbol));
	fundec->name=root->next[0]->token.type_id;
	fundec->line = root->symbol.line;
	fundec->column = root->symbol.column;
	fundec->kind = FUNCTION;
	fundec->isFuncDefined = isDefined;
	fundec->type_func_ret = type;
	if(root->num_nexts==4){
		fundec->type_func_varlist = doVarList(root->next[2],false);
		fundec->func_varlist_size = getVarlistSize(fundec->type_func_varlist);
	}
	else{
		fundec->type_func_varlist = NULL;
		fundec->func_varlist_size = 0;
	}
	putASymbol(fundec);
}

/* 处理函数的参数 */
TypeList* doVarList(TreeNode* root,bool isPutSymbolTable){
	debug(root->name);
	TypeList* newNode=malloc(sizeof(TypeList));
	TreeNode* paramDec = root->next[0];
	Type* basicType=doSpecifier(paramDec->next[0]);
	newNode->type = doVarDec(basicType,paramDec->next[1],false,false,true);
	if(isPutSymbolTable){
		doVarDec(basicType,paramDec->next[1],false,true,true);
	}
	if(root->num_nexts==1){
		newNode->tail = NULL;
	}
	else{
		newNode->tail = doVarList(root->next[2],isPutSymbolTable);
	}
	return newNode;
}

/* 得到参数的个树 */
int getVarlistSize(TypeList* varlist){
	debug("getVarListSize");
	int size=0;
	while(varlist!=NULL){
		size++;
		varlist=varlist->tail;
	}
	return size;
}

/* 处理DecList放入符号表 */
void doDecList(Type* type,TreeNode* root){
	debug(root->name);
	doDec(type,root->next[0]);
	if(root->num_nexts==3)
		doDecList(type,root->next[2]);
}

/* 处理Dec,放入符号表 */
void doDec(Type* type,TreeNode* root){
	debug(root->name);
	Type* varDec;
	if(root->num_nexts == 3)
		varDec=doVarDec(type,root->next[0],true,true,false); //将符号加入符号表
	else
		varDec=doVarDec(type,root->next[0],false,true,false); //将符号加入符号表

	if(varDec != NULL && root->num_nexts==3){ //判断赋值是否合理
		Type* temp = doExp(root->next[2]);
		if(temp != NULL){
			if(typeCmp(temp,varDec)==false){
				printError(5,root->next[1]->token.line,root->next[1]->token.column,"Type mismatched for assignment.");
				semanError++;
			}
		}
	}
}

/* 处理VarDec,放入符号表,返回VarDec的类型 */
Type* doVarDec(Type* type,TreeNode* root,bool initialized,bool isPut, bool isParam){
	debug(root->name);
	if(root->num_nexts==1){ //处理 ID的形式
		Symbol* newSymbol=malloc(sizeof(Symbol));
		newSymbol->name=root->next[0]->token.type_id;
		newSymbol->line=root->symbol.line;
		newSymbol->column=root->symbol.column;
		newSymbol->kind=VARIABLE;
		newSymbol->type_var=type;
		newSymbol->initialized = initialized;
		newSymbol->isParam = isParam;
		if(isPut){
			putASymbol(newSymbol);
			/* GIVE ID THE INDEX */
			root->next[0]->symbolTableIndex = newSymbol->index;
		}
		return type;
	}
	else{ //处理数组的情况
		Type* newType=malloc(sizeof(Type));
		newType->kind=ARRAY;
		newType->array.size=root->next[2]->token.type_int;
		newType->array.elem=type;
		doVarDec(newType,root->next[0],initialized,isPut,isParam);
	}
}

/* 查询符号表 */
Symbol* getSymbol(char* name){
	debug("getSymbol");
	int index = hashFunc(name);
	CrossChainNode* temp = symbolHashTable[index];
	while(temp != NULL){
		if(strcmp(name,temp->symbol->name)==0)
			return temp->symbol;
		temp = temp->rightSide;
	}
	return NULL;
}

/* 处理表达式中的ID */
Type* doExpID(TreeNode* root){
	debug("doExpID");
	char* ID_Name = root->next[0]->token.type_id;
	Symbol* symbol = getSymbol(ID_Name);
	if(symbol != NULL && symbol->kind == VARIABLE){
		root->next[0]->symbolTableIndex = symbol->index;
		return symbol->type_var;
	}
	else{
		int line = root->symbol.line;
		int column = root->symbol.column;
		char msg[50]="Undefined variable";
		printError(1,line,column,myStrcat(msg,ID_Name));
		semanError++;
		return NULL;
	}
}

/* 处理表达式中的函数 */
Type* doExpFunc(TreeNode* root){
	debug("doExpFunc");
	char* FuncName = root->next[0]->token.type_id;
	Symbol* symbol = getSymbol(FuncName);
	if(symbol != NULL){
		if(symbol->kind != FUNCTION){
			char msg[50];
			sprintf(msg,"'%s' is not a function",FuncName);
			printError(11,root->symbol.line,root->symbol.column,
					msg);
			semanError++;
			return NULL;
		}
		else if(root->num_nexts == 3){ //不带参数
			if(symbol->func_varlist_size != 0){
				char msg[50]="arguments not correct for";
				printError(9,root->next[0]->token.line,root->next[0]->token.column,myStrcat(msg,FuncName));
				printPreDec(symbol->line,
						symbol->column,FuncName);
				semanError++;
			}
			return symbol->type_func_ret;
		}
		else{ //有参数的函数
			TypeList* args = doFuncArgs(root->next[2]);
			int size = getVarlistSize(args);
			bool check = true;
			if(size != symbol->func_varlist_size){
				check = false;
			}
			else{
				TypeList* varlist = symbol->type_func_varlist;
				while(args != NULL && varlist != NULL){
					if(typeCmp(args->type,varlist->type)==false){
						check = false;
						break;
					}
					args = args->tail;
					varlist = varlist->tail;
				}
			}
			if(!check){
				char msg[50]="arguments not correct for";
				printError(9,root->next[0]->token.line,root->next[0]->token.column,myStrcat(msg,FuncName));
				printPreDec(symbol->line,
						symbol->column,FuncName);
				
			}
			return symbol->type_func_ret;
		}
	}
	else{
		int line = root->symbol.line;
		int column = root->symbol.column;
		char msg[50]="Undefined Function";
		printError(2,line,column,myStrcat(msg,FuncName));
		semanError++;
		return NULL;
	}

}

/* 处理函数的参数，返回参数列表 */
TypeList* doFuncArgs(TreeNode* root){
	debug(root->name);
	TypeList* retValue = malloc(sizeof(TypeList));
	Type* type = doExp(root->next[0]);
	retValue->type = type;
	if(root->num_nexts == 1){
		retValue->tail = NULL;
	}
	else{
		retValue->tail = doFuncArgs(root->next[2]);
	}
}

/* 处理表达式中的整数 */
Type* doExpINT(TreeNode* root){
	debug(root->next[0]->name);
	Type* type = malloc(sizeof(Type));
	type->kind = BASIC;
	type->basic = TYPEINT;
	return type;
}

/* 处理表达式中的浮点数 */
Type* doExpFLOAT(TreeNode* root){
	debug(root->next[0]->name);
	Type* type = malloc(sizeof(Type));
	type->kind = BASIC;
	type->basic = TYPEFLOAT;
	return type;
}

/* 处理表达式中的等号 */
Type* doExpAssignop(TreeNode* root){
	debug(root->next[1]->name);
	Type* type1 = doExp(root->next[0]);
	Type* type2 = doExp(root->next[2]);
	if(type1 == NULL)
		return type2;
	else if(type2 == NULL)
		return type1;
	else if(typeCmp(type1,type2)==false){
		printError(5,root->next[1]->token.line,root->next[1]->token.column,"Type mismatched for assignment.");
		semanError++;
	}
	else if(leftAssignopOk(root->next[0])==false){
		printError(6,root->symbol.line,root->symbol.column,"The left-hand side of an assignment must be a variable.");
		semanError++;
	}

	return type1;
}

/* 处理表达式中的[]操作 */
Type* doExpLB(TreeNode* root){
	Type* base = doExp(root->next[0]);
	if(base == NULL){
		return NULL;
	}
	else{
		if(base->kind != ARRAY){
			printError(10,root->symbol.line,root->symbol.column,"This variable is not an array or too  many [] are used.");
			semanError++;
			return base;
		}
		else{
			Type* index = doExp(root->next[2]);
			if(index != NULL && 
					(!(index->kind == BASIC && 
					   index->basic == TYPEINT))){
				printError(12,root->next[2]->symbol.line,
						root->next[2]->symbol.column,
						"integer is expected here");
				semanError++;
			}
			return base->array.elem;
		}
	}
}

/* 访问数组操作 */
Type* doExpDOT(TreeNode* root){
	debug(root->next[1]->name);
	Type* base = doExp(root->next[0]);
	if(base == NULL){
		return NULL;
	}
	else{
		if(base->kind != STRUCTURE){
			printError(13,root->next[1]->token.line,root->next[1]->token.column,"Illegal use of '.'.");
			semanError++;
			return base;
		}
		else{
			char* name = root->next[2]->token.type_id;
			FieldList* fieldHead = base->structure;
			while(fieldHead != NULL){
				if(strcmp(name,fieldHead->name)==0){
					return fieldHead->type;
				}
				fieldHead = fieldHead->tail;
			}
			char msg[50] = "Non-existent field";
			printError(14,root->next[2]->token.line,root->next[2]->token.column,myStrcat(msg,name));
			semanError++;
			return NULL;
			
		}
	}
}

/* 检查两个类型是否相同 */
bool typeCmp(Type* type1,Type* type2){
	debug("typeCmp");
	if(type1->kind != type2->kind)
		return false;
	else if(type1->kind == BASIC){
		return type1->basic == type2->basic;
	}
	else if(type1->kind == ARRAY){
		return typeCmp(type1->array.elem,type2->array.elem);
	}
	else{
		FieldList* structure1 = type1->structure;
		FieldList* structure2 = type2->structure;
		while(structure1 != NULL && structure2 != NULL){
			if(typeCmp(structure1->type,structure2->type)==false)
				return false;
			structure1 = structure1->tail;
			structure2 = structure2->tail;
		}
		if(structure1 != NULL || structure2 != NULL) //有多余的域
			return false;
		return true;
	}
}

/* 检查表达式的逻辑非  */
Type* doExpNOT(TreeNode* root){
	debug(root->next[0]->name);
	Type* type = doExp(root->next[1]);
	if(type != NULL){
		if(!(type->kind == BASIC && type->basic == TYPEINT)){
			printError(7,root->symbol.line,root->symbol.column,"Type mismatched for '!'.");
			semanError++;
		}
	}
	Type* retType = generateIntType();
	return retType;
}

/* 检查表达式的取反 */
Type* doExpMinus(TreeNode* root){
	debug(root->next[0]->name);
	Type* type = doExp(root->next[1]);
	if(type != NULL){
		if(type->kind != BASIC){
			printError(7,root->symbol.line,root->symbol.column,"Type mismatched for '-'.");
			semanError++;
		}
		return type;
	}
	return NULL;
}

/* 检查表达式的逻辑操作 */
Type* doExp_AND_OR(TreeNode* root){
	debug(root->next[1]->name);
	Type* type1 = doExp(root->next[0]);
	Type* type2 = doExp(root->next[2]);
	if(type1 != NULL && type2 != NULL){
		if(typeCmp(type1,type2)==false || 
				!(type1->kind == BASIC && type1->basic == TYPEINT)){
			printError(7,root->next[1]->token.line,root->next[1]->token.column,"Type mismatched for '&& ||'.");
			semanError++;
		}
	}
	Type* retType = generateIntType();
	return retType;
}

/* 处理表达式中的比较运算 */
Type* doExpRelop(TreeNode* root){
	debug(root->next[1]->name);
	Type* type1 = doExp(root->next[0]);
	Type* type2 = doExp(root->next[2]);
	if(type1 != NULL && type2 != NULL){
		if(typeCmp(type1,type2) == false || 
				type1->kind != BASIC){
			printError(7,root->next[1]->token.line,root->next[1]->token.column,"Type mismatched for 'relop'.");
			semanError++;
		}
	}
	Type* retType = generateIntType();
	return retType;	
}

/* 处理表达式中的四则运算 */
Type* doExpFundOp(TreeNode* root){
	debug(root->next[1]->name);
	Type* type1 = doExp(root->next[0]);
	Type* type2 = doExp(root->next[2]);
	if(type1 != NULL && type2 != NULL){
		if(typeCmp(type1,type2) == false || 
				type1->kind != BASIC){
			printError(7,root->next[1]->token.line,root->next[1]->token.column,"Type mismatched for '+-*/'.");
			semanError++;
		}
		else{
			return type1;
		}
	}
	return NULL;	
}

/* 生成int类型的一个符号 */
Type* generateIntType(){
	Type* type = malloc(sizeof(Type));
	type->kind = BASIC;
	type->basic = TYPEINT;
}

/* 检查等号左边部分是否合法 */
bool leftAssignopOk(TreeNode* root){
	if(root->num_nexts == 1 &&
			root->next[0]->type_syn == type_ID)
		return true;
	else if(root->num_nexts == 4 &&
			root->next[0]->type_syn == type_Exp &&
			root->next[1]->type_syn == type_LB &&
			root->next[2]->type_syn == type_Exp &&
			root->next[3]->type_syn == type_RB 
			)
		return true;
	else if(root->num_nexts == 3 &&
			root->next[0]->type_syn == type_Exp &&
			root->next[1]->type_syn == type_DOT &&
			root->next[2]->type_syn == type_ID 
			)
		return true;
	else
		return false;
}

/* 进入CompSt */
void inCompSt(){
	debug("inCompSt");
	lastField++;
	fieldFloor[lastField]=NULL;
}

/* 离开CompSt */
void outCompSt(){
	debug("outCompSt");
	CrossChainNode* temp=fieldFloor[lastField];
	while(temp!=NULL){
		/* 将这一层的符号从hash表中去除 */
		char* symbolName=temp->symbol->name;
		int index=hashFunc(symbolName);
		symbolHashTable[index]=symbolHashTable[index]->rightSide;
		temp=temp->underSide;
	}
	fieldFloor[lastField]=NULL; //清除最后一个域
	lastField--;
}


/* 将一个符号放入符号表中 */
void putASymbol(Symbol* symbol){
	debug(symbol->name);
	if(!symbolExit(symbol)){
		int index=hashFunc(symbol->name);
		CrossChainNode* newNode=malloc(sizeof(CrossChainNode));
		newNode->symbol=symbol;
		newNode->rightSide=symbolHashTable[index];
		newNode->underSide=fieldFloor[lastField];
		symbolHashTable[index]=newNode;
		fieldFloor[lastField]=newNode;
		if(symbol->kind == VARIABLE)
			symbol->index = putIntoSymbolTable(symbol); //放入一个线性的符号表，方便在生成代码的时候查询
	}
}

/* 检查一个符号是否已经存在 */
bool symbolExit(Symbol* symbol){
	debug(symbol->name);
	char* symbolName=symbol->name;	
	CrossChainNode* temp=fieldFloor[lastField];
	while(temp!=NULL){
		if(strcmp(symbolName,temp->symbol->name)==0){
			int errorType;
			char msg[50];
			if(inField>0){
				strcpy(msg,"Redefined field");
				errorType = 15;
			}
			else if(symbol->kind==VARIABLE){
				strcpy(msg,"Redefined variable");
				errorType=3;
			}
			else if(symbol->kind==FUNCTION){
				Symbol* exist = temp->symbol;
				if(exist->isFuncDefined && symbol->isFuncDefined){
					strcpy(msg,"Redefined function");
					errorType=4;
				}
				else{
					if(isFuncConsistent(symbol,exist)){
						if(exist->isFuncDefined || 
								symbol->isFuncDefined) //修改标记
							exist->isFuncDefined = true;
						return true;
					}
					else{
						strcpy(msg,"Inconsistent declaration of function");
						errorType = 19;
					}
				}
			}
			else{
				strcpy(msg,"Duplicated name");
				errorType=16;
			}
			//strcat(msg,symbolName);
			//strcat(msg,"'");
			printError(errorType,symbol->line,symbol->column,
					myStrcat(msg,symbolName));
			semanError++;
			printPreDec(temp->symbol->line,
					temp->symbol->column,symbolName);
			return true;
		}
		temp=temp->underSide;
	}
	return false;
}

/* 检查函数声明的一致性 */
bool  isFuncConsistent(Symbol* a,Symbol* b){
	if(typeCmp(a->type_func_ret,b->type_func_ret) == false)
		return false;
	else if(a->func_varlist_size != b->func_varlist_size){
		return false;
	}
	else{
		TypeList* listA = a->type_func_varlist;
		TypeList* listB = b->type_func_varlist;
		while(listA != NULL && listB != NULL){
			if(typeCmp(listA->type,listB->type)==false)
				return false;
			listA = listA->tail;
			listB = listB->tail;
		}
		return true;
	}
}

/* 检查函数是否定义 */
void checkFuncDefined(){
	CrossChainNode* head = fieldFloor[lastField];
	while(head != NULL){
		Symbol* symbol = head->symbol;
		if(symbol->kind == FUNCTION && (!symbol->isFuncDefined)){
			char msg[50] = "Undefined function";
			myStrcat(msg,symbol->name);
			printError(18,symbol->line,symbol->column,msg);
		}
		head = head->underSide;
	}
}

/* 放入类型表中，返回位置 */
int putIntoTypeTable(Type* type){
	typeTable[typeTableTop] = type;
	int retIndex=typeTableTop;
	typeTableTop++;
	return retIndex;
}

/* 放入符号表中，返回下标 */
int putIntoSymbolTable(Symbol* symbol){
	symbolTable[symbolTableTop] = symbol;
	int retIndex = symbolTableTop;
	symbolTableTop++;
	return retIndex;
}

/* 根据下标查询类型 */
Type* getFromTypeTable(int index){
	return typeTable[index];
}

/* 根据下标查询符号 */
Symbol* getFromSymbolTable(int index){
	return symbolTable[index];
}
