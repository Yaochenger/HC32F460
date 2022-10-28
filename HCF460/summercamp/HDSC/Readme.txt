1. PYOCD 应该更新，否则找不到器件
2. Studio 内没有可选的板级支持包，需要添加

------
如果上述手段没有到位，需要在 \repo\Extract\Debugger_Support_Packages\RealThread\PyOCD\0.1.3\packs 中添加 HDSC.HC32F460.xxx.pack
并且在上一级的 pyocd.ymal 中添加该芯片的支持，这个步骤使用 update_ymal.exe 就行，这个 exe 也放在 \packs 目录下执行