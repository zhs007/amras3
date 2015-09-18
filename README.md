# 基于actionscript的AMR编码解码器

### 功能
让flash能支持AMR文件的编码解码。

使用了3GPP的C核心AMR解码器代码 **[(AMR) Speech Codec - 3GPP][1]** ，通过 **[CrossBridge][2]** 编译成swc做核心解码功能。

关于如何播放AMR，可以参考另外一个项目 **[imspeechas3][3]**

### 适用性

1. 编码器的输入流是纯PCM数据流，也就是不带WAV头的，PCM数据流的采样率只能是8khz的，声道可以是单声道/双声道，bps可以是8位/16位。
2. 解码器的输出流也是纯PCM数据流，始终是8khz、单声道、16位。


  [1]: http://www.3gpp.org/DynaReport/26104.htm
  [2]: http://crossbridge.io/
  [3]: https://github.com/zhs007/imspeechas3
