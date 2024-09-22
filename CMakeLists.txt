cmake_minimum_required(VERSION 3.26)
project(QMBBS)

set(CMAKE_CXX_STANDARD 17)

add_executable(QMBBS main.cpp
libs/libgetfile.h
)

add_definitions(-DUNICODE -D_UNICODE)

add_executable(GUI GUI.cpp)

target_link_libraries(GUI PRIVATE Ws2_32)

add_executable(MessageViewer MessageViewer.cpp)

target_link_libraries(MessageViewer PRIVATE Ws2_32)

add_executable(Socket.Server Server.cpp)

target_link_libraries(Socket.Server PRIVATE Ws2_32)


add_executable(Socket.Client Client.cpp)

target_link_libraries(Socket.Client PRIVATE Ws2_32)


add_executable(MessageCacher MessageCacher.cpp)
target_link_libraries(MessageCacher ws2_32)