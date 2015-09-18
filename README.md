# 基于actionscript的AMR编码解码器
使用了3GPP的C核心AMR解码器代码，通过CrossBridge编译成swc做核心解码功能。
注意：

编码的输入流只能是8khz的，可以是单声道、双声道，可以是8位和16位。
解码的输出流始终是8khz、单声道、16位。
