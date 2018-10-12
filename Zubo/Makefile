CROSS = mipsel-linux-
#CROSS =

CC  = ${CROSS}gcc
CXX = ${CROSS}g++
LD  = ${CXX}


APPS = ${CUR_DIR}/src
BIN  = ${CUR_DIR}/bin
INC  = ${CUR_DIR}/inc
DEMO = ${CUR_DIR}/demo

UTIL_DIR = .

UTIL_OBJ = ${UTIL_DIR}/src/net_common.o \
		${UTIL_DIR}/src/bdcp.o \
		${UTIL_DIR}/src/dotini_parser.o \
		${UTIL_DIR}/src/gpio_manager.o


ZUBO = zubo			
			
TARGET = ${ZUBO}



MAIN_OBJ = ./main.o

OBJS = ${MAIN_OBJ} ${UTIL_OBJ}


LIBS = -lpthread

LIBDIR	= -L${UTIL_DIR}/lib

INCLUDES = -I.  -I${UTIL_DIR}/inc 

DEBUG_FLAGS	=	-g3  

CFLAGS   = ${INCLUDES} ${DEBUG_FLAGS} -Wall -fPIC
CXXFLAGS = ${INCLUDES} ${DEBUG_FLAGS} -Wall -fPIC 
LDFLAGS  = ${DEBUG_FLAGS} -Wall -fPIC 

.SUFFIXES : .c .cc .cpp

%.o:%.c	
	${CC} ${CFLAGS} -c -o $@ $<
%.o:%.cpp
	${CXX} ${CXXFLAGS}   -c -o $@ $<



all:  $(TARGET)

${ZUBO} : ${OBJS}
	${LD} ${LDFLAGS} ${LIBDIR} ${OBJS} ${LIBS}   -o ${ZUBO} 
	
	
.phony:

clean:
	rm -rf    ${OBJS} ${TARGET} 



