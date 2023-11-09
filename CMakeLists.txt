cmake_minimum_required(VERSION 3.3)

project(qrb5165-rangefinger-server)


# James' standard list of cmake flags
set(CMAKE_C_FLAGS "-std=gnu99 -Wall -Wextra -Wuninitialized \
	-Wunused-variable -Wdouble-promotion -Wmissing-prototypes \
	-Wmissing-declarations -Werror=undef -Wno-unused-function ${CMAKE_C_FLAGS}")


# tell the linker not to worry about missing symbols in libraries
set(CMAKE_C_FLAGS   "-Wl,--unresolved-symbols=ignore-in-shared-libs ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-Wl,--unresolved-symbols=ignore-in-shared-libs ${CMAKE_CXX_FLAGS}")


# for VOXL, install 64-bit libraries to lib64, 32-bit libs go in /usr/lib
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^aarch64")
	set(LIB_INSTALL_DIR /usr/lib64)
else()
	set(LIB_INSTALL_DIR /usr/lib)
endif()

# include each subdirectory, may have others in example/ or lib/ etc
add_subdirectory (src)
add_subdirectory (tools)
