## C语言的指针（核心要点）

​	之前指针老是学不好，因为什么呢❓❓❓

​	因为写法问题！！！之前书上的写法都是 `int *p`形式(星号*是和参数p挨在一起)，再次强调下，这个写法很容易误导人。建议以后的教科书上不要这么写。好学生都被教坏了。

​	**写成`int* p`形式(星号*是和参数类型int挨在一起)会更好理解，指针就没那么难了。**

```c
int a = 123; //变量a就是普通的整形
int* p = &a; //变量p是一个int的指针类型，指针类型只能用来存储地址，所以将变量a的地址保存到变量p中。
printf("%d\n",*p); //打印123，同等于printf("%d",a);
printf("%p\n",p); //打印变量a的内存地址，同等于printf("%p",&a);
```





## C语言中的`*_t`结尾是什么类型的数据

​	比如`uint8_t`、`uint16_t`、`uint32_t`、`uint64_t`......这些都是整形变量的别名，这么做是为了规范和可移植性。用`_t`结尾可以理解成是通过typedef来定义的。**其实通过利用这样的别名，程序还是很好理解的👍👍👍**

`size_t`：sizeof的typedef形式



## CMakeLists.txt文件配置说明

这个文件的目的是生成makefile，然后通过make命令可以编译链接成可执行文件。

![](./cmakelist.png)

```cmake
#cmake版本号
cmake_minimum_required(VERSION 3.13)
#项目名称，后续可以通过${PEOJECT_NAME}获取到,最好设置成项目文件夹的名字。如果不设置的话，默认就是项目文件夹的名字
project(test_d)
#设置变量
set(CMAKE_C_STANDARD 11)
#生成可执行文件,参数格式(可执行文件的名字 入口文件(这里应该是一个包含main函数的c文件))
add_executable(abc main.c)#可执行文件名字叫abc，入口文件是main.c
add_executable(def test_p.c)
```





## 创建多线程

```c
pthread_t tid;
//第一个参数tid，第二个参数线程param，一般传NULL，第三个参数，方法指针，第四个参数，方法的参数列表，如果没有参数就传NULL
pthread_create(&tid, NULL, (void *) add, NULL);
pthread_join(tid,NULL);
```





## 如何创建RAW SOCKET
> 查看同目录中的c代码

编译成可执行文件：`gcc -o executable-name source-file.c`

>  **在Linux系统编译并运行上面的代码后，发现客户端会发送一个RST包给服务端，为什么？**

​	因为client发了SYN包，server发回来SYN/ACK包，你的操作系统内核先于你的程序接收到这个包，它检查内核里的socket，发现没有一个socket对应于这个包（因为raw tcp socket没有保存ip和端口等信息，所以内核不能识别这个包），所以发了一个RST包给对方，于是对方的tcp socket关闭了。

