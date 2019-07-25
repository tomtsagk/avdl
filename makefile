EXECUTABLE=ddlang

${EXECUTABLE}: *.c
	flex lex.l
	bison -d yacc.y
	gcc *.c -o ${EXECUTABLE}

install: ${EXECUTABLE}
	install ${EXECUTABLE} /usr/bin/

clean:
	rm -f a.out lex.yy.c yacc.tab.c yacc.tab.h global
