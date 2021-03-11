## 物理层
1. 物理设备
    - 网卡，网线，集线器，中继器，调制解调器

## 数据链路层
1. 物理设备
    - 交换机
    - 网桥
    - 网卡（MAC控制器）
2. 协议
    - 以太网
    - ARP
3. 概念
    - MAC地址：（Media Access Control Address）
    - MTU：（Maximum Transmission Unit，最大传输单元）
        - 以太网为1500字节。
        - IPv4要求最小链路MTU为68字节，因为这可以允许最大的IP头部（20字节的固定部分+最多40字节的选项）和最小的数据载荷（IPv4中片段偏移字段以8字节为单位）。
        - IPv6最小的链路MTU为1280字节


## 网络层（IP层）
1. 物理设备
    - 路由器
2. 协议
    - IPv4
    - IPv6
    - ICMP
3. 概念
    - fragmentation：分片（IPv4主机和路由器会分片，IPv6只有主机分片，路由器不会对其转发的数据报分片）
    - reassembling：重组（到达目的地之前一般不被重组）
    - path MTU：路径MTU，两个主机之间最小的MTU
    - minimum reassembly buffer size：最小重组缓冲区大小，这是IPv4或IPv6必须支持的最小数据报大小，IPv4为576字节，IPv6为1500字节

## 传输层
1. 物理设备
    - 网关（工作在传输层及以上）
2. 协议
3. 概念
    - MSS （Maximum Segment Size，最大分节大小）
        - 以太网中，使用IPv4的MSS为1460字节（1500-20字节IP头部-20字节TCP头部），IPv6的MSS为1440（1500-40字节IP头部-20字节TCP头部）
        - TCP的MSS字段为16位，也就是支持65535字节的MSS：对于IPv4足够，因为IPv4的片段位移为13位，以8字节为单位，其TCP数据量为65535-20字节TCP头；对于IPv6，没有特大载荷选项时，其最大TCP数据量为65535-20=65515，而具有特大载荷选项时，65535表示“无限”，其发送的数据报大小则由路径MTU确定。
        - MSS选项使用的目的之一，是试图避免分片。


## 应用层
1. 物理设备
    - CPU


# 参考链接
- OSI模型、设备、协议 
    https://blog.csdn.net/xw20084898/article/details/39438783
- 网卡工作在哪一层？ 
    https://blog.csdn.net/lengye7/article/details/87895146