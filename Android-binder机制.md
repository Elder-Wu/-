binder机制，其实就是Android系统实现了一个binder驱动程序，/dev/binder，打开这个文件并对其进行读写

ServiceManager
  这个类主要就是管理binder驱动的，app可以通过它注册callback，也可以通过它调用其他应用的callback

Android Service 服务端：
  向binder驱动程序注册自己的一系列callback，这些callback就是暴露给第三方的接口，通过aidl文件定义
  那么这个时候，binder驱动就拥有了一份接口引用表，等着其他客户端来调用
    举个例子，手机上有个日历app，该App向binder驱动注册了一个ScheduleList getSchedule(String date)方法。其他app就可以通过binder机制调用这个接口获取日程安排

Android Service 客户端：
  客户端在调用的时候，先打开binder驱动，向驱动程序写入参数，并且告诉binder，我要调用哪个方法
  驱动程序在接收到参数之后，调用copy_from_user，拷贝数据到内核空间
  接着驱动程序会去callback注册表中找到对应的callback，并传递参数进行调用

原理：Linux的内核空间是共享的。数据的传递和回调的执行，都在kernel层就显得很合理了，这样只需要一次数据拷贝，即client端将参数拷贝至kernel层；如果涉及到返回值，kernel层再将返回值拷贝到用户进程


> 在Android手机上
> 
> 应用A暴露startService,startActivity这样的接口给binder驱动
> 
> 应用B通过startActivity接口去打开应用
> 
>   1. 应用进程存在，就发送LAUNCH_ACTIVITY消息给Looper
>   
>   2. 应用进程不存在，就创建进程，然后发送LAUNCH_ACTIVITY消息给Looper（其他的消息也是这个思路，比如resume，pause）
>   
> 注意⚠️：Android App进程启动后，所有的事件都要发送给Looper去处理
