# generate flags from user variables
if(CMAKE_BUILD_TYPE   MATCHES  Debug)
  set(DBGFLAGS  "-ggdb")
elseif(CMAKE_BUILD_TYPE  MATCHES  Release)
  set(DBGFLAGS  "-O1 -s")
endif()

# set CFLAGS
set(CPU         "-mcpu=cortex-m4")
set(FPU         "-mfpu=fpv4-sp-d16")
set(FLOAT_ABI   "-mfloat-abi=hard")
set(MCU         "-mthumb  -mabi=aapcs  ${CPU}  ${FPU}  ${FLOAT_ABI}")

add_definitions(
  -DLN882x
  -DARM_MATH_CM4
)

set(WARNFLAGS   "-Wall  -Wextra  -Wundef  -Wshadow  -Wredundant-decls          \
    -Wstrict-prototypes  -Wimplicit-function-declaration  -Wmissing-prototypes \
    -Wdouble-promotion  -Wfloat-conversion  -pedantic"
)

set(PREPFLAGS   "-MD -MP")
set(SPECSFLAGS  "")
set(ASFLAGS     ${MCU})
set(ARCHFLAGS   ${MCU})
set(OPTFLAGS    ${OPTFLAGS})
set(LINK_FLAGS  "${ARCHFLAGS} -Wl,--gc-sections")

set(CMAKE_C_FLAGS   "${ARCHFLAGS}  ${OPTFLAGS}  ${DBGFLAGS}  ${WARNFLAGS}  ${PREPFLAGS} \
    -std=gnu99 -ffunction-sections -fdata-sections -fno-strict-aliasing ${SPECSFLAGS}"
    CACHE INTERNAL "C compiler flags"
)

set(CMAKE_C_STANDARD    99)

# NOT used.
# set(CMAKE_CXX_FLAGS  "${ARCHFLAGS}  ${OPTFLAGS}  ${DBGFLAGS}  ${CXXWARNFLAGS}  ${PREPFLAGS} \
#     -std=gnu++11  -ffunction-sections  -fdata-sections  -fno-strict-aliasing \
#     -fno-rtti  -fno-exceptions  ${SPECSFLAGS}"
#     CACHE INTERNAL "Cxx compiler flags"
# )

set(CMAKE_ASM_FLAGS  "${ASFLAGS}  -x  assembler-with-cpp  ${DBGFLAGS}"
    CACHE INTERNAL "ASM compiler flags"
)

# set Link script
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/gcc/ln882x.ld")

set(EXTRA_LINK_FLAGS  "-Wl,-Map=${PROJECT_NAME}.map,--cref,--no-warn-mismatch \
    -Wl,--print-memory-usage  --specs=nano.specs  --specs=nosys.specs"
)

set(CMAKE_EXE_LINKER_FLAGS  "${LINK_FLAGS}"
    CACHE INTERNAL "Exe linker flags"
)

set(CMAKE_C_OUTPUT_EXTENSION_REPLACE  1)
