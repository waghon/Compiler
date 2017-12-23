objects=main.c syntax.tab.c helper.c semanteme.c translate.c interCode.c
scanner: $(objects)
	cc $(objects) -lfl -ly -g -o parser
syntax.tab.c: syntax.y lex.yy.c
	bison -d syntax.y
lex.yy.c: lexical.l
	flex lexical.l
clean:
	-rm lex.yy.c syntax.tab.c syntax.tab.h parser
	-rm code*.ir
run: scanner
	sh run.sh
