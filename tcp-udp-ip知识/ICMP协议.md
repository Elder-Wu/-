## ICMP报文格式

![](./icmp-pkg.jpg)

- 类型：占一字节，标识ICMP报文的类型，目前已定义了14种。(从类型值来看ICMP报文可以分为两大类。第一类是取值为1~127的差错报文，第2类是取值128以上的信息报文)
- 代码：占一字节，标识对应ICMP报文的代码。它与类型字段一起共同标识了ICMP报文的详细类型。



比如https://blog.csdn.net/cracker_zhou/article/details/54982815

type:8-Echo Request

code:0

回显请求（通常也叫ping），用来测试网络通不通

```python
p = IP(dst="192.168.8.102")/ICMP()
sr1(p,timeout=1)

p.show()
###[ IP ]###
  version= 4
  ihl= None
  tos= 0x0
  len= None
  id= 1
  flags=
  frag= 0
  ttl= 64
  proto= icmp
  chksum= None
  src= 192.168.8.104
  dst= 192.168.8.102
  \options\
###[ ICMP ]###
     type= echo-request
     code= 0
     chksum= None
     id= 0x0
     seq= 0x0
```

如果可以找到相应主机的话，会回复一个echo-reply包（type:0-echo reply code:0）

```python
----->开始发送包
Begin emission:
.Finished sending 1 packets.
*
Received 2 packets, got 1 answers, remaining 0 packets
----->成功收到回显包，表示可以找到主机
<IP  version=4 ihl=5 tos=0x0 len=28 id=9012 flags= frag=0 ttl=64 proto=icmp chksum=0x0 src=192.168.8.104 dst=192.168.8.104 |<ICMP  type=echo-reply code=0 chksum=0xffff id=0x0 seq=0x0 |>>
```





## 总结

ICMP是工作在IP协议的，所以不能保证可达。另外，既然是IP协议的，就不能设置某个具体的端口，只能用来判断主机是否存在，且可找到。

我在测试的时候，去ping小米手机，但是其实ping不通的。因为小米没有连接上wifi。但是虽然小米手机没有连接wifi，还是可以通过arp缓存，由ip地址匹配到mac地址。