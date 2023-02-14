### javascript是单线程模型，和android的Looper机制一样，底层实现用的epoll机制。一个进程只有一条主线程用来更新UI，因此主线程不能做耗时操作，否则界面会卡死。现在看来好像带界面的都用的是单线程模型，至少android和浏览器中的javascript v8引擎是这样的
### 再说一下epoll的事情。Android里面是利用了epoll的wait方法，epoll wait方法可以传0，传-1，或者传一个具体的值。0代表wait函数立即返回，-1代表wait一直监听(这个行为不会抢占cpu资源)直到监听事件(文件可写，文件可读...)的发生，一个具体的值代表timeout，即在指定时间内返回。
### 首先Android会创建一个Looper对象，调用loop方法，方法里面有一个for<font color=red  size=5>死循环</font>，调用MessageQueue的next()方法获取消息。巧妙的地方是，MessageQueue并没有直接返回一个Message，而是用了一套障眼法。MessageQueue中的next()方法也是一个<font color=red  size=5>死循环</font>。<br>内部代码是这样执行的：
##### 第一次循环调用epoll的wait方法，时间参数传0，代表立刻返回，（这里有点试探性的意思，主要就是判断当前messagequeue有没有东西可以拿，没有message拿，就直接wait(-1)死等；有message拿，就看能不能处理），
1. message列表为空，代表没有message消息需要处理，调用wait(-1)一直等待，直到新的消息出现。
2. message有内容，android会取出排在列表头的第一个message，比较message的when参数和当前时间uptimeMillis，如果时间到了可以执行，把message从queue中取出，然后执行这个消息的dispatchMessage方法，接着继续调用wait(0)；如果时间没到不可以执行，就判断还需要等待多少秒才可以执行，也就是when参数减去uptime参数，那么得出的结果就是还需要等待的时间，聪明的android作者直接将这个值传入到epoll的wait方法中，调用wait(还需要等待的时间)，等到timeout结束，android就可以取出message，并执行相应的操作。Android将epoll的等待机制和MessageQueue进行了巧妙的组合，实在高明
##### 通过上面的流程，android利用wait方法间接实现了消息的获取，和cpu的资源释放。简单点说就是，有消息就处理，没有消息就等待
### Android是如何post消息的。post消息很简单，post的时候，会把每个消息的绝对时间(uptimeMillis)记录下来，插入消息队列时，根据uptimeMillis进行排序。如果新的message插入到队头，就要激活这个消息队列，目的是让Looper方法更新wait的参数(-1,0,具体的值)。据了解，Android作者好像写入了一个w字母，记不太清了，其他没有特别的地方了
# 从整体上看，android并么有充分利用epoll的所有特点，仅仅是使用了wait方法。顶多就是写入一个w激活一下。发消息和处理消息是解耦的
