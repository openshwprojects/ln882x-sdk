- [1. CMake Introduction](#1-cmake-Introduction)
- [2. Environment Configuration](#2-Environment Configuration)
- [3. Project Construction](#3-Project Construction)
- [3.1 Project Structure](#31-Project Structure)
- [3.2 How to Build a Library](#32-How to Build a Library)
- [3.3 How to Publish a Library](#33-How to Publish a Library)
- [3.3.1 The first step is to compile the Release version of the library](#331-The first step is to compile the release version of the library)
- [3.3.2 The second step is to compile, copy and publish](#332-The second step is to compile, copy and publish)
- [3.4 How to Build](#34-How to Build)
- [3.4.1 Simple Build Steps](#341-Simple Build Steps)
- [3.4.1.1 Script `start_build.py` User Guide](#3411-Script-start_buildpy-User Guide)
- [3.4.1.2 Notes](#3412-Notes)
- [3.4.2 Command Line Build Steps](#342-Command Line Build Steps)

# 1. Introduction to CMake

- CMake is a cross-platform build tool that can output various Makefiles or project files;
- CMake does not directly build the final target, but generates a standard Makefile file or build.ninja project file, and then uses the Make tool to read the Makefile file for compilation or the Ninja tool to read build.ninja for compilation;
- CMake can select a generator and can generate multiple different types of project files on the same platform. For example, on the Windows platform, it can generate both VisualStudio project files and MinGW Makefiles or Unix Makefiles.

The `CMakeLists.txt` file is used in the project directory to describe CMake rules and build targets, similar to `Makefile`.

# 2. Environment configuration

See the file [**lightningsemi_sdk_cross_build_setup.pdf**](./lightningsemi_sdk_cross_build_setup.pdf)

# 3. Project construction

## 3.1 Project structure

- The `components` directory stores various components, such as `atcmd`, `airkiss` and `wifi` firmware. Each component contains a `CMakeLists.txt`. During the compilation process, the components will be compiled into libraries. Most of the source code is provided to customers, but the `wifi` firmware is compiled into a library `libwifi.a`, which is stored in the `lib/gcclib/` directory;
- The `doc` directory stores documents;
- The `project` directory stores user product-related codes, all of which are source codes;
- The `lib` directory stores library files. Both Keil and GCC compiled libraries are stored in this directory;
- The `mcu` directory stores LN882X Chip-related drivers are compiled into the library `libdriver_ln882x.a` through CMake and stored in the `lib/gcclib/` directory;
- The `tools` directory stores the tools used in the build process;
- The `CMakeLists.txt` top-level file describes the relationship between the component and the final goal of the user project, and controls whether the component participates in the compilation through the `option()` option; the `option()` in the top-level directory `CMakeLists.txt` sets the default state, and you can set whether the component participates in the compilation in the user project. For example, in the *wifi_mcu_basic_example* project, you only need to add a line of statement `set(COMP_ATCMD_SUPPORT ON PARENT_SCOPE)` to enable the atmcd component;

## 3.2 How to build a library

Each component contains a `CMakeLists.txt` file, which describes the source and header file paths that the component depends on, as well as the name of the final generated library file.

The top-level `CMakeLists.txt` file describes the relationship between the component and the final goal of the user project. The `option()` option is used to control whether the component participates in the compilation. The `option()` in the top-level directory `CMakeLists.txt` sets the default state. You can set whether the component participates in the compilation in the user project, for example, `set(COMP_AIRKISS_SUPPORT ON PARENT_SCOPE)` enables the component `airkiss` to participate in the compilation.

## 3.3 How to publish the library

### 3.3.1 The first step is to compile the Release version of the library

When compiling and generating the library file, you need to turn on the following settings in the top-level `CMakeLists.txt` file.

It should be noted that when publishing the release version, these two options must be set to `OFF`, otherwise the compilation will fail; only in the development stage, both are set to `ON`.

```
# Compile release version
set(CMAKE_BUILD_TYPE Release)

# Include driver_ln882x in the compilation
option(COMP_MCU_LN882X_SUPPORT "MCU LN882X driver." ON)

# Include wifi library in the compilation
option(COMP_WIFI_SUPPORT "WiFi firmware." ON)
```

### 3.3.2 Step 2, compile, copy and publish

For example, enter **`start_build.py rebuild`** in the command line to start compiling. The library files generated in the step are in the **build/lib** directory. Copy the two files `libwifi.a` and `libdriver_ln882x.a` to the **lib/gcclib/** directory.

## 3.4 How to build

### 3.4.1 Simple build steps

See the top-level directory `start_build.py`. When no parameters are passed to this script, it will print the usage and automatically use the default configuration.

In the top-level **CMakeLists.txt** file, find the first section to set the user project. For example, select a user project from the following, shield other variable settings, and set the user project to **wifi_mcu_basic_example** as follows:

```shell
# ... #################### ############################################################################ set(USER_PROJECT wifi_mcu_basic_example) # set(USER_PROJECT wifi_mcu_airkiss_demo) # set(USER_PROJECT wifi_mcu_ota_demo) # set(USER_PROJECT wifi_mcu_qcloud_misc_demo) # set(USER_PROJECT wifi_mcu_smartconfig_demo) # set(USER_PROJECT wifi_mcu_smartliving_demo) # set(USER_PROJECT wifi_mcu_softapconfig_demo)
```

The output file of GCC compilation is `build-xxxx/xxx.elf`. After being processed by our tool, the file generated and burned to Flash is **build-xxx/bin/flashimage.bin**.

#### 3.4.1.1 Script `start_build.py` Usage Guide

This script is common to Windows/Linux platforms.

Usage is as follows:

1. In the top-level directory, `start_build.py` without parameters will print the usage of the script. Please read the usage instructions in detail;

2. In the top-level directory, `start_build.py clean` clears the `build-xxx/` directory and clears the last compilation result;

3. In the top-level directory, `start_build.py config` configures the user project to be generated in the `build-xxx/` directory;

4. In the top-level directory, `start_build.py build` starts compiling the project based on the previous step;

5. In the top-level directory, `start_build.py rebuild` will automatically execute the above 3 steps in sequence;

6. In the top-level directory, `start_build.py jflash` will call the **J-Flash** tool to download the generated `build-xxx/bin/flashimage.bin` file to the module (please make sure that the J-Link USB is connected correctly and the module is powered on);

#### 3.4.1.2 Notes

If you are on Windows For platform compilation, please exit the antivirus software (XXX Security Guard or XXX Computer Manager) first, because the antivirus software runs the build tool in an isolated sandbox and scans the compiled files, which makes the compilation speed very slow.

### 3.4.2 Command line build steps

1. Create a **build-xxx/** folder in the top-level directory of the project, which is used to store intermediate build files and final output files;
2. Enter the **build-xxx/** folder on the command line, and then enter `cmake -DXXX_ARG=XXX_VAL ..` to start generating Makefile. Here, a parameter `XXX_ARG` is passed to CMake, and its value is `XXX_VAL`, where `..` indicates that the source code is located in the upper directory, the variable `CMAKE_SOURCE_DIR` indicates the top-level directory of the source code, and `CMAKE_BINARY_DIR` indicates the directory where the build is located, usually the **build-xxx/** directory;
3. The command line is in the **build-xxx/** folder, and enter `cmake --build .` to start compiling.