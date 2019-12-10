### 通过自己建立socket服务端来充当代理服务器

```python
import socket
import threading


def tcplink(sock, addr):
    while True:
        data = sock.recv(1024)
        if data:
            print(f"receive data from {addr[0]}:{addr[1]}")
            print(str(data, encoding='utf-8'))


server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind(("127.0.0.1", 8000))
server.listen()
print('等待客户端连接...')
while True:
    # 接受一个新连接:
    sock, addr = server.accept()
    # 创建新线程来处理TCP连接:
    threading.Thread(target=tcplink, args=(sock, addr)).start()
```

代码运行起来后，设置浏览器代理，然后随意访问一个网站，就可以看到如下输出

```http
等待客户端连接...
receive data from 127.0.0.1:64418
GET http://wuzha2.com/abc/efg?page=5 HTTP/1.1
Host: wuzha2.com
Proxy-Connection: keep-alive
Cache-Control: max-age=0
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_14_6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/78.0.3904.108 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3
Accept-Encoding: gzip, deflate
Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,th;q=0.7


(省略其他打印)......
```

***注意：此种写法不支持https代理***

假如这里代理服务器是a.b.c.d:1234
然后有个客户端地址是192.168.1.101
然后远程网站是example.com

过程解析：
客户端主机会访问example.com进行get请求，其请求报文为

```html
GET http://example.com/abc/efg?page=4 HTTP/1.1
Host: example.com
Proxy-Connection: keep-alive
Cache-Control: max-age=0
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_14_6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/78.0.3904.108 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3
Accept-Encoding: gzip, deflate
Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,th;q=0.7
```

本来这个报文应该发送给example.com对应的IP地址的，但是因为要经过代理服务器，所以被a.b.c.d:1234接收到了。这个时候a.b.c.d会当做中间人将请求发送给目标example.com，然后拿到返回值再发送给客户主机，整个流程就是这样。



#### Python TCPServer方式实现

```python
from http.server import BaseHTTPRequestHandler
from http.server import HTTPServer

class ProxyHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        # 客户端请求的全路径,如：http://example.com/abc/efg?page=4
        print(self.path)
        # 请求行
        print(self.requestline)
        # 请求头
        print(self.headers)
        # 请求的HTTP文本流对象，可读取请求体信息
        # BaseHTTPRequestHandler.rfile   
        # #self.connection.makefile('wb', self.rbufsize) self.rbufsize = 0 

        # 应答的HTTP文本流对象，可写入应答信息(response body)
        # BaseHTTPRequestHandler.wfile   
        # #self.connection.makefile('rb', self.wbufsize) self.wbufsize = -1

    def do_POST(self):
        # 同理
        pass

# 通过8000端口进行代理，然后只能支持http协议，不支持https协议 https协议需要用隧道
s = HTTPServer(("127.0.0.1", 8000), ProxyHandler)
s.serve_forever()
```



### 隧道代理

如果是隧道代理，会先发送CONNECT，通知代理服务器与目标服务器建立连接，如果连接成功，代理服务器会告诉客户端connection established.然后客户端会把请求内容原封不动发送给代理服务器
代理服务器转发，然后再获取结果，再转发给客户端



隧道代理的原理是：

> HTTP 客户端通过 CONNECT 方法请求隧道代理创建一条到达任意目的服务器和端口的 TCP 连接，并对客户端和服务器之间的后继数据进行盲转发。





![tunnel](./tunnel.jpg)



步骤如下

1. 客户端发送一个 http CONNECT 请求

```text
CONNECT baidu.com:443 HTTP/1.1
```

2. 代理收到这样的请求后，拿到目标服务器域名及端口，与目标服务端建立 TCP 连接，并响应给浏览器这样一个 HTTP 报文：

```text
HTTP/1.1 200 Connection Established
```

3. 建立完隧道以后，客户端与目标服务器照之前的方式发送请求，代理节点只是做转发功能，无从知道转发的流量具体是什么

