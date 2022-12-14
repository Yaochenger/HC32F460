================================================================================
                                样例使用说明
================================================================================
版本历史
Date        Version     Author      IAR     MDK     GCC     Description
2022-03-31  1.0         CDT         7.70    5.16    8.3.1   First version
================================================================================
平台说明
================================================================================
GCC工程，由Eclipse IDE外挂GNU-ARM Toolchain，再结合pyOCD GDB Server实现工程的编译、
链接和调试。在用Eclipse导入工程后，请将xxxx_PyOCDDebug中pyocd-gdbserver和SVD文件
设置为正确的路径；请将xxxx_PyOCDDownload中pyocd设置为正确的路径。注意，这些路径不
能包含非英文字符。

================================================================================
功能描述
================================================================================
本样例展示INTC模块EXTINT检测功能。

说明：
本样例展示软件中断，并通过开发板上的按键KEY10，展示了全局中断、分组中断、
共享中断三种实现方式。

================================================================================
测试环境
================================================================================
测试用板:
---------------------
EV_F460_LQ100_Rev2.0

辅助工具:
---------------------
无

辅助软件:
---------------------
无

================================================================================
使用步骤
================================================================================
1）选择IRQ_TYPE（默认定义为IRQ_TYPE_GLOBAL），并重新编译；
2）启动IDE的下载程序并运行；
3）每按下一次按键K10
   若 IRQ_TYPE 定义为 IRQ_TYPE_GLOBAL ，则LED_BLUE切换一次亮灭状态；
   若 IRQ_TYPE 定义为 IRQ_TYPE_GROUP ，则LED_YELLOW切换一次亮灭状态；
   若 IRQ_TYPE 定义为 IRQ_TYPE_SHARE ，则LED_RED切换一次亮灭状态。

================================================================================
注意
================================================================================
1. 全局中断使用入口No.0~31中的No.0；
2. 分组中断使用入口No.32~39中的No.33；
3. 共享中断使用入口No.128。

================================================================================
