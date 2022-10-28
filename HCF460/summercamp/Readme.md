## 学习营目标

#### 第一周目标

使学生能在一周的学习后，可以实现对 RT-Thread 有基本了解，掌握以下技能：

* 内核

    * 对线程有了解，知晓线程的知识，并且正确认识 mian, finish, idle 三个特殊的线程函数
    * 对 IPC 有了解，知晓 IPC 如何干预线程的运行，以及抢占式 RTOS 的线程运行特点
    * 知晓 RT-Thread 的启动流程，对系统资源的初始化有印象

    > 对ARM，RISC-V的启动汇编可以进行简单分析；
    >
    > 结合 Map 可以知晓 RO, RW, ZI 数据的意义及存储特点；

* 组件

    * NET 下辖的 AT、lwIP 有基础了解，知道如何使用
    * Driver 下辖的 spi、iic、uart、wifi驱动可以使用
    * 文件系统的使用（挂载，使用），多种文件系统有印象，不要求使用
    * I/O设备管理器，RT-Thread Object 对象管理的意义

    > 对预编译命令，sizeof，rt_container_of 的理解以及使用。
    >
    > 对预编译，编译，链接有基础的理解；对链接脚本如何干预 Map 有印象

* 工具类

    * Kconfig，Sconscript, rtconfig.py 如何介入工程的管理和 scons 命令有了解，可以简单更改Kconfig与Sconscript来干预工程的生成与维护
    * Git 的使用，github, gitee 的使用

    > 知晓Sconscirpt与rtconfig.py 实际上是python代码，对于嵌入式开发来说，是很好的组织工具

* 软件包

    * IIC 下 MPU6050，DTH11 的使用
    * SPI下 RW007 的使用 或者 AT 下 ESP8266 的使用
    * Kawaii-mqtt 的使用，MQTT 云平台的连接

