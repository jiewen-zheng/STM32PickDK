cmake_minimum_required(VERSION 3.25)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)

##################################################
#                 Config project                 #
##################################################
set(PROJECT_NAME STM32PicoDK)
set(CMAKE_SYSTEM_PROCESSOR Cortex-M3)
set(OPTIMIZATION_LEVEL -O0)
set(FPU_ENABLE FALSE)

##################################################
#                 Compilers Path                 #
##################################################

set(KEIL_PATH D:/software/Keil_v5)
set(ARMCLANG_PATH ${KEIL_PATH}/ARM/ARMCLANG/bin)
set(SECTIONS_SCRIPT_PATH ${CMAKE_HOME_DIRECTORY}/MDK-ARM/${PROJECT_NAME}/${PROJECT_NAME}.sct)

##################################################
#           Specify compilers and tools          #
##################################################
set(CMAKE_C_COMPILER ${ARMCLANG_PATH}/armclang.exe)
set(CMAKE_CXX_COMPILER ${ARMCLANG_PATH}/armclang.exe)
set(CMAKE_ASM_COMPILER  ${ARMCLANG_PATH}/armclang.exe)
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_CXX_COMPILER_WORKS TRUE)
set(CMAKE_ASM_COMPILER_WORKS TRUE)

set(CMAKE_C_LINK_EXECUTABLE ${ARMCLANG_PATH}/armlink.exe)
set(CMAKE_CXX_LINK_EXECUTABLE ${ARMCLANG_PATH}/armlink.exe)
set(CMAKE_ASM_LINK_EXECUTABLE ${ARMCLANG_PATH}/armlink.exe)

##################################################
#                Config Compilers                #
##################################################

set(C_TARGET_FLAG --target=arm-arm-none-eabi)
set(ASM_TARGET_FLAG --target=arm-arm-none-eabi)
set(LINKER_TARGET_FLAG --cpu=${CMAKE_SYSTEM_PROCESSOR})

set(COMPILE_RULE_FLAG "-mcpu=${CMAKE_SYSTEM_PROCESSOR}")

# C compilers options
set(CMAKE_C_FLAGS_INIT "${C_TARGET_FLAG} ${COMPILE_RULE_FLAG} -c \
            -fno-rtti -flto -funsigned-char -fshort-enums -fshort-wchar \
            -gdwarf-4 ${OPTIMIZATION_LEVEL} -ffunction-sections -w\
            ")
# C++ compilers options
set(CMAKE_CXX_FLAGS_INIT "${CMAKE_C_FLAGS_INIT}")

# ASM compilers options
set(CMAKE_ASM_FLAGS_INIT "${ASM_TARGET_FLAG} ${COMPILE_RULE_FLAG} \
            -masm=auto -c -gdwarf-4 \
            ")

##################################################
#                Config Linker                   #
##################################################

set(CMAKE_EXE_LINKER_FLAGS_INIT "\
            ${LINKER_TARGET_FLAG} \
            --lto \
            --strict --scatter ${SECTIONS_SCRIPT_PATH} \
            --summary_stderr \
            --info summarysizes \
            --map --load_addr_map_info --xref --callgraph --symbols \
            --info sizes --info totals --info unused --info veneers \
            ")

# Enable ASM
#ENABLE_LANGUAGE(ASM)

# Check FPU
if(FPU_ENABLE)
    add_compile_definitions(ARM_MATH_CM4;ARM_MATH_MATRIX_CHECK;ARM_MATH_ROUNDING)
    add_compile_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
    add_link_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
else()
    add_compile_options(-mfloat-abi=soft -mfpu=none)
endif()

##################################################
#                Config Language                 #
##################################################

project(STM32PicoDK C CXX ASM)

set(CMAKE_C_STANDARD 99)

set(CMAKE_CXX_STANDARD 11)

##################################################
#                Config mcu file                 #
##################################################

add_definitions(-DUSE_HAL_DRIVER -DSTM32F103xB)

include_directories(
        Core/Inc
        Drivers/STM32F1xx_HAL_Driver/Inc/Legacy
        Drivers/STM32F1xx_HAL_Driver/Inc
        Drivers/CMSIS/Device/ST/STM32F1xx/Include
        Drivers/CMSIS/Include
        UserApp
)

file(GLOB_RECURSE SOURCES
        "Core/*.*"
        "Drivers/*.*"
        "UserApp/*.*"
        "UserApp/*.c"
        "UserApp/*.cpp"
        )

# add startup file
file(GLOB_RECURSE SOURCES
        ${SOURCES}
        "MDK-ARM/startup_stm32f103xb.s"
        )

# exclude file list
file(GLOB REMOVE_SOURCES
        "Core/Startup/*.*"
        "Core/Src/syscalls.c"
        "Core/Src/sysmem.c"
        "UserApp/TFT_Lcd/*.*"
        "STM32F103C8Tx_FLASH.ld"
        )

list(REMOVE_ITEM SOURCES ${REMOVE_SOURCES})

add_executable(${PROJECT_NAME} ${SOURCES} ${LINKER_SCRIPT}
        UserApp/example.h)

##################################################
#                  Config Build                  #
##################################################

set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)
set(ELF_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf)

set(ARMCC_FROMELF ${ARMCLANG_PATH}/fromelf.exe)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMENT "Building ${HEX_FILE}"
        COMMAND ${ARMCC_FROMELF} --i32combined --output="${HEX_FILE}" "${ELF_FILE}"
        )