# binder机制，其实就是Android系统实现了一个binder驱动程序，/dev/binder，打开这个文件并对其进行读写

# ServiceManager
  这个类主要就是管理binder驱动的，app可以通过它注册callback，也可以通过它调用其他应用的callback

# 提供接口的app
  向binder驱动程序注册自己的一系列callback，这些callback就是暴露给第三方的接口，通过aidl文件定义
  那么这个时候，binder驱动就拥有了一份接口引用表，等着其他客户端来调用
    举个例子，手机上有个日历app，该App向binder驱动注册了一个ScheduleList getSchedule(String date)方法。其他app就可以通过binder机制调用这个接口获取日程安排

# 使用接口的app
  客户端在调用的时候，先打开binder驱动，向驱动程序写入参数，并且告诉binder，我要调用哪个方法
  驱动程序在接收到参数之后，找到对应的callback，将callback的引用返回给客户端
  客户端拿到接口的引用后，就可以直接调用了。类似dns解析，通过dns解析域名获取ip地址，而这里获取的函数指针


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
