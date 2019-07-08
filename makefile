all:
	flex lex.l
	bison -d yacc.y
	gcc *.c

clean:
	rm -f a.out lex.yy.c yacc.tab.c yacc.tab.h global