看代码

```python
import socket
import select
from http.server import BaseHTTPRequestHandler, HTTPServer

class ProxyHandler(BaseHTTPRequestHandler):

    def send_data(self, sock, data):
        print(data)
        bytes_sent = 0
        while True:
            r = sock.send(data[bytes_sent:])
            if r < 0:
                return r
            bytes_sent += r
            if bytes_sent == len(data):
                return bytes_sent

    def handle_tcp(self, sock, remote):
        # 处理 client socket 和 remote socket 的数据流
        try:
            fdset = [sock, remote]
            while True:
                # 用 IO 多路复用 select 监听套接字是否有数据流
                r, w, e = select.select(fdset, [], [])
                if sock in r:
                    data = sock.recv(4096)
                    if len(data) <= 0:
                        break
                    result = self.send_data(remote, data)
                    if result < len(data):
                        raise Exception('failed to send all data')

                if remote in r:
                    data = remote.recv(4096)
                    if len(data) <= 0:
                        break
                    result = self.send_data(sock, data)
                    if result < len(data):
                        raise Exception('failed to send all data')
        except Exception as e:
            raise(e)
        finally:
            sock.close()
            remote.close()

    def do_CONNECT(self):

        # 解析出 host 和 port
        uri = self.path.split(":")
        host, port = uri[0], int(uri[1])
        host_ip = socket.gethostbyname(host)

        remote_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        remote_sock.connect((host_ip, port))
        # 告诉客户端 CONNECT 成功
        self.wfile.write("{protocol_version} 200 Connection Established\r\n\r\n".format(protocol_version=self.protocol_version).encode())

        # 转发请求
        self.handle_tcp(self.connection, remote_sock)

def main():
    try:
        server = HTTPServer(('', 8888), ProxyHandler)
        server.serve_forever()
    except KeyboardInterrupt:
        server.socket.close()

if __name__ == '__main__':
    main()
```



### Socks5代理原理也差不多，这里就不赘述了，以后有需要再总结



总而言之，代理的核心原理就是帮助客户端走转发



## 端口代理和端口转发的区别

端口代理：工作在socket层。在某个端口上开启了tcp或者udp服务，比如8080，然后其他程序也打开了tcp或udp服务，但是将数据发送到了代理端口上，代理端口去处理，一般都是直接处理HTTP层或者TCP层或者UDP层数据。比如vpn。

> ***补充总结：为什么vpn只能在socket层进行代理***
>
> ​	在IP（第三层）层的话，此时还没有拆包，只能拿到src和dst地址，而src=客户机(不能翻墙的机器)，dst=代理服务器，拿不到第三个地址。即使要转发，也不知道转发给谁。假如强行转发给随机的IPxxx.xxx.xxx.xxx，会出问题，请求不到想要的内容，也不敢保证一定能请求成功。
>
> ​	在TCP/UDP（第四层）层的话，通过拆包可以拿到端口号了，sport=客户机端口号，dport=代理服务器提供翻墙服务的端口号，没有其他端口号了，如果我随便提供一个转发的端口号a，不能保证a端口号上一定有TCP或者UDP服务。
>
> ​	最后数据会到HTTP层，http层拆包后拿到”GET / HTTP1.1\r\n\r\n“这样的数据，解析后能知道客户到底想要请求什么内容。代替客户机请求到内容后，再返回给客户机，这才是正常的流程。



端口转发：属于第四层转发。比如iptable，有个包在tcp层应该传给80端口，但是iptable配置了要转到8080端口，所以实际收到tcp包的应该是8080端口

ip转发：属于第三层转发。比如iptable，假如有个包要发给自己，但是经过iptables配置，会把包发给192.168.1.110或者其他人。比如arp中间人，会把路由器和目标机的请求进行转发。通过sniff便可拿到请求数据。

![](./ip-forward.jpg)

