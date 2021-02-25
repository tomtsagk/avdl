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
COMPILER_FLAGS=-Wall -Werror -Wpedantic

#
# output library
#
STATIC_OUT=${DIRECTORY_BUILD}/lib${PROJECT_NAME}.a

#
# install data
#
PREFIX=

### recipes ###

#
# compile the whole library
#
all: ${DIRECTORIES} ${STATIC_OUT}

#
# create static library
#
${STATIC_OUT}: ${OBJ}
	ar rcs $@ $^

#
# make sure compiled build directories exist
#
${DIRECTORIES}:
	mkdir $@

#
# compile source files to object files
#
${DIRECTORY_OBJ}/%.o: ${DIRECTORY_SRC}/%.c ${HEADERS}
	${CC} ${COMPILER_FLAGS} -Iinclude -DDD_PLATFORM_NATIVE -o $@ -c $<

#
# install library
#
install: ${STATIC_OUT}
	mkdir -p ${PREFIX}/usr/include
	install -m644 ${HEADERS} ${PREFIX}/usr/include
	mkdir -p ${PREFIX}/usr/lib
	install -m755 ${STATIC_OUT} ${PREFIX}/usr/lib

#
# clean working directory
#
clean:
	rm -r ${DIRECTORY_BUILD}

#
# phony recipes
#
.PHONY: clean
