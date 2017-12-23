%{
	#include"syntax.h"
	static int errors=0;// 用来统计出错的个数
	char TYPENAME[2][10]={"int","float"};
%}

/* 启用位置功能 */
%locations

/* declared tokens */
%token INT
%token FLOAT
%token ID
%token SEMI
%token COMMA
%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT SINGLEMINUS
%left DOT LP RP LB RB
%token TYPE
%token LC
%token RC
%token STRUCT
%token RETURN
%token IF
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%token WHILE

%%
/* High-level Definitions */
Program : ExtDefList { BuiltTree1(Program); 
		if(!errors){ 
			if(semanteme($$))
				translate($$);
		}
		else{
			printf("%d errors have been finded\n", errors);
		}
	}
	;
ExtDefList : ExtDef ExtDefList { BuiltTree2(ExtDefList);  }
	| /* empty */ { BuiltTree0(ExtDefList);  }
	;
ExtDef : Specifier ExtDecList SEMI { BuiltTree3(ExtDef);  }
	| Specifier SEMI { BuiltTree2(ExtDef);  }
	| Specifier FunDec SEMI{ BuiltTree3(ExtDef);  }
	| Specifier FunDec CompSt { BuiltTree3(ExtDef);  }
	| Specifier ExtDecList error  {
			printError(-1,@2.last_line,@2.last_column,"missing ';'");
			errors++;
			yyerrok;
		}
	| Specifier error {
			printError(-1,@1.last_line,@1.last_column,"missing ';'");
			errors++;
			yyerrok;
		}
	| error SEMI {
			printError(-1,@1.first_line,@1.first_column,"syntax error");
			errors++;
			yyerrok;
		}
	| error '\n' {
			printError(-1,@1.first_line,@1.first_column,"syntax error");
			errors++;
			yyerrok;
		}
	;
ExtDecList : VarDec { BuiltTree1(ExtDecList);  }
	| VarDec COMMA ExtDecList { BuiltTree3(ExtDecList);  }
	| error '\n' {
			printError(-1,@1.first_line,@1.first_column,"syntax error");
			yyerrok;
		}
	;

/* Specifiers */
Specifier : TYPE { BuiltTree1(Specifier);  }
	| StructSpecifier { BuiltTree1(Specifier);  }
	;
StructSpecifier : STRUCT OptTag LC DefList RC { BuiltTree5(StructSpecifier);  }
	| STRUCT Tag { BuiltTree2(StructSpecifier);  }
	;
OptTag : ID { BuiltTree1(OptTag);  }
	| /* empty */ { BuiltTree0(OptTag);  }
	;
Tag :ID { BuiltTree1(Tag);  }
	;

/* Declarators */
VarDec : ID { BuiltTree1(VarDec);  }
	| VarDec LB INT RB { BuiltTree4(VarDec);  }
	| VarDec LB error RB {
			printError(-1,@3.first_line,@3.first_column,"error between '[]'");
			errors++;
			yyerrok;
		}
	| VarDec LB INT error {
			printError(-1,@3.last_line,@3.last_column,"expected ']'");
			errors++;
			yyerrok;
		}
	;
FunDec : ID LP VarList RP { BuiltTree4(FunDec);  }
	| ID LP RP { BuiltTree3(FunDec);  }
	;
VarList : ParamDec COMMA VarList { BuiltTree3(VarList);  }
	| ParamDec { BuiltTree1(VarList);  }
	;
ParamDec : Specifier VarDec { BuiltTree2(ParamDec);  }
	| Specifier error{
			printError(-1,@1.last_line,@1.last_column,"error defination");
			errors++;
			yyerrok;
		}
	| error VarDec{
			printError(-1,@1.last_line,@1.last_column,"error defination");
			errors++;
			yyerrok;
		}
	;

/* Statements */
CompSt : LC DefList StmtList RC { BuiltTree4(CompSt);  }
	| error RC{
			printError(-1,@1.first_line,@1.first_column,"syntax error");
			errors++;
			yyerrok;
		}
	| error STAR DIV DefList StmtList RC{
			printError(-1,@1.last_line,@1.last_column,"unexpected '*/'");
			errors++;
			yyerrok;
		}
	;
StmtList : Stmt StmtList { BuiltTree2(StmtList);  }
	| /* empty */ { BuiltTree0(StmtList);  }
	;
