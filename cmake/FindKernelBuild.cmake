set(KERNEL_BUILD_DIR "" CACHE STRING "Path to the kernel build directory")
if ("${KERNEL_BUILD_DIR}" STREQUAL "")
    execute_process(COMMAND uname -r OUTPUT_VARIABLE KERNEL_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(KERNEL_BUILD_DIR "/lib/modules/${KERNEL_VERSION}/build")
endif ()

find_file(KERNEL_MAKEFILE NAMES Makefile PATHS ${KERNEL_BUILD_DIR} NO_DEFAULT_PATH)

if (NOT KERNEL_MAKEFILE)
    message(FATAL_ERROR "There is no Makefile in the kernel build directory!")
endif ()