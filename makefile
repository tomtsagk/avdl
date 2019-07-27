EXECUTABLE=ddlang
ENGINE=dd_pixi_engine.js

${EXECUTABLE}: *.c ${ENGINE}
	flex lex.l
	bison -d yacc.y
	gcc *.c -o ${EXECUTABLE}

${ENGINE}: engines/pixi
	${MAKE} -C engines/pixi
	cp engines/pixi/$@ $@
	cp engines/pixi/index.html index.html
	cp engines/pixi/pixi.min.js pixi.min.js

install: ${EXECUTABLE}
	install ${EXECUTABLE} /usr/bin/
	mkdir -p /usr/share/${EXECUTABLE}
	install ${ENGINE} /usr/share/${EXECUTABLE}/${ENGINE}
	install index.html /usr/share/${EXECUTABLE}/index.html
	install pixi.min.js /usr/share/${EXECUTABLE}/pixi.min.js

clean:
	rm -f a.out lex.yy.c yacc.tab.c yacc.tab.h global
