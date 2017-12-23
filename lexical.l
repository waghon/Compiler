%{
/* 这部分可以写一些需要直接拷贝的声明*/	
#include "syntax.tab.h"
extern int errors;

/* 用于记录行号和列号 */
int yycolumn = 1;
#define YY_USER_ACTION \
yylloc.first_line = yylloc.last_line = yylineno; \
yylloc.first_column = yycolumn; \
yylloc.last_column = yycolumn + yyleng - 1; \
yycolumn += yyleng;

#define common_ops(TokenNode) \
yylval = malloc(sizeof(TreeNode)); \
yylval->type_node = TYPETOKEN; \
yylval->type_syn = concat(type_,TokenNode); \
yylval->num_nexts=0; \
yylval->token.line = yylloc.first_line; \
yylval->token.column = yylloc.first_column; \
strcpy(yylval->name,#TokenNode);

/* 通过宏定义来简化操作，将每个部分共有的功能提取出来*/
#define do_ops(TokenNode) \
common_ops(TokenNode) \
return TokenNode
%}

%option yylineno

digit [0-9]
octalDigit [0-7]
hexDigit [0-9A-Fa-f]
letter [a-zA-Z]

INT 0|[1-9]{digit}*
OCTALINT 0{octalDigit}+
HEXINT 0[xX]{hexDigit}+
FLOAT ({digit}+\.{digit}*|\.{digit}+)([eE][+-]?{digit}+)?
SEMI ;
COMMA ,
ASSIGNOP =
RELOP >|<|>=|<=|==|!=
PLUS \+
MINUS -
STAR \*
DIV \/
AND &&
OR \|\|
DOT \.
NOT !
TYPE int|float
LP \(
RP \)
LB \[
RB \]
LC \{
RC \}
STRUCT struct
RETURN return
IF if
ELSE else
WHILE while
ID (_|{letter})(_|{letter}|{digit})*

INCORRECTOCTALHEX 0{octalDigit}*[89][digit]*|0[xX]{hexDigit}*[g-zG-Z]{letter}*
INCORRECTFLOAT ({digit}+\.{digit}*|\.{digit}+)[eE]
SINGLENOTE \/\/
MULTINOTE \/\*[^*]*(\*([^/][^*]*)?)*\*\/

EMPTY " "|"	"
ENTER \n

%%
{INT} { 
	common_ops(INT);
	sscanf(yytext,"%d", &yylval->token.type_int);
	return INT;  }
{OCTALINT} { 
	common_ops(INT);
	sscanf(yytext,"%o", &yylval->token.type_int);
	return INT;  }
{HEXINT} { 
	common_ops(INT);
	sscanf(yytext,"%x", &yylval->token.type_int);
	return INT;  }
{FLOAT} { 
	common_ops(FLOAT);
	sscanf(yytext,"%f", &yylval->token.type_float);
	return FLOAT;  }
{SEMI} { do_ops(SEMI);  } 
{COMMA} { do_ops(COMMA);  } 
{ASSIGNOP} { do_ops(ASSIGNOP);  } 
{RELOP} {  
	common_ops(RELOP);
	strcpy(yylval->token.type_relop,yytext);
	return RELOP;  }
{PLUS} { do_ops(PLUS);  } 
{MINUS} { do_ops(MINUS);  } 
{STAR} { do_ops(STAR);  } 
{DIV} { do_ops(DIV);  } 
{AND} { do_ops(AND);  } 
{OR} { do_ops(OR);  } 
{DOT} { do_ops(DOT);  } 
{NOT} { do_ops(NOT);  } 
{TYPE} {
	common_ops(TYPE); 
	if(strcmp("int",yytext)==0) yylval->token.type_int = TYPEINT;
	else yylval->token.type_int = TYPEFLOAT;
	return TYPE;  }
{LP} { do_ops(LP);  } 
{RP} { do_ops(RP);  } 
{LB} { do_ops(LB);  } 
{RB} { do_ops(RB);  } 
{LC} { do_ops(LC);  } 
{RC} { do_ops(RC);  } 
{STRUCT} { do_ops(STRUCT);  } 
{RETURN} { do_ops(RETURN);  } 
{IF} { do_ops(IF);  } 
{ELSE} { do_ops(ELSE);  } 
{WHILE} { do_ops(WHILE);  } 
{ID} { 
	common_ops(ID);
	strcpy(yylval->token.type_id,yytext);
	return ID;  }
{EMPTY} {}
{ENTER} { yycolumn = 1; }
{SINGLENOTE} { 
	char c = input();
	while(c != '\n') c=input();
  }
{MULTINOTE} {}
{INCORRECTOCTALHEX}  { 
	errors++;
	printf("Error type A at Line %d: Incorrect number \'%s\'\n",
	yylineno, yytext);
	return ID;	
  }
{INCORRECTFLOAT} { 
	errors++;
	printf("Error type A at Line %d: Incorrect floating point number \'%s\'\n",yylineno, yytext);
	return FLOAT;
	}
. { 
	errors++;
	printf("Error type A at Line %d: Mysterious characters \'%s\'\n",
	yylineno, yytext);  
	} 
