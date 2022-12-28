#!/bin/bash

PROJECT_PATH=../
PROJECT_NAME=Project
OTA_PATH=$BL60X_SDK_PATH/tools/flash_tool/chips/bl602/ota
WHOLE_BIN_PATH=$BL60X_SDK_PATH/tools/flash_tool/chips/bl602/img_create_iot
BOOT_PATH=$PROJECT_NAME/components/iot_sys/boot
DTS_PATH=$PROJECT_NAME/components/iot_sys/device_tree
PARTITION_PATH=$PROJECT_NAME/components/iot_sys/partition
FW_INFO_PATH=$PROJECT_NAME/components/iot_sys/include
PACK_PATH=shellscript

echo start building...


cd $PROJECT_PATH


build_time=`date "+%Y%m%d.%H%M%S"`"."`date "+%N" | cut -c 1-3`
echo $build_time

# build
fw_build(){
    rm -rf build_out/$PROJECT_NAME
    rm -rf build_out/ailink
    make COMPILE_TIME=$build_time CONFIG_CHIP_NAME=BL602 CONFIG_LINK_ROM=1 CONFIG_BLE_TP_SERVER=1 CONFIG_BLECONTROLLER_LIB=$3 -j
    cp -r $OTA_PATH build_out
}



fw_file_path="./"$FW_INFO_PATH"/iot_sys.h"
echo $fw_file_path
(dos2unix $fw_file_path)
fw_product_id=$(cat ${fw_file_path} | grep "#define DEVICE_PRODUCT_ID " | tr -s ' '| cut -d " " -f 3 | awk '{print $0}')
fw_version=$(cat ${fw_file_path} | grep "#define DEVICE_FW_VERSION " | tr -s ' '| cut -d " " -f 3 | awk '{print $0}')

echo "found fw version: " $fw_version
if [ ! $fw_product_id ] || [ ! $fw_version ];then
	# 如果找不到,也许是文件名大小写不一致, 转换为小写后再尝试查找
	fw_file_path=$(echo $fw_file_path | tr '[A-Z]' '[a-z]')
	fw_product_id=$(cat ${fw_file_path} | grep "#define DEVICE_PRODUCT_ID " | tr -s ' '| cut -d " " -f 3 | awk '{print $0}')
	fw_version=$(cat ${fw_file_path} | grep "#define DEVICE_FW_VERSION " | tr -s ' '| cut -d " " -f 3 | awk '{print $0}')
	if [[ ! $fw_product_id ]] || [[ ! $fw_version ]];then
		echo "Error!! Not found fw info, please check whether if this path is correct: " $fw_file_path
		exit 0
	fi
else
	# 去除 (" ")
	fw_product_id=`echo $fw_product_id | sed -e 's/(\"//g'`
	fw_product_id=`echo $fw_product_id | sed -e 's/\")//g'`
	fw_version=`echo $fw_version | sed -e 's/(\"//g'`
	fw_version=`echo $fw_version | sed -e 's/\")//g'`
	echo "found fw product id: " $fw_product_id
	echo "found fw version: " $fw_version
fi
echo "found fw product id: " $fw_product_id
echo "found fw version: " $fw_version


# check and create dir
check_and_mkdir(){
	if [ ! -d $1 ];then
		echo "creat dir:" $1
		mkdir $1
	fi
}



otapack_ini_path="./"$PACK_PATH"/otapack.ini"
(dos2unix $otapack_ini_path)
echo $otapack_ini_path
sed -in "s/.*OTA version=.*/OTA version=$fw_version/" $otapack_ini_path
sed -in "s/.*Product ID=.*/Product ID=$fw_product_id/" $otapack_ini_path

ota_pack_file=pack.sh
ota_file_path="./"$PACK_PATH"/"$ota_pack_file

