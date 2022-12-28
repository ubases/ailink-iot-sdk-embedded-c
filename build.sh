#!/bin/bash

EXAMPLE_PATH=example
AILINK_PATH=Project/components/ailink
SHELLSCRIPT_PATH=shellscript
COREALINK_PATH=coreAilink
UTILS_PATH=utils
DTS_PATH=Project/components/iot_sys/device_tree
PARTITION_PATH=Project/components/iot_sys/partition
SDK_DTS_PATH=$BL60X_SDK_PATH/tools/flash_tool/chips/bl602/device_tree
SDK_PARTITION_PATH=$BL60X_SDK_PATH/tools/flash_tool/chips/bl602/partition


#平台
platform=("linux" "bl602")

#指定example
example_file=("transmit_2M" "transmit_4M")

num=0
platform_num=${#platform[*]}


if [ $# -lt 2 ];then
    echo "Missing parameters"
    exit 1
fi

echo "second param is "$2

case $1 in
    ${platform[0]})
        echo "linux"
        cc="gcc -m32"
        ar="ar"
        make clean
        make CC="$cc" AR="$ar"
        ;;
    ${platform[1]})
        echo "bl602"
        compileinfo="riscv64-unknown-elf"
        ar="riscv64-unknown-elf"
        ccflags="-march=rv32imafc -mabi=ilp32f -Wl,--cref -nostartfiles -Wl,--gc-sections -Wl,-static -Wl,--start-group -ffreestanding"

        if [ $3 ]; then
            if [ $3 == "build" ]; then
                echo "build coreAilink"
                make clean
                make compile="$compileinfo" platform_dir="${platform[1]}" EXTRA_CCFLAGS="$ccflags"
            fi
        fi
        ;;
esac

if [ $? -eq 0 ]; then
    mkdir -p lib
    mv *.a lib/
    echo "/====================================================="
    echo "compile time: `date`"
    echo "/====================================================="
fi

CURRENT_PATH=$pwd

case $2 in
    ${example_file[0]})
        echo "transmit"

        cp -r $EXAMPLE_PATH/$1/$2/$PARTITION_PATH/2M_partition_ailink_bl602.toml $SDK_PARTITION_PATH
        mv $SDK_PARTITION_PATH/2M_partition_ailink_bl602.toml $SDK_PARTITION_PATH/partition_cfg_2M.toml

        cp -r $EXAMPLE_PATH/$1/$2/$DTS_PATH/bl_factory_params_IoTKitA_40M.dts $SDK_DTS_PATH
        mv $SDK_DTS_PATH/bl_factory_params_IoTKitA_40M.dts $SDK_DTS_PATH/bl_factory_params_IoTKitA_40M.dts

        if [ $3 ]; then
            if [ $3 == "build" ]; then
                echo "process coreAilink and process $2 example"
                cp -r $COREALINK_PATH/include/ailink_ota.h $EXAMPLE_PATH/$1/$2/$AILINK_PATH/include
                cp -r $COREALINK_PATH/include/ailink_profile.h $EXAMPLE_PATH/$1/$2/$AILINK_PATH/include
                cp -r $COREALINK_PATH/include/ailink_utils.h $EXAMPLE_PATH/$1/$2/$AILINK_PATH/include
                cp -r $UTILS_PATH/cJSON.h $EXAMPLE_PATH/$1/$2/$AILINK_PATH/include
                cp -r $UTILS_PATH/ringbuff.h $EXAMPLE_PATH/$1/$2/$AILINK_PATH/include
                cp -r $COREALINK_PATH/include/ailink_softtimer.h $EXAMPLE_PATH/$1/$2/$AILINK_PATH/include
                cp -r lib $EXAMPLE_PATH/$1/$2/$AILINK_PATH/
                cd $EXAMPLE_PATH/$1/$2/$SHELLSCRIPT_PATH
                ./buildscript.sh
                cd $CURRENT_PATH
            else
                echo "process $2 example"
                cd $EXAMPLE_PATH/$1/$2/$SHELLSCRIPT_PATH
                ./buildscript.sh $3
                cd $CURRENT_PATH
            fi
        else
            echo "process $2 example"
            cd $EXAMPLE_PATH/$1/$2/$SHELLSCRIPT_PATH
            ./buildscript.sh
            cd $CURRENT_PATH
        fi
        ;;
    ${example_file[1]})
        echo "transmit"

        cp -r $EXAMPLE_PATH/$1/$2/$PARTITION_PATH/4M_partition_ailink_bl602.toml $SDK_PARTITION_PATH
        mv $SDK_PARTITION_PATH/4M_partition_ailink_bl602.toml $SDK_PARTITION_PATH/partition_cfg_2M.toml

        cp -r $EXAMPLE_PATH/$1/$2/$DTS_PATH/bl_factory_params_IoTKitA_40M.dts $SDK_DTS_PATH
        mv $SDK_DTS_PATH/bl_factory_params_IoTKitA_40M.dts $SDK_DTS_PATH/bl_factory_params_IoTKitA_40M.dts

        if [ $3 ]; then
            if [ $3 == "build" ]; then
                echo "process coreAilink and process $2 example"
                cp -r $COREALINK_PATH/include/ailink_ota.h $EXAMPLE_PATH/$1/$2/$AILINK_PATH/include
                cp -r $COREALINK_PATH/include/ailink_profile.h $EXAMPLE_PATH/$1/$2/$AILINK_PATH/include
                cp -r $COREALINK_PATH/include/ailink_utils.h $EXAMPLE_PATH/$1/$2/$AILINK_PATH/include
                cp -r $UTILS_PATH/cJSON.h $EXAMPLE_PATH/$1/$2/$AILINK_PATH/include
                cp -r $UTILS_PATH/ringbuff.h $EXAMPLE_PATH/$1/$2/$AILINK_PATH/include
                cp -r $COREALINK_PATH/include/ailink_softtimer.h $EXAMPLE_PATH/$1/$2/$AILINK_PATH/include
                cp -r lib $EXAMPLE_PATH/$1/$2/$AILINK_PATH/
                cd $EXAMPLE_PATH/$1/$2/$SHELLSCRIPT_PATH
                ./buildscript.sh
                cd $CURRENT_PATH
            else
                echo "process $2 example"
                cd $EXAMPLE_PATH/$1/$2/$SHELLSCRIPT_PATH
                ./buildscript.sh $3
                cd $CURRENT_PATH
            fi
        else
            echo "process $2 example"
            cd $EXAMPLE_PATH/$1/$2/$SHELLSCRIPT_PATH
            ./buildscript.sh
            cd $CURRENT_PATH
        fi
        ;;
esac

