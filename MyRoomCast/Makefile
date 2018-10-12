CROSS = mipsel-linux-
#CROSS =

CC  = ${CROSS}gcc
CXX = ${CROSS}g++
LD  = ${CXX}


APPS = ${CUR_DIR}/src
BIN  = ${CUR_DIR}/bin
INC  = ${CUR_DIR}/inc
DEMO = ${CUR_DIR}/demo

UTIL_DIR = ./utils

UTIL_OBJ = ${UTIL_DIR}/src/TcpClient.o \
			${UTIL_DIR}/src/TcpServer.o \
			${UTIL_DIR}/src/Stream.o \
			${UTIL_DIR}/src/fifo_data.o \
			${UTIL_DIR}/src/common.o \
			${UTIL_DIR}/src/OssPlayer.o

MP2_OBJ	= ${UTIL_DIR}/src/mp2/mp2_codec.o \
	 	${UTIL_DIR}/src/mp2/mpegaudio.o \
		${UTIL_DIR}/src/mp2/mpegaudiodec.o \
		${UTIL_DIR}/src/mp2/mpegaudiotab.o 
				
FLAC_OBJ = ${UTIL_DIR}/src/flac/FlacEncode.o \
		${UTIL_DIR}/src/flac/FlacDecode.o

MP2_DIR =${UTIL_DIR}/inc/mp2

FLAC_DIR = ${UTIL_DIR}/inc/flac

ROOM_CAST = roomcast			
			
TARGET = ${ROOM_CAST}



MAIN_OBJ = ./main.o

OBJS = ${MAIN_OBJ} ${UTIL_OBJ} ${MP2_OBJ} ${FLAC_OBJ}


LIBS = -ldl -lrt -lpthread -lFLAC

LIBDIR	= -L${UTIL_DIR}/lib

INCLUDES = -I.  -I${UTIL_DIR}/inc -I${MP2_DIR} -I${FLAC_DIR}

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

${ROOM_CAST} : ${OBJS}
	${LD} ${LDFLAGS} ${LIBDIR} ${OBJS} ${LIBS}   -o ${ROOM_CAST} 
	
	
.phony:

clean:
	rm -rf    ${OBJS} ${TARGET} 



