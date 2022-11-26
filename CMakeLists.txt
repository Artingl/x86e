cmake_minimum_required(VERSION 3.23)
project(x86e)

set(CMAKE_CXX_STANDARD 20)
set(X86E_VERSION "b0.1-a")

add_executable(${PROJECT_NAME} src/main.cpp include/cpu/i386.h include/cpu/cpu.h src/cpu/cpu.cpp src/io/Logger.cpp include/io/Logger.h src/cpu/i386.cpp include/memory/memory.h src/memory/memory.cpp include/io/fs.h src/io/fs.cpp include/cpu/im/x86im.h include/cpu/im/i386im.h src/cpu/im/x86im.cpp src/cpu/im/i386im.cpp include/utils/utils.h src/utils/utils.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_compile_definitions(${PROJECT_NAME} PRIVATE VERSION=\"${X86E_VERSION}\")
