#
# package data
#
PACKAGE_NAME=avdl
PACKAGE_VERSION=0.1.3

#
# system data
#
prefix=/usr/local/

#
# compiler data
#
#COMPILER_FLAGS=-Wall -Wpedantic -Wformat-security -fprofile-arcs -ftest-coverage --coverage#-Werror
COMPILER_FLAGS=-Wall -Wpedantic -Wformat-security #-Werror
COMPILER_DEFINES=\
	-DPKG_NAME=\"${PACKAGE_NAME}\"\
	-DPKG_VERSION=\"${PACKAGE_VERSION}\"\
	-DPKG_LOCATION=\"${prefix}\"
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
# engine files
#
ENGINE_FILES_SRC=$(wildcard engines/cengine/src/*.c)
ENGINE_FILES_HEADERS=$(wildcard engines/cengine/include/*.h)
ENGINE_FILES_ANDROID=$(ENGINE_FILES_HEADERS:engines/cengine/include/%.h=engine/%.h)\
	$(ENGINE_FILES_SRC:engines/cengine/src/%.c=engine/%.c)

#
# executable
#
EXECUTABLE=${DIRECTORY_EXE}/${PACKAGE_NAME}

#
# c engine data
#
CENGINE_PATH=engines/cengine

#
# test data
#
TESTS=$(wildcard tests/*)
TEST_NAMES=${TESTS:tests/%=%}
VALGRIND_ARGS=--error-exitcode=1 --tool=memcheck --leak-check=full \
	--track-origins=yes --show-leak-kinds=all --errors-for-leak-kinds=all

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
	${DESTDIR}${prefix}/share/avdl/cengine \
	${DESTDIR}${prefix}/share/avdl/android \
	${DESTDIR}${prefix}/share/avdl/android/app/src/main/cpp/engine \
	${DESTDIR}${prefix}/share/vim/vimfiles/syntax/ \
	${DESTDIR}${prefix}/share/vim/vimfiles/ftdetect/

${INSTALL_DIRS}:
	mkdir -p $@

#
# install the program to the current system
#
install: ${EXECUTABLE} ${INSTALL_DIRS}
	@# avdl files
	install ${EXECUTABLE} ${DESTDIR}${prefix}/bin/
	install manual/avdl.1 ${DESTDIR}${prefix}/share/man/man1/
	@# c engine
	${MAKE} -C ${CENGINE_PATH} prefix="${prefix}" DESTDIR="${DESTDIR}" install
	@# android engine
	cp -r engines/android/* ${DESTDIR}${prefix}/share/avdl/android
	cp -r engines/cengine/src/*.c engines/cengine/include/*.h\
		${DESTDIR}${prefix}/share/avdl/android/app/src/main/cpp/engine
	sed -i '/%AVDL_ENGINE_FILES%/ s#%AVDL_ENGINE_FILES%#${ENGINE_FILES_ANDROID}#'\
		${DESTDIR}${prefix}/share/avdl/android/app/src/main/cpp/CMakeLists.txt.in
	@# vim syntax files
	install vim/syntax/avdl.vim ${DESTDIR}${prefix}/share/vim/vimfiles/syntax/
	install vim/ftdetect/avdl.vim ${DESTDIR}${prefix}/share/vim/vimfiles/ftdetect/

#mkdir -p ${INSTALL_LOCATION}/share/info/
#install avdl.info.gz ${INSTALL_LOCATION}/share/info/

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

test:
	$(foreach TEST_NAME, ${TEST_NAMES}, \
		$(CC) ${COMPILER_FLAGS} ${COMPILER_DEFINES} ${COMPILER_INCLUDES} --coverage \
			tests/${TEST_NAME}/${TEST_NAME}.test.c $(shell cat tests/${TEST_NAME}/dependencies) \
			-o tests/${TEST_NAME}/result.out \
		&& ./tests/${TEST_NAME}/result.out \
	)

test-advance:
	$(foreach TEST_NAME, ${TEST_NAMES}, \
		$(CC) ${COMPILER_FLAGS} ${COMPILER_DEFINES} ${COMPILER_INCLUDES} --coverage \
			tests/${TEST_NAME}/${TEST_NAME}.test.c $(shell cat tests/${TEST_NAME}/dependencies) \
			-o tests/${TEST_NAME}/result.out \
		&& ./tests/${TEST_NAME}/result.out \
		&& gcov ./tests/${TEST_NAME}/*.gcno \
		&& mv *.gcov ./tests/${TEST_NAME} \
		&& valgrind ${VALGRIND_ARGS} ./tests/${TEST_NAME}/result.out \
	)

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

.PHONY: all tarball clean install test test-advance
