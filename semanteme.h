#ifndef SEMANTEME_H
#define SEMANTEME_H

#include "syntaxTree.h"
#include "helper.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define MAX_FIELD_FLOOR 20
#define MAX_HASH_TABLE 555
#define hashFunc(name) (hash_pjw(name)%MAX_HASH_TABLE)

/* 定义语义分析中用到的结构 */

typedef struct Type_ Type;
typedef struct FieldList_ FieldList;
typedef struct TypeList_ TypeList;
typedef struct Symbol_ Symbol;
typedef struct CrossChainNode_ CrossChainNode;
/* 类型的定义 */
struct Type_
{
	enum { BASIC, ARRAY, STRUCTURE } kind;
	union
	{
		// 基本类型
		int basic;
		// 数组类型信息包括元素类型与数组大小构成
		struct { Type* elem; int size; } array;
		// 结构体类型信息是一个链表
		FieldList* structure;
	};
};
/* 结构体中的域的定义 */
struct FieldList_
{
	char* name; // 域的名字
	Type* type; // 域的类型
	FieldList* tail; // 下一个域
};
/* 函数参数列表的定义 */
struct TypeList_
{
	char *name; // 名字
	Type* type; // 域的类型
	TypeList* tail; // 下一个域
};
/* 符号的定义 */
struct Symbol_
{
	char *name; // 符号的名字
	int line; //符号定义所在行号
	int column; //符号定义多在列号
	int index; //在符号表中的索引
	bool isParam; //是否是函数的参数
	enum { VARIABLE, FUNCTION, STRUCTURE_DEFINE } kind;// 符号的类型，主要是三类，变量，函数，和结构体的定义
	union{
		struct { // 变量的类型
			Type* type_var;
			bool initialized;
		};
		struct{ // 函数的符号表示，需要返回类型和参数个数及类型
			bool isFuncDefined;
			Type* type_func_ret;
			int func_varlist_size;
			TypeList* type_func_varlist;
		};
		Type* type_structure; // 结构体的类型
	};
};
/* 十字链表节点的定义 */
struct CrossChainNode_{
	Symbol* symbol;
	CrossChainNode* rightSide;
	CrossChainNode* underSide;
};

/* 函数接口 */
void clearSymbolTable();
bool semanteme(TreeNode* root); 
void synSemanteme(TreeNode* root,Type* funcreturn);
void doBranchHandle(TreeNode* root,Type* funcReturn);
void doDef(TreeNode* root);
void doExtDef(TreeNode* root,Type* funcReturn);
void doStmt(TreeNode* root,Type* funcReturn);
Type* doExp(TreeNode* root);
Type* doSpecifier(TreeNode* root);
Type* doStructSpecifier(TreeNode* root);
FieldList* getFieldList(TreeNode* root);
void doExtDecList(Type* type,TreeNode* root);
void doFunDec(Type* type,TreeNode* root,bool isDefined);
void checkFuncDefined();
bool  isFuncConsistent(Symbol* a,Symbol* b);
void doDecList(Type* type,TreeNode* root);
void doDec(Type* type,TreeNode* root);
Type* doVarDec(Type* type,TreeNode* root,bool initialized,bool isPut,bool isParam);
TypeList* doVarList(TreeNode* root,bool isPutSymbolTable);
int getVarlistSize(TypeList* varlist);
Symbol* getSymbol(char* name);
Type* doExpID(TreeNode* root);
Type* doExpFunc(TreeNode* root);
Type* doExpINT(TreeNode* root);
Type* doExpFLOAT(TreeNode* root);
Type* doExpAssignop(TreeNode* root);
Type* doExpNOT(TreeNode* root);
Type* doExpMinus(TreeNode* root);
Type* doExp_AND_OR(TreeNode* root);
Type* doExpRelop(TreeNode* root);
Type* doExpFundOp(TreeNode* root);
Type* doExpLB(TreeNode* root);
Type* doExpDOT(TreeNode* root);
Type* generateIntType();
TypeList* doFuncArgs(TreeNode* root);
bool typeCmp(Type* type1,Type* type2);
bool leftAssignopOk(TreeNode* root);
void inCompSt();
void outCompSt();
void putASymbol(Symbol* symbol);
bool symbolExit(Symbol* symbol);
int putIntoTypeTable(Type* type);
int putIntoSymbolTable(Symbol* symbol);
Type* getFromTypeTable(int index);
Symbol* getFromSymbolTable(int index);
void putReadWriteFunction();

#endif
