# 刷机主要是3个步骤
1. 手机解锁-参阅手机厂商相关文档
2. 去TWRP官网搜索recovery包，再去adx上找到自己想要刷的rom，再去下载一个magisk，其中，只需要把rom和magisk利用adb push到手机的Download文件夹下
  1)recovery包用来覆盖官方的recovery模式，官方的recovery模式是不允许刷第三方包的，有限制
  2)magisk包用来获取root权限，详情可以搜索相关原理
  3)rom是自己想要刷的rom，比如lineageOs
3. 手机先关机，按住音量-和开机键进入fastboot界面
4. fastboot flash recovery [刚才下载好的包路径]
5. 然后重启手机，重启的时候按住音量+和开机键，直到出现我们刷入的recovery页面
6. 滑动允许，同时wipe data/format card
7. 在Download文件夹下把magisk和rom都安装好
8. 重启手机，刷机成功

*如果遇到任何问题，直接搜索对应的手机型号，查看相关教程，最好去搜索官方的教程。*
