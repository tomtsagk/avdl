EXECUTABLE=ddlang
ENGINE=dd_pixi_engine.js

SRC=$(wildcard src/*.c)

YACC_SRC=yacc.tab.c
YACC_HEADER=yacc.tab.h
LEX_SRC=lex.yy.c

# build the executable, depends on source, lex, yacc and all engines
${EXECUTABLE}: ${SRC} ${YACC_SRC} ${YACC_HEADER} ${LEX_SRC} ${ENGINE}
	gcc -Iinclude -I. ${SRC} ${YACC_SRC} ${LEX_SRC} -o ${EXECUTABLE}

# how to build lex
${LEX_SRC}: src/lex.l
	flex src/lex.l

# how to build yacc
${YACC_SRC} ${YACC_HEADER}: src/yacc.y
	bison -d src/yacc.y

# how to build the pixi engine
${ENGINE}: engines/pixi
	${MAKE} -C engines/pixi

install: ${EXECUTABLE}
	install ${EXECUTABLE} /usr/bin/
	mkdir -p /usr/share/${EXECUTABLE}
	install engines/pixi/${ENGINE} /usr/share/${EXECUTABLE}/${ENGINE}
	install engines/pixi/index.html /usr/share/${EXECUTABLE}/index.html

clean:
	rm -f ${EXECUTABLE} ${LEX_SRC} ${YACC_SRC} ${YACC_HEADER}
