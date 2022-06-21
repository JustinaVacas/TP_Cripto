cmake_minimum_required(VERSION 3.10)
project(TP_Cripto)

set(CMAKE_C_STANDARD 11)
set(SOURCES stegobmp.c decode.c encode.c encrypt.c)
set(HEADERS ./include/decode.h ./include/encode.h ./include/encrypt.h ./include/types.h)

add_executable(stegobmp ${SOURCES} ${HEADERS})
target_include_directories(stegobmp PRIVATE ./include)
target_link_libraries(stegobmp crypto)