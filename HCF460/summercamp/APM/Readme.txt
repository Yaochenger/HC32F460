-------------------------
J-LINK

1. JlinkDevice.xml 粘贴到 Jlink-7.5a 下的 JlinkDevice.xml 文件中，注意：是增加不是替换
2. Geehy 文件夹放到 J-Link-7.5a/Device目录下
3. Geegy 的串口没有引出到调试器端，需要手动接 PA9, PA10 作为 Shell 输出


--------------------------
DAP-LINK

1. 复制 APEXMIC.APM32F1xx_DFP.1.0.4.pack 到 .\RT-ThreadStudio\repo\Extract\Debugger_Support_Packages\RealThread\PyOCD\0.1.3\packs 下
2. 复制 update_yaml.exe 到 \RT-ThreadStudio\repo\Extract\Debugger_Support_Packages\RealThread\PyOCD\0.1.3\packs 路径下 
3. 在点击 update_yaml.exe
4. Geegy 的串口没有引出到调试器端，需要手动接 PA9, PA10 作为 Shell 输出