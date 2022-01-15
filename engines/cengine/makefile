### variables ###

#
# project stats
#
PROJECT_NAME=avdl-cengine
PROJECT_VERSION=1.0.0

#
# directories
#
#
# SRC: source files directory
# BUILD: compiled build directory (everything compiled belongs here)
# OBJ: compiled objects directory
# DIRECTORIES: directories that should be created if they don't exist
#
DIRECTORY_SRC=src
DIRECTORY_BUILD=build
DIRECTORY_OBJ=${DIRECTORY_BUILD}/objects
DIRECTORIES=${DIRECTORY_BUILD} ${DIRECTORY_OBJ}

#
# source files
#
SRC=$(wildcard ${DIRECTORY_SRC}/*.c)
OBJ=${SRC:${DIRECTORY_SRC}/%.c=${DIRECTORY_OBJ}/%.o}
HEADERS=$(wildcard include/*.h)

#
# compiler flags
#
#COMPILER_FLAGS=-Wall -Werror -Wpedantic
COMPILER_FLAGS=-Wall -Wpedantic -Wformat-security

#
# output library
#
STATIC_OUT=${DIRECTORY_BUILD}/lib${PROJECT_NAME}.a

#
# install data
#
prefix=/usr/local

### recipes ###

#
# compile the whole library
#
all: ${DIRECTORIES} #${STATIC_OUT}

#
# create static library
#
${STATIC_OUT}: ${OBJ}
	ar rcs $@ $^

#
# make sure compiled build directories exist
#
${DIRECTORIES}:
	mkdir -p $@

#
# compile source files to object files
#
${DIRECTORY_OBJ}/%.o: ${DIRECTORY_SRC}/%.c ${HEADERS}
	${CC} ${COMPILER_FLAGS} -Iinclude -DDD_PLATFORM_NATIVE -o $@ -c $<

#
# create install directories
#
INSTALL_DIRS = ${DESTDIR}${prefix}/include ${DESTDIR}${prefix}/lib

${INSTALL_DIRS}:
	mkdir -p $@

#
# install library
#
install: ${DIRECTORIES} ${INSTALL_DIRS} #${STATIC_OUT}
	install -m644 ${HEADERS} ${DESTDIR}${prefix}/include
	install -m644 ${SRC} ${DESTDIR}${prefix}/share/avdl/cengine
	#install -m755 ${STATIC_OUT} ${DESTDIR}${prefix}/lib

#
# clean working directory
#
clean:
	rm -rf build

#
# phony recipes
#
.PHONY: clean
