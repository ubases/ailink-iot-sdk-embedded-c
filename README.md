# AiLink使用说明



## 1. 什么是AiLink?

​	AiLink是一个协议栈，该协议栈内包含了MQTT协议、对接爱星云平台的IOT协议和配网协议。AiLink主要是由多个功能模块和主线程处理组合而成，从而通过该AiLink可直接接入爱星云平台。AiLink拥有基于MQTT通信的广域网模块、基于UDP通信的局域网模块、基于BLE通信的BLE配网模块、基于UDP通信的AP配网模块，以及拥有基于http通信的OTA模块。因此，AiLink是一个集合了多功能的协议栈。

​	AiLink内包含的MQTT协议是采用开源的MQTT协议源码，其源码[下载地址](https://os.mbed.com/teams/mqtt/code/MQTTPacket/)在这里。



## 2. AiLink有什么用途？

​	AiLink主要是负责将不同平台的IOT设备可快速地接入到爱星云平台上。但是，AiLink用途不仅仅在于接入爱星云。其用途可从两个角度上看，其一是从接入爱星云角度，其二是从嵌入式开发者角度。

​	从接入爱星云角度上看，因AiLink已实现了对接爱星云平台的IOT协议，用户开发者只需基于AiLink各个平台上的example进行二次开发其功能的业务需求，然后设备便可快速地接入了爱星云平台。

​	从嵌入式开发者角度上看，因AiLink仅仅是一个协议栈，只需根据不同平台进行适配其平台接口；嵌入式开发者便可使用AiLink在不同平台上的设备可快速地接入爱星云平台。

​	因此，AiLink是一个可移植性、可扩展性和易用性都比较高的协议栈；可协助用户开发者在不用平台上接入爱星云和二次开发业务需求的用途。

​	

## 3. AiLink下载

​	3.1 AiLink源码获取

```
git clone https://e.coding.net/axk/BAT_AIoT_PaaS/AiLink.git --recurse-submodules
```

​	3.2 AiLink目录结构说明

```

.
├── build.sh							// 编译脚本
├── coreMQTT							// MQTT开源协议栈
├── doc									// 文档说明
├── example								// 示例demo
│   └── bl602							// 博流bl602平台的示例demo
│       ├── transmit_2M					// bl602开源源码的2M固件demo工程
│       └── transmit_4M					// bl602开源源码的4M固件demo工程
├── lib									// AiLink静态库
├── libraries							// AiLink功能外部依赖库
│   └── bl_iot_sdk						// 博流bl602平台的SDK
├── LICENSE								// 开源协议文件
├── Makefile					
├── README.md
└── utils								// 开源功能模块源码目录
```



## 4. AiLink可支持平台

* bl602：

  * 博流SDK[下载地址](https://github.com/bouffalolab/bl_iot_sdk.git)

  * 博流资料

    * 博流[官网地址](https://www.bouffalolab.com/)
    * 博流[bl602芯片参考手册](https://dev.bouffalolab.com/media/doc/602/reference_manual/zh/html/index.html)
    * 博流[SDK入门编程指南](https://bouffalolab.github.io/bl_iot_sdk/)

  * 注意：

    在AiLink上编译bl602示例demo时，并不是使用博流开源上的SDK，而是使用了安信可这边根据需求已修改过的博流SDK（该SDK将会继续维护同步博流开源的SDK）。修改点如下：
    
    a. 修改SDK上日志打印的波特率为115200.
    
    b. 修改SDK上Makefile文件，可在编译生成ota文件
    
    c. 修改DHCPD_SERVER_IP 的ip地址为"192.168.4.1"，此为AP配网功能
    
    d. 修改SDK日志打印的buf大小

* 其他：



## 5. AiLink教程目录

​	1）[开发环境配置](doc/开发环境配置.md)

​	2）[开发指导](doc/开发指导手册.md)

​	3）[程序烧录说明](doc/BL602固件烧录教程.md)



## 6. 编译

* 执行build.sh脚本编译

  * 该文件在AiLink根目录上

  * 执行该文件时需传入两个参数（平台和示例demo文件夹名）。

    ```shell
    ./build.sh bl602 transmit_2M
    ```

  * 注意：在执行build.sh脚本时，该脚本执行的步骤如下：

    * 把AiLink上的c文件编译生成libAilink静态库，存放于lib文件上。
    * 当执行build.sh脚本编译源码demo时，将会将AiLink上的c文件拷贝到示例demo进行更新最新c文件。
    * 当执行build.sh脚本编译静态库demo时，将会将AiLink的libAilink静态库拷贝到示例demo上。
    * 最后编译指定demo工程的程序，并打包生成固件包

* 在示例demo上，执行genromap脚本编译

  * 进入bl602示例demo工程后，可执行以下指令后，便可生成二进制固件，固件存放于build_out文件夹中（固件名Project.bin）。

    ```
    ./genromap
    ```

* 在示例demo上，执行buildscript.sh脚本编译

  * 该编译脚本是在shellscript文件夹中

  * 该脚本主要是bl602程序工程的编译与固件打包功能。

  * 执行以下指令编译工程程序和打包固件

    ```
    ./buildscript.sh
    ```

    







