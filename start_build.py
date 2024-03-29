#!/usr/bin/env python3
# -*- coding:utf-8 -*-

"""
@date: 2021-09-07 09:37:03
@author: shenglin.zhan@lightningsemi.com
@description: build script for windows/linux platform.
"""

import sys
import os
import shutil

def read_cmake_config():
    """
    Read configuration from top CMakeLists.txt.
    Return True on success, return False on failure.
    """
    global user_project
    global build_path

    with open("CMakeLists.txt", "r") as fObj:
        for line in fObj:
            if line.startswith(("set", "SET")):
                index = line.find("USER_PROJECT")
                if index >= 0:
                    left_bracket = line.find("(")
                    right_bracket = line.find(")")
                    if (left_bracket > 0) and (right_bracket > 0):
                        user_project = line[(left_bracket+1):right_bracket].split(" ")[-1]
                        print("user_project = {_p}".format(_p=user_project))
                        build_path = "-".join([build_path, user_project])
                        print("build_path = {_b}".format(_b=build_path))
                    return True
    print("Warning: read configuration from top CMakeLists.txt!!!")
    print("     Please check if there are following lines in CMakeLists.txt:")
    print("")
    print("set(USER_PROJECT  wifi_mcu_basic_example)")
    print("")
    return False


def show_usage():
    print("*****************************  usage  *****************************")
    print("argv[1] -- build action, such as clean/config/build/rebuild/jflash.")
    print("-------------------------------------------------------------------")
    print("NOTE: change project in top CMakeLists.txt, such as: ")
    print("set(USER_PROJECT   wifi_mcu_basic_example)")
    print("-------------------------------------------------------------------")
    print("*******************************************************************")


def action_clean():
    print("------  clean directory: {_b}  ------".format(_b=build_path))
    if os.path.exists(build_path):
        shutil.rmtree(build_path)


def action_config():
    global make_generator

    print("------  config: {_p}  ------".format(_p=user_project))
    cmd = "cmake -S . -B {_b}  -G \"{_g}\"".format(_b=build_path, _g=make_generator)
    os.system(cmd)


def action_build():
    global make_generator

    print("------  build: {_p}  ------".format(_p=user_project))
    if not os.path.exists(build_path):
        cmd = "cmake -S . -B {_b}  -G \"{_g}\"".format(_b=build_path, _g=make_generator)
        os.system(cmd)

    cmd = "cmake --build {_b}".format(_b=build_path)
    os.system(cmd)


def action_rebuild():
    global make_generator

    print("------  rebuild: {_p}  ------".format(_p=user_project))
    if os.path.exists(build_path):
        shutil.rmtree(build_path)

    cmd = "cmake -S . -B {_b} -G \"{_g}\"".format(_b=build_path, _g=make_generator)
    os.system(cmd)

    cmd = "cmake --build {_b}".format(_b=build_path)
    os.system(cmd)


def action_jflash():
    print("------  download flashimage.bin via JFlash  ------")

    jflash_exe = None
    if sys.platform == "linux":
        jflash_exe = "JFlash"
    elif sys.platform == "win32":
        jflash_exe = "JFlash.exe"
    else:
        print("Not supported platform!!!")
        exit(-1)

    opts = " -erasechip  -programverify  -startapp"
    cmd = "{_j} -openprj./tools/JFlash/LN881x.jflash  -open{_b}/bin/flashimage.bin,0x0 {_opt}".format(_j=jflash_exe, _b=build_path, _opt=opts)
    os.system(cmd)

    print("------  reset the chip and the code starts running...  ------")


# NOTE: read from top CMakeLists.txt.
user_project    = "wifi_mcu_basic_example"  # default user project.

# NOTE: first argument of this script.
user_action     = "config"                  # default action.

# NOTE: set up cmake build directory.
build_path      = "build"

# NOTE: select generator.
make_generator  = "Ninja"
# make_generator  = "Unix Makefiles"


def main():
    global user_project
    global user_action

    valid_action_collection = {
        "clean":   action_clean,
        "config":  action_config,
        "build":   action_build,
        "rebuild": action_rebuild,
        "jflash":  action_jflash
    }

    if not read_cmake_config():
        return

    if 1 == len(sys.argv):
        show_usage()
    else:
        if sys.argv[1] in valid_action_collection.keys():
            user_action = sys.argv[1]

    valid_action_collection.get(user_action)()


if __name__ == "__main__":
    main()
