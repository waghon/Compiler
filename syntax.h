#ifndef SYNTAX_H
#define SYNTAX_H
	/* some direct defination */
	#include<stdio.h>
	#include<string.h>
	#include"helper.h"
	#include"syntaxTree.h"
	#include "semanteme.h"
	#include "translate.h"
	
	// 函数声明 
	void treePrint(TreeNode*,int);
		

	//修改默认的YYSTYPE类型
	#define YYSTYPE TreeNode*
	
	// 定义构建语法树的操作，用宏定义来实现，根据子树节点不同, \
		定义不同的宏定义,例如 Built_Tree_2 表示2个子节点
	#define BuiltTree0(Name) \
	/* 创建根结点 */ \
	yyval = malloc(sizeof(TreeNode)); \
	/* 设置根结点的属性 */ \
	yyval->type_node = TYPESYMBOL; \
	yyval->type_syn = concat(type_,Name); \
	yyval->symbol.column = yyloc.first_column; \
	strcpy(yyval->name,#Name); \
	yyval->symbol.line = yyloc.first_line; \
	/* 将子节点插入 */ \
	yyval->num_nexts = 0

	#define BuiltTree1(Name) \
	/* 创建根结点 */ \
	yyval = malloc(sizeof(TreeNode)); \
	/* 设置根结点的属性 */ \
	yyval->type_node = TYPESYMBOL; \
	yyval->type_syn = concat(type_,Name); \
	yyval->symbol.column = yyloc.first_column; \
	strcpy(yyval->name,#Name); \
	yyval->symbol.line = yyloc.first_line; \
	/* 将子节点插入 */ \
	yyval->num_nexts = 1; \
	yyval->next[0] = (yyvsp[0])
	
	#define BuiltTree2(Name) \
	/* 创建根结点 */ \
	yyval = malloc(sizeof(TreeNode)); \
	/* 设置根结点的属性 */ \
	yyval->type_node = TYPESYMBOL; \
	yyval->type_syn = concat(type_,Name); \
	yyval->symbol.column = yyloc.first_column; \
	strcpy(yyval->name,#Name); \
	yyval->symbol.line = yyloc.first_line; \
	/* 将子节点插入 */ \
	yyval->num_nexts = 2; \
	yyval->next[0] = (yyvsp[-1]); \
	yyval->next[1] = (yyvsp[0])
	
	#define BuiltTree3(Name) \
	/* 创建根结点 */ \
	yyval = malloc(sizeof(TreeNode)); \
	/* 设置根结点的属性 */ \
	yyval->type_node = TYPESYMBOL; \
	yyval->type_syn = concat(type_,Name); \
	yyval->symbol.column = yyloc.first_column; \
	strcpy(yyval->name,#Name); \
	yyval->symbol.line = yyloc.first_line; \
	/* 将子节点插入 */ \
	yyval->num_nexts = 3; \
	yyval->next[0] = (yyvsp[-2]); \
	yyval->next[1] = (yyvsp[-1]); \
	yyval->next[2] = (yyvsp[0])
	
	#define BuiltTree4(Name) \
	/* 创建根结点 */ \
	yyval = malloc(sizeof(TreeNode)); \
	/* 设置根结点的属性 */ \
	yyval->type_node = TYPESYMBOL; \
	yyval->type_syn = concat(type_,Name); \
	yyval->symbol.column = yyloc.first_column; \
	strcpy(yyval->name,#Name); \
	yyval->symbol.line = yyloc.first_line; \
	/* 将子节点插入 */ \
	yyval->num_nexts = 4; \
	yyval->next[0] = (yyvsp[-3]); \
	yyval->next[1] = (yyvsp[-2]); \
	yyval->next[2] = (yyvsp[-1]); \
	yyval->next[3] = (yyvsp[0])
	
	#define BuiltTree5(Name) \
	/* 创建根结点 */ \
	yyval = malloc(sizeof(TreeNode)); \
	/* 设置根结点的属性 */ \
	yyval->type_node = TYPESYMBOL; \
	yyval->type_syn = concat(type_,Name); \
	yyval->symbol.column = yyloc.first_column; \
	strcpy(yyval->name,#Name); \
	yyval->symbol.line = yyloc.first_line; \
	/* 将子节点插入 */ \
	yyval->num_nexts = 5; \
	yyval->next[0] = (yyvsp[-4]); \
	yyval->next[1] = (yyvsp[-3]); \
	yyval->next[2] = (yyvsp[-2]); \
	yyval->next[3] = (yyvsp[-1]); \
	yyval->next[4] = (yyvsp[0])
	
	#define BuiltTree6(Name) \
	/* 创建根结点 */ \
	yyval = malloc(sizeof(TreeNode)); \
	/* 设置根结点的属性 */ \
	yyval->type_node = TYPESYMBOL; \
	yyval->type_syn = concat(type_,Name); \
	yyval->symbol.column = yyloc.first_column; \
	strcpy(yyval->name,#Name); \
	yyval->symbol.line = yyloc.first_line; \
	/* 将子节点插入 */ \
	yyval->num_nexts = 6; \
	yyval->next[0] = (yyvsp[-5]); \
	yyval->next[1] = (yyvsp[-4]); \
	yyval->next[2] = (yyvsp[-3]); \
	yyval->next[3] = (yyvsp[-2]); \
	yyval->next[4] = (yyvsp[-1]); \
	yyval->next[5] = (yyvsp[0])
	
	#define BuiltTree7(Name) \
	/* 创建根结点 */ \
	yyval = malloc(sizeof(TreeNode)); \
	/* 设置根结点的属性 */ \
	yyval->type_node = TYPESYMBOL; \
	yyval->type_syn = concat(type_,Name); \
	yyval->symbol.column = yyloc.first_column; \
	strcpy(yyval->name,#Name); \
	yyval->symbol.line = yyloc.first_line; \
	/* 将子节点插入 */ \
	yyval->num_nexts = 7; \
	yyval->next[0] = (yyvsp[-6]); \
	yyval->next[1] = (yyvsp[-5]); \
	yyval->next[2] = (yyvsp[-4]); \
	yyval->next[3] = (yyvsp[-3]); \
	yyval->next[4] = (yyvsp[-2]); \
	yyval->next[5] = (yyvsp[-1]); \
	yyval->next[6] = (yyvsp[0])

#endif
