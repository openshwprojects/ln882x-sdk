python ..\..\..\tools\user_cmd\after_build_soc.py @L

..\..\..\tools\bin\mkimage.exe cmd_app ..\..\..\lib\boot_ln882x.bin @L.bin @L.asm flashimage.bin flash_partition_cfg.json ver=1.1

