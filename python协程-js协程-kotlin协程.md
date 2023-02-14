# 一个超级简单的理解方式：将协程当成正常的方法调用，唯一不同的是，方法调用是将整个方法执行完后，才退出栈。而协程可以允许方法运行到一半就退栈，也允许方法随时恢复。有个特别的地方，如果协程运行的方法中含有子线程，其实这个和正常方法一样，遇到子线程就并行运行。
# 协程本质上还是单线程，现阶段协程最大的特点就是异步代码可以写成同步调用，底层原理就是协程的代码运行时会覆盖当前栈，导致CS：IP只能运行协程栈的代码，只有当协程的代码运行结束后，CS:IP指针才能重新指向主线程栈。协程栈可以包含多个子任务栈，每个子任务栈都是独立的，每个子任务栈的上下文信息保存在堆区，cpu同一时间只能运行一个任务栈，可通过await关键字交出cpu控制权切换任务栈。协程会将当前正在运行的任务栈上下文信息从堆区复制到栈区，遇到await关键字后，再把现场保存到堆区，然后从堆区拷贝另外一个任务栈执行，不断循环
# 栈顶的数据始终都是当前正在执行的任务，不可能运行其他任务，所以是单线程的。有点类似于“方法调用”
# 为什么协程可以实现运行到一半去运行其他代码
> 方法A运行到一半，协程会保存方法A的上下文环境（寄存器，局部变量，CS:IP指针），将这些信息保存到堆。那么下次从堆区复制过来后，就可以继续执行方法A
# 与ChatGPT的一段对话，解释了协程的底层实现原理

### 关于python协程，我的理解，其实程序每次在切换协程的时候，就是为新的协程开了一个栈

> 您的理解有一定的道理，但是不完全准确。

> 协程是一种轻量级线程，其执行流程可以由程序员控制。与线程不同的是，协程是在单线程中实现的，不需要切换线程，因此具有很高的效率和灵活性。在Python中，协程通常使用生成器实现，即使用yield关键字来实现协程的挂起和恢复。

> 在Python中，每个协程都是一个生成器对象。当程序运行到yield语句时，协程就会被挂起，程序执行权返回到调用协程的代码中。当程序需要再次执行协程时，调用协程的代码可以通过调用生成器的send()方法，向协程发送参数，并唤醒协程继续执行。

> 在协程的执行过程中，Python使用了一个调度器（scheduler）来管理协程的执行。调度器会记录协程的状态和执行位置，并在需要时切换不同的协程。切换协程时，Python不需要为新的协程创建一个新的栈，而是使用了类似于CPU上下文切换的机制，将当前协程的状态保存下来，然后恢复下一个协程的状态，使得程序可以从上一次挂起的位置继续执行。

> 因此，协程的切换过程并不涉及创建新的栈，而是通过保存和恢复状态来实现的。这也是协程具有高效性的原因之一。

### 那么，协程切换时，状态应该是保存在堆区的，不是保存在栈区的。保存在栈区，无法自由切换

> 您的理解是正确的，协程状态保存在堆区而不是栈区。

> 在Python中，协程状态（包括当前执行位置、局部变量等）是保存在堆区的，而不是保存在栈区。这是因为Python的协程机制是基于生成器实现的，每个生成器对象都有自己的状态和堆栈。当一个生成器对象被挂起时，其状态和堆栈都被保存到堆区，等待下一次恢复执行时再次载入。因此，协程状态的保存和恢复并不依赖于栈，而是依赖于堆区的内存管理。

> 协程切换时，Python会在堆区中保存当前协程的状态，并在需要时恢复下一个协程的状态。因为状态保存在堆区，所以Python的协程机制具有很高的灵活性，可以实现非常复杂的协程调度算法。

> 需要注意的是，在Python 3.7之前，协程状态的保存和恢复是通过调用C语言的函数来实现的，而在Python 3.7及之后的版本中，协程状态的保存和恢复则是通过C语言和汇编语言相结合的方法实现的，这也使得Python的协程机制在性能方面得到了进一步的提升。

### 为什么kotlin中的协程可以实现子线程和UI线程的切换，也是利用堆区实现的吗

> 是的，Kotlin中的协程也是利用堆区实现的，能够实现子线程和UI线程的切换。

> Kotlin中的协程基于协程库提供的 CoroutineContext 类实现。CoroutineContext 定义了协程执行时所需的上下文信息，包括协程执行所在的线程、异常处理器、以及其他自定义的上下文信息等。Kotlin中的协程库提供了一些内置的 CoroutineContext 实现，如 Dispatchers.Default、Dispatchers.IO、Dispatchers.Main 等，用于表示协程执行所在的线程。在 Android 中，Dispatchers.Main 表示协程执行所在的 UI 线程。

> Kotlin中的协程通过将协程执行所需的状态信息保存在堆区的协程上下文中，实现了线程切换。当协程需要切换到另一个线程时，它会将协程上下文传递给线程调度器，由调度器将协程的状态信息保存到堆区中，并切换到目标线程。当协程需要恢复执行时，调度器会将协程的状态信息从堆区中读取出来，恢复协程的执行。

