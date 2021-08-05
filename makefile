#
# package data
#
PACKAGE_NAME=avdl
PACKAGE_VERSION=0.0.5

#
# compiler data
#
COMPILER_FLAGS=-Wall -Wpedantic -Wformat-security #-Werror
COMPILER_DEFINES=\
	-DPACKAGE_NAME=\"${PACKAGE_NAME}\"\
	-DPACKAGE_VERSION=\"${PACKAGE_VERSION}\"
COMPILER_INCLUDES=-Iinclude -I${DIRECTORY_OBJ}

#
# directories
#
DIRECTORY_BUILD=build
DIRECTORY_EXE=${DIRECTORY_BUILD}/bin
DIRECTORY_OBJ=${DIRECTORY_BUILD}/objects
DIRECTORY_ALL=${DIRECTORY_BUILD} ${DIRECTORY_EXE} ${DIRECTORY_OBJ}

#
# source files
#
SRC=$(wildcard src/*.c)
OBJ=${SRC:src/%.c=${DIRECTORY_OBJ}/%.o}
HEADERS=$(widcard include/*.h)

#
# executable
#
EXECUTABLE=${DIRECTORY_EXE}/${PACKAGE_NAME}

#
# engine data
#
ENGINE_PATH=engines/cengine
ENGINE_OUT=${ENGINE_PATH}/build/libavdl-cengine.a

#
# system data
#
prefix=/usr/local

# android
#DIRECTORIES=engines/android/app/src/main/cpp/engine/
#CENGINE_FILES_HEADER=$(wildcard engines/cengine/include/*.h)
#CENGINE_FILES_SRC=$(wildcard engines/cengine/src/*.c)
#CENGINE_FILES_ANDROID_HEADER=$(CENGINE_FILES_HEADER:engines/cengine/include/%.h=engines/android/app/src/main/cpp/engine/%.h)
#CENGINE_FILES_ANDROID_SRC=$(CENGINE_FILES_SRC:engines/cengine/src/%.c=engines/android/app/src/main/cpp/engine/%.c)
#CENGINE_FILES_LOCAL=$(CENGINE_FILES_SRC:engines/cengine/src/%.c=engine\/%.c)

all: ${EXECUTABLE} ${ENGINE_OUT}

# build the executable, depends on source, lex, yacc and all engines
${EXECUTABLE}: ${DIRECTORY_ALL} ${DIRECTORIES} ${OBJ} #${CENGINE_FILES_ANDROID_HEADER} ${CENGINE_FILES_ANDROID_SRC} engines/android/app/src/main/cpp/CMakeLists.txt
	$(CC) ${COMPILER_FLAGS} ${COMPILER_DEFINES} ${COMPILER_INCLUDES} ${OBJ} -o $@

# how to build the c engine
engine: ${ENGINE_OUT}

${ENGINE_OUT}: ${CENGINE_FILES_SRC}
	${MAKE} -C ${ENGINE_PATH}

install: ${EXECUTABLE} ${ENGINE_OUT}
	${MAKE} -C ${ENGINE_PATH} prefix="${prefix}" destdir="${DESTDIR}" install
	mkdir -p ${DESTDIR}${prefix}/bin
	install ${EXECUTABLE} ${DESTDIR}${prefix}/bin/
	mkdir -p ${DESTDIR}${prefix}/share/man/man1/
	install manual/avdl.1 ${DESTDIR}${prefix}/share/man/man1/
	mkdir -p ${DESTDIR}${prefix}/share/avdl/android
	cp -r engines/android/* ${DESTDIR}${prefix}/share/avdl/android

#mkdir -p ${INSTALL_LOCATION}/share/info/
#install avdl.info.gz ${INSTALL_LOCATION}/share/info/
# vim files
#mkdir -p ${INSTALL_LOCATION}/share/vim/vimfiles/syntax/
#install vim/syntax/avdl.vim ${INSTALL_LOCATION}/share/vim/vimfiles/syntax/
#mkdir -p ${INSTALL_LOCATION}/share/vim/vimfiles/ftdetect/
#install vim/ftdetect/avdl.vim ${INSTALL_LOCATION}/share/vim/vimfiles/ftdetect/

tarball: ${PACKAGE_NAME}.tar

${PACKAGE_NAME}.tar:
	tar cf $@ src makefile engines include

clean:
	${MAKE} -C ${ENGINE_PATH} clean
	rm -f ${EXECUTABLE} ${OBJ}

# android files
#engines/android/app/src/main/cpp/engine/%.h: engines/cengine/include/%.h
#	cp $< $@

#engines/android/app/src/main/cpp/engine/%.c: engines/cengine/src/%.c
#	cp $< $@

#engines/android/app/src/main/cpp/CMakeLists.txt: engines/android/app/src/main/cpp/CMakeLists.txt.in
#	sed 's/%AVDL_ENGINE_FILES%/${CENGINE_FILES_LOCAL}/' $< > $@

${DIRECTORIES} ${DIRECTORY_ALL}:
	mkdir -p $@

${DIRECTORY_OBJ}/%.o: src/%.c ${HEADERS}
	$(CC) ${COMPILER_FLAGS} ${COMPILER_DEFINES} ${COMPILER_INCLUDES} -c $< -o $@


.PHONY: clean destclean install engine
