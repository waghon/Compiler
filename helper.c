#include "helper.h"

char currentLine[MAXLINESIZE];

/* 打印错误信息 */
void printError(int type,int line, int column, char* msg){
	fseek(fileForLine,0,SEEK_SET);//定位到文件开始
	int i;
	/* 定位到当前行 */
	for(i=0;i<line;i++)
		fgets(currentLine,MAXLINESIZE,fileForLine);	
	if(type==-1)
		fprintf(stderr, "Error type B at Line %d: %s\n", line,msg);
	else
		fprintf(stderr, "Error type %d at Line %d: %s\n", 
				type,line,msg);

	/* 输出当前行和错误点 */
	fprintf(stderr, " %s", currentLine);
	fprintf(stderr, " ");
	for(i=0; i<column-1; i++){
		if(currentLine[i]=='	')
			fprintf(stderr, "	");
		else 
			fprintf(stderr, " ");
	}
	fprintf(stderr, "^\n");
}

/* 打印错误信息 */
void printPreDec(int line, int column, char* name){
	fseek(fileForLine,0,SEEK_SET);//定位到文件开始
	int i;
	/* 定位到当前行 */
	for(i=0;i<line;i++)
		fgets(currentLine,MAXLINESIZE,fileForLine);	
	
	fprintf(stderr, "previous definition of '%s' was here at Line %d: \n", name, line);

	/* 输出当前行和错误点 */
	fprintf(stderr, " %s", currentLine);
	fprintf(stderr, " ");
	for(i=0; i<column-1; i++){
		if(currentLine[i]=='	')
			fprintf(stderr, "	");
		else 
			fprintf(stderr, " ");
	}
	fprintf(stderr, "^\n");
}

/* 打印debug信息 */
void debug(char* s){
#ifdef DEBUG_
	printf("%s\n",s);
#endif
}

/* 特殊的字符串拼接 */
char* myStrcat(char* msg,char* name){
	strcat(msg," '");
	strcat(msg,name);
	strcat(msg,"'");
	return msg;
}

unsigned int hash_pjw(char* name)
{
	unsigned int val = 0, i;
	for (; *name; ++name)
	{
		val = (val << 2) + *name;
		if (i = val & ~0x3fff) val = (val ^ (i >> 12)) & 0x3fff;
	}
	return val;
}

