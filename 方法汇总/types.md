1. syn反射。反射源多次重试，有放大效果。！！！
2. 原始syn。对方的syncookie和synproxy会造成性能下降。
3. udp反射配合udp flood
4. 原始ack。当发包速率很大的时候，主机操作系统将耗费大量的精力接收报文、判断状态，同时要主动回应RST报文，正常的数据包就可能无法得到及时的处理
5. syn与任意标记组合，如ack，fin，psh





6. dns-flood
7. 慢速连接攻击 http flood



避免持续性攻击，转而使用间歇性的攻击，10分钟～30分钟，流量大（大部分都在10G以下）。

**大多数攻击都在1Gbps~10Gbps范围内**

多种方法攻击。

UDP是最常见的形式

最常见的攻击类型仍然是 SYN 泛洪（79.7%），其次是 UDP 泛洪（9.4%）

僵尸网络 C&C; 服务器数量排名为美国（47.55%）、荷兰（22.06%）、中国（6.37%）



SYN攻击背后的主要思想是发送大量SYN数据包以消耗TCP \ IP堆栈上分配的内存。多年来，SYN攻击变得更加复杂。最新的变种是Tsunami SYN Flood Attack。这会使用TCP SYN位的大数据包使Internet管道饱和，从而导致TCP \ IP堆栈的并行损坏。

除了 SYN泛洪之外，TCP网络攻击还会针对各种攻击使用所有其他TCP，ACK泛洪，RST洪水，推送洪水，FIN洪水及其组合。只要存在腐败可能，攻击者就会尽一切努力。

垃圾洪水管道饱和

最佳组合：**随机源syn+syn反射+ack+udp+udp反射(DNS(递归查询，让dns服务商奔溃，然后就会直接导致网站解析失败，待会误伤很多其他网站)+cldap+SSDP)**+HTTP



沙特阿拉伯，荷兰和罗马尼亚