EXECUTABLE=ddlang
ENGINE=dd_pixi_engine.js

${EXECUTABLE}: *.c ${ENGINE}
	flex lex.l
	bison -d yacc.y
	gcc *.c -o ${EXECUTABLE}

${ENGINE}: engines/pixi
	${MAKE} -C engines/pixi

install: ${EXECUTABLE}
	install ${EXECUTABLE} /usr/bin/
	mkdir -p /usr/share/${EXECUTABLE}
	install engines/pixi/${ENGINE} /usr/share/${EXECUTABLE}/${ENGINE}
	install engines/pixi/index.html /usr/share/${EXECUTABLE}/index.html
	install engines/pixi/pixi.min.js /usr/share/${EXECUTABLE}/pixi.min.js

clean:
	rm -f a.out lex.yy.c yacc.tab.c yacc.tab.h global