# 传入debug或者release后执行 ota的打包脚本
do_ota_pack(){
	if [ -f ${ota_file_path} ];then
		(dos2unix $ota_file_path)
		echo "found ota packing file: " ${ota_file_path}
		pwd_now=$PWD
		cd ./${PACK_PATH}
		source ${ota_pack_file}
		if [ $? -ne 0 ];then
			 echo "Failed!."
			 read
		fi
		mv "./ota_pack.bin" "./ota_pack_$1.bin"
		cd ${pwd_now}
	else
	  echo "not found ota packing file."
	fi
}

CreateFolder(){

    # build_time=`date "+%Y%m%d.%H%M%S"`"."`date "+%N" | cut -c 1-3`
    # echo $build_time
    # creat output root dir
    current_git_branch_latest_id=`git rev-parse HEAD`
    output_dir="./"$fw_product_id"_"$fw_version"_"$build_time"/"
    check_and_mkdir $output_dir

    # creat Develop dir
    develop_dir="Develop/"
    output_develop=$output_dir$develop_dir
    check_and_mkdir $output_develop

    # create debug dir
    debug_dir=$output_develop"debug/"
    check_and_mkdir $debug_dir

    # create release dir
    release_dir=$output_develop"release/"
    check_and_mkdir $release_dir

    # creat Productiion dir
    production_dir="Production/"
    output_production=$output_dir$production_dir
    check_and_mkdir $output_production

    # creat OTA Output dir
    sdk_dir="sdk/"
    output_sdk=$output_dir$sdk_dir
    check_and_mkdir $output_sdk

    # creat OTA Output dir
    ota_output_dir="OTA/"
    output_ota=$output_dir$ota_output_dir
    check_and_mkdir $output_ota

    # create ota debug dir
    ota_debug_dir=$output_ota"debug/"
    check_and_mkdir $ota_debug_dir

    # create ota release dir
    ota_release_dir=$output_ota"release/"
    check_and_mkdir $ota_release_dir

    echo $current_git_branch_latest_id >$output_dir/"commit".txt
}



