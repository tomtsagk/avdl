#
# package data
#
PACKAGE_NAME=avdl
PACKAGE_VERSION=0.1.3

#
# compiler data
#
COMPILER_FLAGS=-Wall -Wpedantic -Wformat-security#-Werror
COMPILER_DEFINES=\
	-DPKG_NAME=\"${PACKAGE_NAME}\"\
	-DPKG_VERSION=\"${PACKAGE_VERSION}\"
COMPILER_INCLUDES=-Iinclude

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
# c engine data
#
CENGINE_PATH=engines/cengine

#
# system data
#
prefix=/usr/local

#
# compile the package, together with all engines
#
all: ${EXECUTABLE}
	${MAKE} -C ${CENGINE_PATH} all

#
# build the executable, depends on source files
#
${EXECUTABLE}: ${DIRECTORY_ALL} ${OBJ}
	$(CC) ${COMPILER_FLAGS} ${COMPILER_DEFINES} ${COMPILER_INCLUDES} ${OBJ} -o $@

#
# create install directories
#
INSTALL_DIRS = ${DESTDIR}${prefix}/bin ${DESTDIR}${prefix}/share/man/man1/ \
	${DESTDIR}${prefix}/share/avdl/android

${INSTALL_DIRS}:
	mkdir -p $@

#
# install the program to the current system
#
install: ${EXECUTABLE} ${INSTALL_DIRS}
	@# avdl files
	install ${EXECUTABLE} ${DESTDIR}${prefix}/bin/
	install manual/avdl.1 ${DESTDIR}${prefix}/share/man/man1/
	cp -r engines/android/* ${DESTDIR}${prefix}/share/avdl/android
	@# c engine
	${MAKE} -C ${CENGINE_PATH} prefix="${prefix}" destdir="${DESTDIR}" install

#mkdir -p ${INSTALL_LOCATION}/share/info/
#install avdl.info.gz ${INSTALL_LOCATION}/share/info/
# vim files
#mkdir -p ${INSTALL_LOCATION}/share/vim/vimfiles/syntax/
#install vim/syntax/avdl.vim ${INSTALL_LOCATION}/share/vim/vimfiles/syntax/
#mkdir -p ${INSTALL_LOCATION}/share/vim/vimfiles/ftdetect/
#install vim/ftdetect/avdl.vim ${INSTALL_LOCATION}/share/vim/vimfiles/ftdetect/

#
# create a tarball of all source files needed to compile this project
#
tarball: ${PACKAGE_NAME}-${PACKAGE_VERSION}.tar

${PACKAGE_NAME}-${PACKAGE_VERSION}.tar:
	tar cf $@ src makefile engines include

#
# clean all automatically generated files
#
clean:
	${MAKE} -C ${CENGINE_PATH} clean
	rm -f ${EXECUTABLE} ${OBJ}

#
# create needed directories
#
${DIRECTORIES} ${DIRECTORY_ALL}:
	mkdir -p $@

#
# compile .c source files
#
${DIRECTORY_OBJ}/%.o: src/%.c ${HEADERS}
	$(CC) ${COMPILER_FLAGS} ${COMPILER_DEFINES} ${COMPILER_INCLUDES} -c $< -o $@

.PHONY: all tarball clean install
