# python的协程主要用来处理I/O等待问题，比如网络请求时的I/O延时，文件描述符(socket,pipe,file)的读写，其底层逻辑是select多路复用，针对cpu密集型的python程序，没有任何帮助。

```python
import asyncio
import aiohttp

async def fetch_baidu():
    async with aiohttp.ClientSession() as session:
        async with session.get('https://www.baidu.com') as response:
            return await response.text()

async def main():
    result = await fetch_baidu()
    print(result)

asyncio.run(main())
```
-----------------------------------------------------------


# javascript的协程主要依靠Promise这个对象，我认为其最大的用途就是可以将异步代码写成同步调用。至于说异步IO的功能，我觉得不明显。
```javascript
const fetchBaidu = () => {
  return new Promise((resolve, reject) => {
    const xhr = new XMLHttpRequest();
    xhr.open('GET', 'https://www.baidu.com');
    xhr.onload = () => {
      if (xhr.status >= 200 && xhr.status < 300) {
        resolve(xhr.responseText);
      } else {
        reject(`Error: ${xhr.status}`);
      }
    };
    xhr.onerror = () => {
      reject('Error: Network Error');
    };
    xhr.send();
  });
};

const fetchAndLogBaidu = async () => {
  try {
    const response = await fetchBaidu();
    console.log(response);
  } catch (error) {
    console.error(error);
  }
};

// 调用 async 函数
fetchAndLogBaidu();
```
<font color=red size=5> javascript是单线程模型，和android的Looper机制一样，底层实现用的epoll机制。一个进程只有一条主线程用来更新UI，因此主线程不能做耗时操作，否则界面会卡死。现在看来好像带界面的都用的是单线程模型，至少android和浏览器中的javascript v8引擎是这样的</font>
<font color=gray size=5>再说一下epoll的事情。Android里面是利用了epoll的wait方法，epoll wait方法可以传0，传-1，或者传一个具体的值。0代表wait函数立即返回，-1代表wait一直监听(这个行为不会抢占cpu资源)直到监听事件(文件可写，文件可读...)的发生，一个具体的值代表timeout，即在指定时间内返回。<br></font>
<font color=#2E8B57  size=5>首先Android会创建一个Looper对象，调用loop方法，方法里面有一个for<font color=red  size=5>死循环</font>，调用MessageQueue的next()方法获取消息。巧妙的地方是，MessageQueue并没有直接返回一个Message，而是用了一套障眼法。MessageQueue中的next()方法也是一个<font color=red  size=5>死循环</font>。<br>内部代码是这样执行的：第一次循环调用epoll的wait方法，时间参数传0，代表立刻返回，（这里有点试探性的意思，主要就是判断当前messagequeue有没有东西可以拿，没有message拿，就直接wait(-1)死等；有message拿，就看能不能处理），1.message列表为空，代表没有message消息需要处理，调用wait(-1)一直等待，直到新的消息出现。<br>2.message有内容，android会取出排在列表头的第一个message，比较message的when参数和当前时间uptimeMillis，如果时间到了可以执行，把message从queue中取出，然后执行这个消息的dispatchMessage方法；如果时间没到不可以执行，就判断还需要等待多少秒才可以执行，也就是when参数减去uptime参数，那么得出的结果就是还需要等待的时间，聪明的android作者直接将这个值传入到epoll的wait方法中，调用wait(还需要等待的时间)，等到timeout结束，android就可以取出message，并执行相应的操作。Android将epoll的等待机制和MessageQueue进行了巧妙的组合，实在高明</font>
<font color=#2F2F4F size=5>Android是如何post消息的。post消息很简单，post的时候，会把每个消息的绝对时间(uptimeMillis)记录下来，插入消息队列时，根据uptimeMillis进行排序。如果当前的MessageQueue是空的，那么对应的就是wait(-1)，代表这个方法永远不会返回。所以要激活这个消息队列，只需要给epoll写入一个值，据了解，Android作者好像写入了一个w字母，记不太清了，其他没有特别的地方了</font>
<font color=black size=5>从整体上看，android并么有充分利用epoll的所有特点，仅仅是使用了wait方法。顶多就是写入一个w激活一下。</font>


# 需要使用协程的场景就是IO阻塞，即socket读取，pipe管道读取，文件读取
# 另外一个场景就是异步代码写成同步代码的形式，比如Android里面，网络请求结束后更新UI是一个异步操作，用协程会大大提高代码阅读性。javascript中也可以用await关键字，将异步代码写成同步调用。协程本质上还是单线程。

# 补充下epoll相关的知识。在浏览器和android中，点击事件会发送一个message，给事件循环（js里面叫event loop，Android里面叫looper）添加一条消息，至于这部分代码，我现在还没有事件去研究。


