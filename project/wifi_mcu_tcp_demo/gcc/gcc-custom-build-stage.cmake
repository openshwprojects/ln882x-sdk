################################################################################
#####################   update flash partition table first  ####################
################################################################################
set(FLASH_PYTHON_SCRIPT   ${CMAKE_SOURCE_DIR}/tools/user_cmd/before_build.py)
set(FLASH_PART_CFG_JSON   ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/cfg/flash_partition_cfg.json)
set(FLASH_PART_TABLE_FILE ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/cfg/flash_partition_table.h)

execute_process(COMMAND python ${FLASH_PYTHON_SCRIPT} ${FLASH_PART_CFG_JSON} ${FLASH_PART_TABLE_FILE}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/cfg
)

################################################################################
################################   make image  #################################
################################################################################
set(BIN_TARGET    ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.bin)
set(HEX_TARGET    ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.hex)
set(MAP_TARGET    ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.map)
set(ASM_TARGET    ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.diasm)
set(FLASH_TARGET  ${EXECUTABLE_OUTPUT_PATH}/flashimage.bin)

# post build:
# 1) *.elf --> *.bin
# 2) *.bin + boot_ln882x.bin --> flashimage.bin
add_custom_command(TARGET  ${TARGET_ELF_NAME}
        POST_BUILD
        COMMAND  ${CMAKE_OBJCOPY}  -O  ihex     ${EXECUTABLE_OUTPUT_PATH}/${TARGET_ELF_NAME}     ${HEX_TARGET}
        COMMAND  ${CMAKE_OBJCOPY}  -O  binary   ${EXECUTABLE_OUTPUT_PATH}/${TARGET_ELF_NAME}     ${BIN_TARGET}
        COMMAND  ${CMAKE_OBJDUMP}  -S           ${EXECUTABLE_OUTPUT_PATH}/${TARGET_ELF_NAME}  >  ${ASM_TARGET}
        COMMAND  ${CMAKE_SIZE}                  ${EXECUTABLE_OUTPUT_PATH}/${TARGET_ELF_NAME}
        COMMAND  ${LN_MKIMAGE}     cmd_app      ${CMAKE_SOURCE_DIR}/lib/boot_ln882x.bin  ${BIN_TARGET}  ${ASM_TARGET}  ${FLASH_TARGET}  ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/cfg/flash_partition_cfg.json  ver=1.1
        WORKING_DIRECTORY  ${EXECUTABLE_OUTPUT_PATH}
        COMMENT  ">>>>>>  Generating ${HEX_TARGET}, ${BIN_TARGET}, ${ASM_TARGET}, ${FLASH_TARGET}  <<<<<<"
)
