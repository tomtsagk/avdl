#
# package data
#
PACKAGE_NAME=avdl
PACKAGE_VERSION=0.4.3

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
DIRECTORY_TESTS=${DIRECTORY_BUILD}/tests
DIRECTORY_TESTS_DEPS=${DIRECTORY_TESTS}/objects
DIRECTORY_TESTS_CENG=${DIRECTORY_TESTS}/cengine
DIRECTORY_TESTS_CENG_DEPS=${DIRECTORY_TESTS_CENG}/objects
DIRECTORY_COVERAGE=coverage/
DIRECTORY_ALL=${DIRECTORY_BUILD} ${DIRECTORY_EXE} ${DIRECTORY_OBJ} ${DIRECTORY_TESTS} \
	${DIRECTORY_TESTS_DEPS} ${DIRECTORY_COVERAGE} ${DIRECTORY_TESTS_CENG} ${DIRECTORY_TESTS_CENG_DEPS}

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
# unit test data
#
TESTS=$(wildcard tests/*.test.c)
TESTS_OBJ=${TESTS:tests/%.c=build/tests/%.o}
TESTS_OBJ_DEPS=${SRC:src/%.c=build/tests/objects/%.o}
TESTS_OUT=${TESTS_OBJ:%.o=%.out}

TESTS_CENG=$(wildcard engines/cengine/tests/*.test.c)
TESTS_CENG_OBJ=${TESTS_CENG:engines/cengine/tests/%.c=build/tests/cengine/%.o}
TESTS_CENG_OBJ_DEPS=${ENGINE_FILES_SRC:engines/cengine/src/%.c=build/tests/cengine/objects/%.o}
TESTS_CENG_OUT=${TESTS_CENG_OBJ:%.o=%.out}

TESTS_NAMES=${TESTS:tests/%.test.c=%}

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
	rm -rf build

#
# simple tests, they are just compiled and run
#
test: ${TESTS_NAMES}
${TESTS_NAMES}:
	@echo "Running tests on $@"
	@$(CC) ${COMPILER_FLAGS} ${COMPILER_DEFINES} ${COMPILER_INCLUDES} tests/$@.test.c src/*.c -DAVDL_UNIT_TEST -o $@.test.out -Wno-unused-variable -Wno-parentheses
	@./$@.test.out
	@rm $@.test.out

#
# advanced tests, depend on `gcc`, `gcov`, `lcov` and `valgrind`
# they check code coverage and memory leaks, on top of simple tests
#
test-advance: ${DIRECTORY_COVERAGE}/lcov.info

# generate the final lcov.info file
${DIRECTORY_COVERAGE}/lcov.info: ${TESTS_OUT} ${TESTS_CENG_OUT} ${DIRECTORY_COVERAGE}
	gcov ${DIRECTORY_TESTS_CENG_DEPS}/*.o
	gcov ${DIRECTORY_TESTS_DEPS}/*.o
	geninfo . -q -o ${DIRECTORY_TESTS}/result.info
	lcov -a ${DIRECTORY_TESTS}/result.info -o ${DIRECTORY_COVERAGE}/lcov.info

# test executables
${DIRECTORY_TESTS_CENG}/%.out: ${DIRECTORY_TESTS_CENG}/%.o ${TESTS_CENG_OBJ_DEPS}
	@echo "Running cengine tests on $@"
	@${CC} ${COMPILER_FLAGS} ${COMPILER_DEFINES} ${COMPILER_INCLUDES} -Iengines/cengine/include --coverage -o $@ $^ -DAVDL_UNIT_TEST -Wno-unused-variable -Wno-parentheses -lGLU -lm -w -lSDL2 -lSDL2_mixer -lpthread -lGL -lGLEW -DDD_PLATFORM_NATIVE
	@./$@
	@# These fail on some systems, so disabled for now
	@#valgrind ${VALGRIND_ARGS} ./$@

${DIRECTORY_TESTS}/%.out: ${DIRECTORY_TESTS}/%.o ${TESTS_OBJ_DEPS}
	@echo "Running tests on $@"
	@${CC} ${COMPILER_FLAGS} ${COMPILER_DEFINES} ${COMPILER_INCLUDES} --coverage -o $@ $^ -DAVDL_UNIT_TEST
	@./$@
	@valgrind ${VALGRIND_ARGS} ./$@

# test objects
${DIRECTORY_TESTS_CENG}/%.o: engines/cengine/tests/%.c ${DIRECTORY_TESTS_CENG}
	@${CC} ${COMPILER_FLAGS} ${COMPILER_DEFINES} ${COMPILER_INCLUDES} -Iengines/cengine/include -c --coverage -o $@ $< -DAVDL_UNIT_TEST -Wno-unused-variable -Wno-parentheses -lGLU -lm -w -lSDL2 -lSDL2_mixer -lpthread -lGL -lGLEW -DDD_PLATFORM_NATIVE

${DIRECTORY_TESTS}/%.o: tests/%.c ${DIRECTORY_TESTS}
	@${CC} ${COMPILER_FLAGS} ${COMPILER_DEFINES} ${COMPILER_INCLUDES} --coverage -c -o $@ $< -DAVDL_UNIT_TEST

# test dependency objects
${DIRECTORY_TESTS_CENG_DEPS}/%.o: engines/cengine/src/%.c ${DIRECTORY_TESTS_CENG_DEPS}
	@${CC} ${COMPILER_FLAGS} ${COMPILER_DEFINES} ${COMPILER_INCLUDES} -Iengines/cengine/include -c --coverage -o $@ $< -DAVDL_UNIT_TEST -Wno-unused-variable -Wno-parentheses -lGLU -lm -w -lSDL2 -lSDL2_mixer -lpthread -lGL -lGLEW -DDD_PLATFORM_NATIVE

${DIRECTORY_TESTS_DEPS}/%.o: src/%.c ${DIRECTORY_TESTS_DEPS}
	@${CC} ${COMPILER_FLAGS} ${COMPILER_DEFINES} ${COMPILER_INCLUDES} --coverage -c -o $@ $< -DAVDL_UNIT_TEST

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

.PHONY: all tarball clean install test test-advance ${TESTS_NAMES}
