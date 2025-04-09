## 1. 网络相关命令

### 1.1. netstat

网络诊断工具，可以显示网络连接、路由表、接口统计信息、伪装连接和多播成员等信息。

选项：
- `-i`: interface接口（网卡）
- `-n`: numeric
- `-r`: route路由表
- `-a`: 显示所有连接和监听端口
- `-p`: 显示程序名和PID（需要管理员权限）
- `-t`: 显示TCP连接
- `-u`: 显示UDP连接

显示所有TCP连接及其状态（如 LISTEN, ESTABLISHED, TIME_WAIT, CLOSE_WAIT 等）：

```bash
netstat -ant
```

定期刷新网络状态（每秒刷新一次）：

```bash
watch netstat -at
```

### 1.2. ss

ss（Socket Statistics）是一个用于显示网络连接信息的工具，类似于 netstat，但功能更强大且效率更高。

语法：

```bash
ss [Options] [ FILTER ]
```

选项（Options）：

- `-t`: 显示所有TCP连接
- `-u`: 显示所有UDP连接
- `-l`：显示所有监听（listen）套接字
- `-a`: 显示所有连接及其状态
- `-s`: 显示详细信息

过滤规则（FILTER）：

```bash
FILTER := [ state STATE-FILTER ] [ EXPRESSION ]
```

状态过滤规则（STATE-FILTER）：

> 使用关键字 state 或 exclude 跟上状态标识符。
> 
> 可用的状态标识符：
> - `all` - 所有状态
> - `connected` - 除了 listening 和 closed 之外的所有状态
> - `synchronized` - 除了 syn-sent 外的所有 connected 状态
> - `bucket` - 显示以 minisockets 形式状态维护的连接，比如 time-wait 和 syn-recv .
> - `big` - 与 bucket 相反。

示例：

```bash
$ ss exclude close-wait
$ ss state established
$ ss state established exclude close-wait
```

表达式（EXPRESSION）：

> 表达式由一系列谓词和布尔操作符构成。
>
> 布尔操作符包括：`or` (或 `|` 或 `||`), `and` (或 `&` 或 `&&`), `not` (或 `!`)。
>
> 如果谓词之间没有使用布尔操作符，则默认为 `and` 操作符。
>
> 子表达式可以使用 `(` 和 `)` 包裹为一组。
>
> 支持的谓词：
>
> - {dst|src} [=] HOST
> - {dport|sport} [op] [FAMILY:]:PORT
>
>   其中，d表示destination，s表示source
>
> - dev [=|!=] DEVICE
> - fwmark [=|!=] MASK
> - cgroup [=|!=] PATH
> - autobound
> 
> 比较运算符（op）及其别名：
> - `=` `==` `eq`
> - `!=` `ne` `neq`
> - `>` `gt`
> - `<` `lt`
> - `>=` `ge` `geq`
> - `<=` `le` `leq`
> - `!` `not`
> - `|` `||` `or`
> - `&` `&&` `and`

主机表示语法：

```
[FAMILY:]ADDRESS[:PORT]
```

ADDRESS 和 PORT 的形式取决于使用的 family.
"*" 可以作为 address 或 port 的通配符。

### 1.3. lsof
含义：list open files，列出打开的文件
可以对netstat进行补充，因为其可以列出进程名
示例：
  `lsof -i TCP:daytime`

### 1.4. ifconfig
`interface` 接口名可以通过`netstat -i`查询
`-a` 输出所有已配置接口信息
示例：
  `ifconfig eth0 down/up` 关闭/启动指定显卡
[documentation](https://www.runoob.com/linux/linux-comm-ifconfig.html)

### 1.5. ping
`ping -b ip地址` 找出本地网络中众多主机IP地址。参数`ip地址`是ifconfig输出的broadcast地址
该应用利用ICMP协议回送请求和应答报文，其测试用的IP数据报为84字节

### 1.6. traceroute
该应用利用ICMP协议进行网络诊断
自行构造UDP分组来发送并读取所引发的ICMP应答

### 1.7. ssh 或 rsh

### 1.8. route
显示/控制路由表

### 1.9. tracert

### 1.10. telnet
示例：
  `telnet <目标IP地址> <端口号>`

### 1.11. hostname
查看主机名称
示例：
  `-i` 查看主机对应的IP

### 1.12. inetd
守护进程，提供`echo`, `discard`, `daytime`, `chargen`, `time`等标准服务，这些服务由`/etc/services`映射到相应的端口上。

### 1.13. nc
即 netcat，可以用来作为客户端，以测试服务器程序，可以用于测试TCP和UDP连接。
示例：
  服务端：
    ```bash
    $ rm -f /tmp/f; mkfifo /tmp/f
    $ cat /tmp/f | /bin/sh -i 2>&1 | nc -l 127.0.0.1 1234 > /tmp/f
    ```
  客户端：
    ```bash
    $ nc host.example.com 1234
    ```

### 1.14. tcpdump
抓包工具
示例：
  `tcpdump -i lo port 9877` 监视127.0.0.1:9877，其中lo是网络接口名，由ifconfig可以查看
  `tcpdump port 80 or port 8080` 监视80或8080端口
  `tcpdump -i any port 9877 -l`
输出字段说明：
  flags: Tcpflags are some combination of S (SYN), F (FIN), P (PUSH), R (RST), U (URG), W (ECN CWR), E (ECN-Echo) or `.` (ACK), or `none` if no flags are set.

### 1.15. curl
可用于测试HTTP/HTTPS连接
示例：
  `curl -v telnet://<目标IP地址>:<端口号>`

### 1.16. nmap
一个网络扫描工具，可以用于检测开放的TCP端口
示例：
  `nmap -p <端口号> <目标IP地址>`

## 2. 系统相关命令

### 2.1. sysctl
被用于在内核运行时动态地修改内核的运行参数
可用的内核参数在目录/proc/sys中
示例：
  用sysctl可以读取设置超过五百个系统变量