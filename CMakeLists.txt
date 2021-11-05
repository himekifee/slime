cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
# Ref:
# https://musteresel.github.io/posts/2020/02/cmake-template-linux-kernel-module.html
# https://github.com/tud-zih-energy/x86_adapt/blob/master/CMakeLists.txt
# https://gitlab.com/phip1611/cmake-kernel-module/-/blob/ubuntu-clion-ide/CMakeLists.txt
# https://github.com/enginning/cmake-kernel-module/blob/main/CMakeLists.txt

project(slime VERSION 0.0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(MODULE_NAME slime)
configure_file(src/kernel/config.h.in src/kernel/config.h @ONLY)

set(CMAKE_C_FLAGS "-Wall -std=gnu99 -Wno-declaration-after-statement -fno-stack-protector")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -Os")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -DSLIME_DEBUG -ggdb -O0 -Wstrict-prototypes -Wundef -Wno-long-long -Wcomment -finline-functions -fno-strict-aliasing")
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS_DEBUG})
endif (${CMAKE_BUILD_TYPE} STREQUAL "Debug")

# find MODULE_LICENSE("GPL"), MODULE_AUTHOR() etc.
add_definitions(-D__KERNEL__ -DMODULE)

# Find kernel headers
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(KernelBuild REQUIRED)


set(SUBMODULES src/kernel/modules/hide_dir.c src/kernel/include/hide_dir.h)
set(LIST_SRC thirdparty/list/src/list.c thirdparty/list/src/list.h thirdparty/list/src/list_node.c thirdparty/list/src/list_iterator.c)


include_directories(thirdparty/khook/khook thirdparty/list/src src/kernel/include ${KERNEL_BUILD_DIR}/include ${KERNEL_BUILD_DIR}/arch/x86/include ${KERNEL_BUILD_DIR}/arch/x86/include/generated)
include_directories(${CMAKE_BINARY_DIR}/src/kernel)
get_property(INCLUDE_DIRS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
add_library(${MODULE_NAME}_lib STATIC src/kernel/slime.lds src/kernel/loader.c src/kernel/include/loader.h thirdparty/khook/khook/engine.c ${SUBMODULES} ${LIST_SRC})
get_target_property(MODULE_SOURCES slime_lib SOURCES)

# Add include directories to Kbuild EXTRA_CFLAGS
foreach (include_dir ${INCLUDE_DIRS})
    list(APPEND KBUILD_INCLUDE "-I${include_dir} ")
endforeach ()
string(REPLACE ";" " " KBUILD_INCLUDE_STRING "${KBUILD_INCLUDE}")

string(REPLACE ";" " " SUBMODULES_STRING "${SUBMODULES}")
string(REPLACE "src/kernel/" " " SUBMODULES_STRING "${SUBMODULES_STRING}")

string(REPLACE ";" " " LIST_SRC_STRING "${LIST_SRC}")
string(REPLACE "thirdparty/" "../../thirdparty/" LIST_SRC_STRING "${LIST_SRC_STRING}")

string(REPLACE ";" " " MODULE_SOURCES_STRING "${MODULE_SOURCES}")
configure_file(src/kernel/Kbuild.in src/kernel/Kbuild @ONLY)

foreach (src ${MODULE_SOURCES})
    configure_file(${src} ${src} COPYONLY)
endforeach ()

set(MODULE_MAKE ${CMAKE_MAKE_PROGRAM} -C ${KERNEL_BUILD_DIR} M=${CMAKE_BINARY_DIR}/src/kernel)

add_custom_command(OUTPUT ${MODULE_NAME}.ko
        COMMAND ${MODULE_MAKE} modules
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS ${slime_sources} ${CMAKE_BINARY_DIR}/src/kernel/Kbuild
        VERBATIM)

add_custom_target(module DEPENDS copy_khook ${MODULE_NAME}.ko)
add_custom_target(module_clean COMMAND ${MODULE_MAKE} clean)
add_custom_target(copy_khook ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/thirdparty/khook ${CMAKE_BINARY_DIR}/thirdparty/khook
        DEPENDS ${MY_TARGET})