#include<stdio.h>
extern FILE* yyin;
FILE* fileForLine;
extern char* codeFileName;
int main(int argc,char** argv){
	if (argc <= 2) 
		return 1;
	FILE* f = fopen(argv[1], "r");
	fileForLine = fopen(argv[1], "r");
	if (!f)
	{
		perror(argv[1]);
		return 1;
	}
	codeFileName = argv[2];
	yyrestart(f);
	yyparse();
	
	return 0;
}