> 因为协程的状态信息保存在堆区中，所以它可以在不同的线程中自由切换。这使得Kotlin中的协程非常适合在Android开发中使用，可以方便地实现子线程和UI线程之间的切换，避免了编写复杂的线程切换代码。

### 我的理解是，这里面应该是新建子线程了，然后当用户指定Dispatchers.IO时，kotlin会用新建的子线程去执行代码

> 不完全正确。Kotlin 中的协程库使用的是线程池，而不是每次都新建子线程。当使用 `Dispatchers.IO` 时，Kotlin 协程库会从一个预定义的线程池中获取一个空闲的线程，然后将协程分配给该线程去执行。这个线程池的大小是有限制的，所以当所有线程都在执行任务时，后续的协程任务就需要等待线程池中的某个线程空闲下来才能执行。

> 对于主线程的 `Dispatchers.Main`，它并不是新建子线程，而是利用 Android 系统中的主线程来执行协程任务。在 Android 系统中，主线程也称为 UI 线程，主要负责 UI 绘制和用户交互等操作。因此，在使用 `Dispatchers.Main` 执行协程任务时，应该尽量保持协程任务的执行时间短，以避免阻塞主线程导致 UI 卡顿的问题。

> 总的来说，Kotlin 协程的优势在于可以通过 `Dispatchers` 对象来自由切换不同的线程或线程池执行协程任务，从而避免了手动管理线程或线程池的麻烦。

### 协程最大的好处是异步代码可以写成同步调用的形式，其次才是线程调度

> ***协程的最大优势之一是可以将异步代码写成类似于同步调用的形式***，这样可以避免回调地狱和复杂的控制流程，提高代码的可读性和可维护性。在协程中，可以使用 `suspend` 关键字标记异步操作的挂起函数，并使用 `async` 或 `launch` 函数创建协程，从而让异步代码看起来像是顺序执行的代码。

> 另外，协程的另一个重要优势是线程调度的灵活性。在协程中，我们可以使用 `Dispatchers` 对象来自由切换不同的线程或线程池执行协程任务，这样就可以避免手动管理线程或线程池的麻烦，同时还可以充分利用现代多核处理器的性能优势，实现更高效的并发编程。

> 因此，协程的好处不仅仅在于可以写出简洁、可读性好的异步代码，还在于提供了一种灵活、高效的并发编程模型，可以极大地简化并发编程的复杂性。

---



# Python的协程主要用来处理I/O等待问题，比如网络请求时的I/O延时，文件描述符(socket,pipe,file)的读写，其底层逻辑是select多路复用，针对cpu密集型的Python程序，没有任何帮助。

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

print("before run")
asyncio.run(main())
print("after run")

#代码解析
#首先程序运行时有主线程栈，即两个print和一个asyncio三行代码构成的栈，本来程序应该按照正常流程执行的
#但是运行到asyncio.run(main())的时候，程序创建了一个新的栈，叫main()方法栈，这个栈会覆盖在主线程栈，导致了主线程的暂时挂起
#main方法开始执行，遇到了await关键字，进行网络IO，这是一个耗时操作，于是将cpu控制权返回给调用它的asyncio，通知asyncio，我这里遇到了耗时操作，暂时不需要占用cpu资源，你可以先去执行其他的任务，等我准备好了之后，我会用epoll通知你，你再来继续执行我的代码
#asyncio会把main栈的上下文信息全部备份到堆区，接着在堆区查找其他可以运行的“栈”，而这些堆区的“栈”，其实就是对应每个python协程中的coroutine task，如果找不到可以执行的task，那么asyncio就会进入等待状态，底层是调用了epoll wait(-1)方法，直到刚才的main栈发来可读的通知
#main栈在重新获得cpu控制权后，会将代码执行完毕，然后task执行完毕，cpu重新交给asyncio
#asyncio发现如果所有task都执行完毕了，会退出。程序会重新返回到主线程，继续执行print("after run")方法
```

```python
import asyncio
import aiohttp

async def fetch_url(session, url):
    async with session.get(url) as response:
        return await response.text()

async def main():
    urls = ['http://www.baidu.com', 'http://www.xiaomi.com', 'http://www.taobao.com']
    async with aiohttp.ClientSession() as session:
        tasks = [fetch_url(session, url) for url in urls]
        results = await asyncio.gather(*tasks)
        print(results)

if __name__ == '__main__':
    print("before")
    asyncio.run(main())
    print("after")
    
#代码解析
#程序新开了一个main协程栈，覆盖主线程栈
#main运行到await关键字，意味着要等待后面coroutine的执行完
#包含3个子任务栈，子任务栈各自执行自己的代码互不影响
#3个子任务栈运行结束后会打印出结果，main协程栈运行结束，回到主线程栈
#打印after，整个程序结束
```
---

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

# kotin中的协程主要就是用来切换线程（如在IO线程网络请求，在UI线程展示结果），最大的优势是，异步代码写成同步调用
```kotlin
GlobalScope.launch {
    val content = withContext(Dispatchers.IO) {
        URL("https://www.baidu.com").openStream().bufferedReader().readText()
    }
    withContext(Dispatchers.Main) {
        textView.text = content
    }
}
```
