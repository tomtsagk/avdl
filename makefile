#
# package data
#
PACKAGE_NAME=avdl
PACKAGE_VERSION=0.2.2

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
TESTS=$(wildcard tests/*.test.c)
TEST_NAMES=${TESTS:tests/%.test.c=%}
TEST_NAMES_ADV=${TESTS:tests/%.test.c=%-adv}
CENG_TESTS=$(wildcard engines/cengine/tests/*.test.c)
CENG_TEST_NAMES=${CENG_TESTS:engines/cengine/tests/%.test.c=%}
CENG_TEST_NAMES_ADV=${CENG_TESTS:engines/cengine/tests/%.test.c=%-adv}
TEST_DEPENDENCIES=src/*.c
VALGRIND_ARGS=--error-exitcode=1 --tool=memcheck --leak-check=full \
	--track-origins=yes --show-leak-kinds=all --errors-for-leak-kinds=all -q

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

#
# simple tests, they are just compiled and run
#
test: ${TEST_NAMES}
${TEST_NAMES}:
	@echo "Running tests on $@"
	@$(CC) ${COMPILER_FLAGS} ${COMPILER_DEFINES} ${COMPILER_INCLUDES} tests/$@.test.c ${TEST_DEPENDENCIES} -DAVDL_UNIT_TEST -o $@.test.out -Wno-unused-variable -Wno-parentheses
	@./$@.test.out
	@rm $@.test.out

#
# advanced tests, depend on `gcc`, `gcov`, `lcov` and `valgrind`
# they check code coverage and memory leaks, on top of simple tests
#
test-advance: ${TEST_NAMES_ADV} ${CENG_TEST_NAMES}
	mkdir -p coverage
	lcov $(foreach TEST_NAME, ${TEST_NAMES}, \
		-a ./tests/${TEST_NAME}-lcov.info \
	) $(foreach TEST_NAME, ${CENG_TEST_NAMES}, \
		-a ./engines/cengine/tests/${TEST_NAME}-lcov.info \
	) -o ./coverage/lcov.info -q

${TEST_NAMES_ADV}:
	@echo "Running advanced tests on $@"
	$(CC) ${COMPILER_FLAGS} ${COMPILER_DEFINES} ${COMPILER_INCLUDES} --coverage tests/${@:%-adv=%}.test.c ${TEST_DEPENDENCIES} -DAVDL_UNIT_TEST -o $@.test.out
	./$@.test.out
	gcov ./*.gcno
	geninfo . -b . -o ./tests/${@:%-adv=%}-lcov.info -q
	valgrind ${VALGRIND_ARGS} ./$@.test.out
	rm -f -- ./$@.test.out ./*.gc*

${CENG_TEST_NAMES}:
	$(CC) ${COMPILER_FLAGS} ${COMPILER_DEFINES} ${COMPILER_INCLUDES} -Iengines/cengine/include --coverage engines/cengine/tests/$@.test.c engines/cengine/src/*.c -DAVDL_UNIT_TEST -o $@.test.out -Wno-unused-variable -Wno-parentheses -lGLU -lm -w -lSDL2 -lSDL2_mixer -lpthread -lGL -lGLEW -DDD_PLATFORM_NATIVE
	./$@.test.out
	gcov ./*.gcno
	geninfo . -b . -o ./engines/cengine/tests/${@:%-adv=%}-lcov.info -q
	@#valgrind ${VALGRIND_ARGS} ./$@.test.out
	rm -f -- ./$@.test.out ./*.gc*

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

.PHONY: all tarball clean install test test-advance ${TEST_NAMES} ${TEST_NAMES_ADV}
