# 把安卓手机的系统刷成其他系统（⚠️注意，这一步并非获得root权限，只是看自己手机系统不爽，想要安装一个更加纯净的系统）
# 刷系统的主要步骤
1. 手机解锁-参阅手机厂商相关文档 
2. 去TWRP官网搜索recovery包，再去adx上找到自己想要刷的rom
  > (1)recovery包用来覆盖官方的recovery模式，官方的recovery模式是不允许刷第三方包的，有限制
  > 
  > (2)rom是自己想要刷的rom，比如lineageOs
5. 手机先关机，按住音量-和开机键进入fastboot界面
6. fastboot flash recovery [刚才下载好的包路径]
7. 然后重启手机，重启的时候按住音量+和开机键，直到出现我们刷入的recovery页面
8. 滑动允许，同时wipe data/format card
9. 通过adb sideload的方式，把新系统刷入手机
10. 重启手机，刷机成功

*如果遇到任何问题，直接搜索对应的手机型号，查看相关教程，最好去搜索官方的教程。*

## 简单的原理就是，手机开机会有两个选择，进入正常启动模式/或者recovery模式，recovery模式可用来对正常模式的OS进行刷写，我们刷机的时候吧recovery应用程序和rom都刷了。结果就是，系统开机后，引导程序引导为“我们故意刷的recovery.img”，当这个img加载出来后，就可以随心所欲抹除数据和加载新的rom了

# 获取root权限（可选，如果只是想单纯体验刷机快感，就不需要root）
（参考链接1：https://www.xda-developers.com/how-to-install-magisk/）
（参考链接2：[https://www.xda-developers.com/how-to-install-magisk/](https://sspai.com/post/67932)）
1. 下载magisk，一定要下载官方的，打开安装后的 Magisk App，你能看到一项名为 Ramdisk 的值。请确保此项的值为「是」「True」，我们再进行下一步。
2. 从你当前刷的rom包中解压出来，有个boot.img，把这个文件传到手机上
3. 在手机上打开Magisk，卡片[Magisk]-->点击“安装”-->方式-->选择并修补一个文件-->选择刚才的boot.img-->安装
4. 生成一个[Internal Storage]/Download/magisk_patched_[random_strings].img，将这个文件通过adb传到电脑上
5. adb reboot bootloader;fastboot flash boot [上面经过patch的img];fastboot reboot;root成功
6. 判断是否root成功，看一下Magisk中的Magisk的卡片有没有显示版本号，类似25.5(25200)

> 需要注意的是，如果这里刷patch过的boot.img的时候，如果出现问题了，就进入bootloader模式，把原来的boot.img(没有patch过的原始版本)刷进去，然后重新开机。boot.img和你当前的系统版本是一一对应的所以刷机的时候要把系统rom.zip保存好，手机变砖了还能救回来。

## 大概的原理就是，让bootloader引导，执行我们修改过的boot.img文件，从而绕开系统权限检测，达到root的目的。


