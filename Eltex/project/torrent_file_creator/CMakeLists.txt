# CMakeList.txt: проект CMake для LinuxTest; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.8)

# Включение горячей перезагрузки для компиляторов MSVC, если поддерживается.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project (LinuxTest)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Compiler and flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wformat=2 -Wundef -Wfloat-equal -Wwrite-strings -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wjump-misses-init -fstack-protector-strong -D_FORTIFY_SOURCE=3 -pedantic -Wunused-variable -Wunused-function -Wunused-parameter -Wold-style-definition -Wmissing-format-attribute -g -Wno-deprecated-declarations ")

# Include ncurses library
# set(CURSES_NEED_NCURSES TRUE)
# set(CURSES_NEED_WIDE TRUE)
# find_package(Curses REQUIRED)
set(ENV{OPENSSL_ROOT_DIR} "/home/linuxbrew/.linuxbrew/opt/openssl@3")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# Добавьте источник в исполняемый файл этого проекта.
add_executable(LinuxTest "LinuxTest.c"        "color.h" "torrentfile.h")
target_link_libraries(LinuxTest PRIVATE Threads::Threads)
target_link_libraries(LinuxTest PRIVATE crypt)
target_link_libraries(LinuxTest PRIVATE OpenSSL::SSL OpenSSL::Crypto)
# target_link_libraries(LinuxTest PRIVATE ${CURSES_LIBRARIES})
# include_directories(${CURSES_INCLUDE_DIRS})
# include_directories("/usr/include/ncursesw/")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET LinuxTest PROPERTY CXX_STANDARD 20)
endif()

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