Stmt : Exp SEMI  { BuiltTree2(Stmt);  }
	| CompSt { BuiltTree1(Stmt);  }
	| RETURN Exp SEMI { BuiltTree3(Stmt);  }
	| IF LP Exp RP Stmt %prec LOWER_THAN_ELSE { BuiltTree5(Stmt);  }
	| IF LP Exp RP Stmt ELSE Stmt { BuiltTree7(Stmt);  }
	| WHILE LP Exp RP Stmt { BuiltTree5(Stmt);  }
	| error Stmt{
			printError(-1,@1.last_line,@1.last_column,"expected ';'");
			errors++;
			yyerrok;
		}
	| error ELSE Stmt{
			printError(-1,@2.first_line,@2.first_column,"expected ';'");
			errors++;
			yyerrok;
		}
	| Exp error{
			printError(-1,@1.last_line,@1.last_column,"expected ';'");
			errors++;
			yyerrok;
		}
	| RETURN Exp error{
			printError(-1,@2.last_line,@2.last_column,"expected ';'");
			errors++;
			yyerrok;
		}
	| error SEMI{
			printError(-1,@1.first_line,@1.first_column,"syntax error");
			errors++;
			yyerrok;
		}
	| error '\n' {
			printError(-1,@1.first_line,@1.first_column,"syntax error");
			errors++;
			yyerrok;
		}
	;

/* Local Definitions */
DefList : Def DefList { BuiltTree2(DefList);  }
	| /* empty */ { BuiltTree0(DefList);  }
	;
Def : Specifier DecList SEMI { BuiltTree3(Def);  }
	| Specifier DecList error{
			printError(-1,@2.last_line,@2.last_column,"expected ';'");
			errors++;
			yyerrok;
		}
	| Specifier error '\n' {
			printError(-1,@1.first_line,@1.first_column,"syntax error");
			errors++;
			yyerrok;
		}
	| Specifier error SEMI {
			printError(-1,@1.first_line,@1.first_column,"syntax error");
			errors++;
			yyerrok;
		}
	;
DecList : Dec { BuiltTree1(DecList);  }
	| Dec COMMA DecList { BuiltTree3(DecList);  }
	;
Dec : VarDec { BuiltTree1(Dec);  }
	| VarDec ASSIGNOP Exp { BuiltTree3(Dec);  }
	;

/* Expressions */
Exp : Exp ASSIGNOP Exp { BuiltTree3(Exp);  }
	| Exp AND Exp { BuiltTree3(Exp);  }
	| Exp OR Exp { BuiltTree3(Exp);  }
	| Exp RELOP Exp { BuiltTree3(Exp);  }
	| Exp PLUS Exp { BuiltTree3(Exp);  }
	| Exp MINUS Exp { BuiltTree3(Exp);  }
	| Exp STAR Exp { BuiltTree3(Exp);  }
	| Exp DIV Exp { BuiltTree3(Exp);  }
	| LP Exp RP { BuiltTree3(Exp);  }
	| MINUS Exp %prec SINGLEMINUS { BuiltTree2(Exp);  }
	| NOT Exp { BuiltTree2(Exp);  }
	| ID LP Args RP { BuiltTree4(Exp);  }
	| ID LP RP { BuiltTree3(Exp);  }
	| Exp LB Exp RB { BuiltTree4(Exp);  }
	| Exp DOT ID { BuiltTree3(Exp);  }
	| ID { BuiltTree1(Exp);  }
	| INT { BuiltTree1(Exp);  }
	| FLOAT { BuiltTree1(Exp);  }
	| Exp LB error RB{
			printError(-1,@3.first_line,@3.first_column,"error between '[]'");
			errors++;
			yyerrok;
		}
	;
Args : Exp COMMA Args { BuiltTree3(Args);  }
	| Exp { BuiltTree1(Args);  }
	;

%%

#include "lex.yy.c"
yyerror(char* msg) {
	errors++;
//	fprintf(stderr, "error: %s\n", msg);
}



/* 打印token额外的信息 */
print_extra_info(TreeNode* node){
	if(strcmp(node->name,"ID")==0){
		printf(": %s", node->token.type_id);
	}
	if(strcmp(node->name,"INT")==0){
		printf(": %d", node->token.type_int);
	}
	if(strcmp(node->name,"FLOAT")==0){
		printf(": %f", node->token.type_float);
	}
	if(strcmp(node->name,"TYPE")==0){
		printf(": %s", TYPENAME[node->token.type_int]);
	}
}

/* 打印缩进部分 */
void print_indent(int indent){
	int i;
	for(i=0; i<indent; i++){
		printf("  ");
	}
}

/* 根据语法树和缩进信息打印语法树 */
void treePrint(TreeNode* root,int indent){
	if(root->type_node == TYPESYMBOL && root->num_nexts>0){
		print_indent(indent);	
		printf("%s (%d)\n",root->name,root->symbol.line);
		/* 递归打印子树 */
		int i;
		for(i=0; i< root->num_nexts; i++){
			treePrint(root->next[i],indent+1);
		}
		return;
	}
	else if(root->type_node == TYPETOKEN){
		print_indent(indent);
		printf("%s",root->name);
		print_extra_info(root);	
		printf("\n");
	}
}