# 当执行脚本时没有带参数 -k 则执行idf.py build 编译 debug版本与release版本固件
if [ $# -eq 0 ];then

    # 增加 device log
    var_tmp=$(cat ${fw_file_path} | grep "#define DEVICE_LOG_ENABLE")
    if [ "$var_tmp" ];then
        sed -in 's/.*#define DEVICE_LOG_ENABLE.*/#define DEVICE_LOG_ENABLE/' $fw_file_path
        echo $var_tmp
        fw_build
    else
        sed -in '/#ifdef DEVICE_LOG_ENABLE/i\#define DEVICE_LOG_ENABLE' $fw_file_path
        echo $var_tmp
        fw_build
    fi

else

    if [ "pack" == $1 ];then
        # 关闭 device log
        echo $fw_file_path

        CreateFolder
        cp -r $BOOT_PATH/boot2_isp_release.bin $output_sdk
        cp -r $DTS_PATH/bl_factory_params_IoTKitA_40M.dts $output_sdk
        cp -r $PARTITION_PATH/4M_partition_ailink_bl602.toml $output_sdk
        sha256sum $output_sdk"boot2_isp_release.bin" > $output_sdk"SHA256.txt"
        sha256sum $output_sdk"bl_factory_params_IoTKitA_40M.dts" >> $output_sdk"SHA256.txt"
        sha256sum $output_sdk"4M_partition_ailink_bl602.toml" >> $output_sdk"SHA256.txt"

        var_tmp=$(cat ${fw_file_path} | grep "#define DEVICE_LOG_ENABLE")
        if [ "$var_tmp" ];then
            sed -in 's/.*#define DEVICE_LOG_ENABLE.*/\/\/ #define DEVICE_LOG_ENABLE/' $fw_file_path
            echo $var_tmp
            fw_build

            cp -r $WHOLE_BIN_PATH/whole_flash_data.bin $output_production
            cp -r build_out/$PROJECT_NAME.bin $release_dir
            cp -r build_out/$PROJECT_NAME.elf $release_dir

            cp -r $OTA_PATH/FW_OTA.bin.xz.ota $PACK_PATH
            do_ota_pack "release"
            cp -r $PACK_PATH/"ota_pack_release.bin" $ota_release_dir
            rm -r $PACK_PATH/"ota_pack_release.bin"
            rm -r $PACK_PATH/FW_OTA.bin.xz.ota
            
            sha256sum $release_dir"$PROJECT_NAME.bin" > $release_dir"SHA256.txt"
            sha256sum $ota_release_dir"ota_pack_release.bin" > $ota_release_dir"SHA256.txt"
            sha256sum $output_production"whole_flash_data.bin" > $output_production"SHA256.txt"
        fi

        # 增加 device log
        var_tmp=$(cat ${fw_file_path} | grep "#define DEVICE_LOG_ENABLE")
        if [ "$var_tmp" ];then
            sed -in 's/.*#define DEVICE_LOG_ENABLE.*/#define DEVICE_LOG_ENABLE/' $fw_file_path
            echo $var_tmp
            fw_build

            cp -r build_out/$PROJECT_NAME.bin $debug_dir
            cp -r build_out/$PROJECT_NAME.elf $debug_dir

            cp -r $OTA_PATH/FW_OTA.bin.xz.ota $PACK_PATH
            do_ota_pack "debug"
            cp -r $PACK_PATH/"ota_pack_debug.bin" $ota_debug_dir
            rm -r $PACK_PATH/"ota_pack_debug.bin"
            rm -r $PACK_PATH/FW_OTA.bin.xz.ota

            sha256sum $debug_dir"$PROJECT_NAME.bin" > $debug_dir"SHA256.txt"
            sha256sum $ota_debug_dir"ota_pack_debug.bin" > $ota_debug_dir"SHA256.txt"
        else
            sed -in '/#ifdef DEVICE_LOG_ENABLE/i\#define DEVICE_LOG_ENABLE' $fw_file_path
            echo $var_tmp
            fw_build

            cp -r build_out/$PROJECT_NAME.bin $debug_dir
            cp -r build_out/$PROJECT_NAME.elf $debug_dir

            cp -r $OTA_PATH/FW_OTA.bin.xz.ota $PACK_PATH
            do_ota_pack "debug"
            cp -r $PACK_PATH/"ota_pack_debug.bin" $ota_debug_dir
            rm -r $PACK_PATH/"ota_pack_debug.bin"
            rm -r $PACK_PATH/FW_OTA.bin.xz.ota

            sha256sum $debug_dir"$PROJECT_NAME.bin" > $debug_dir"SHA256.txt"
            sha256sum $ota_debug_dir"ota_pack_debug.bin" > $ota_debug_dir"SHA256.txt"
        fi
    elif [ "release" == $1 ];then
        # 关闭 device log
        echo $fw_file_path
        var_tmp=$(cat ${fw_file_path} | grep "#define DEVICE_LOG_ENABLE")
        if [ "$var_tmp" ];then
            sed -in 's/.*#define DEVICE_LOG_ENABLE.*/\/\/ #define DEVICE_LOG_ENABLE/' $fw_file_path
            echo $var_tmp
            fw_build
        fi
    else
        # 增加 device log
        var_tmp=$(cat ${fw_file_path} | grep "#define DEVICE_LOG_ENABLE")
        if [ "$var_tmp" ];then
            sed -in 's/.*#define DEVICE_LOG_ENABLE.*/#define DEVICE_LOG_ENABLE/' $fw_file_path
            echo $var_tmp
            fw_build
            
        else
            sed -in '/#ifdef DEVICE_LOG_ENABLE/i\#define DEVICE_LOG_ENABLE' $fw_file_path
            echo $var_tmp
            fw_build
        fi
    fi
fi

otapack_ini_path_temp="./"$PACK_PATH"/otapack.inin"
fw_file_path_temp="./"$FW_INFO_PATH"/iot_sys.hn"

echo $otapack_ini_path_temp
echo $fw_file_path_temp

rm -r $otapack_ini_path_temp
rm -r $fw_file_path_temp

# echo finish build