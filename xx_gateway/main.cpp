/*

网关 1.0 单个 网关 网络拓扑图如下：

	client1 <------------------------------------------------------>\      /<----> service1
	client2 <------------------------------------------------------>- 网关 -<----> service3
	client3 <------------------------------------------------------>/      \<----> service2

备注：
	网关 放置在 离内部服务较近线路畅通 的机房，优先确保 网关 与 service 连接质量




问题：
	部分国家( 例如越南 )通过上面的方式访问，网络通信质量可能依然较差( 客户端直接跨国 )


办法：
	基于 网关 1.0，将其拆分为 3 部分： 客户接入端，隧道，服务接入端


网关 2.0 网络拓扑图如下：

	client1 <---->\									                        /<----> service1
	client2 <---->- 客户接入端 =============(隧道)============== 服务接入端 -<----> service3
	client3 <---->/									                        \<----> service2

备注：
	"客户接入端" 放置在 离玩家较近线路畅通 的机房, "服务接入端" 放置在 离内部服务较近线路畅通 的机房
	"隧道" 可以是各式 vpn / 分流+汇聚 / 协议转换 / 专线 等 方案, 确保 "客户接入端" 与 "服务接入端" 之间的数据畅通


实际部署：
	1.0 与 2.0 并存. 大部分地区走 1.0 结构就够用了
	1.0 升级到 2.0, 无需客户端 & 服务器段修改代码.




问题:
	运维可能需要更灵活的路由方案，以应对变化频繁的网络环境. 同时增加隧道健壮性, 允许一定程度的容灾

办法：
	基于 2.0, 增加多组隧道并行协作方案, 具体有 2 种模式:
	1. 智能路由型
		同一时间只有其中 1 根隧道用于通信，定时通过延迟或故障判断策略来智能选择
	2. 负载均衡型
		同一时间有多个隧道用于通信，冗余发送相同数据（先到先用，后到忽略），流量 换 速度&稳定

网关 3.0 网络拓扑图如下：

	client1 <---->\            /===========(隧道1)============\            /<----> service1
	client2 <---->- 客户接入端 ============(隧道2)============= 服务接入端 -<----> service3
	client3 <---->/            \===========(隧道3)============/            \<----> service2

	实际部署：
	直接替代 2.0





扩展问题1:
	运维需要进一步直观的了解各个 网关 的工作情况，更方便的开展工作

办法：
	基于 1.0 + 3.0, 增加各种连接/攻击日志 & 健康状态汇报，把这些数据汇总，进行实时的图形化展示，并支持简单的鼠标点选操作就能实现切换线路

网络拓扑图如下：

						 /------ 网关1
	运维 <------> 总控台 ------- 网关2
						 \------ 网关3



扩展问题2:
	运维希望将 网关 反向应用于自己的运维工作，例如加速文件上传，加速浏览网页什么的

办法：
	基于 3.0, 特化一个兼容 socks5 之类代理协议的版本, 并提供 pc ios android 等客户端, 以及 linux server 一键部署












1.0 设计明细:

	3 类用例：client, gateway, service。 都是多对多

	包结构设计

	总包结构不变:
	4 byte 后续长度 + data

	data 部分 有几种结构：

	1. client <-> gateway
	serviceId + data
	serviceId 长度为 4 字节, 由 需要与 client 交互的 service 要求 gateway 产生并建立映射

	2. gateway <-> service
	clientId + data
	clientId 长度为 4 字节, 用于 service 向 client 发数据时定位到相应的 peer

	3. gateway <-> service 控制指令
	0xFFFFFFFF + data
	data 部分由服务直接处理，不转发

	控制指令简单设计：
	cmd + args...
	为简化设计方便调试，cmd 为 string 类型。解析完 string 之后再继续读预期参数.


	gateway 启动后，根据配置，首先连上一个可以拉取到 所有内服服务列表 的服务，得到 服务的 内网ip/port 列表, 并与他们分别建立长连接
	如果不考虑 不停服，动态加游戏服务器 之类的事情，也可以直接通过配置文件或者数据库读取到上述内容，并连接

	原则上 client 没有办法 直接主动 与 gateway 后面的 servers 交互( 无 address 映射 )
	gateway 就像一个交换机，保护内网设备。

	当某 service 认为，client 需要与之交互, 首先它应该通知与该 client 连接的 gateway，建立 address 到 service peer 的映射，
	拿到分配的 address 后，向 client 发送该 address

	在 client peer 下会存在一个类似 server_peers[ address ] 这样的数组结构, 用于映射转发
	如果客户端断开，则结构消失，同时 gateway 也需要向 字典中的 service peer 广播该 client 断开的消息，模拟断线

	根据上述设计，当 client 初连 gateway 时，发的包不管携带什么 address，都无法随意发送数据到内部服务
	基于此，特设定： server_peers[ address == 0 ] 为 默认映射，指向 login/lobby 之类入口服务

	address 在生成时可以从 1 往上扫空位. 发现没被占用就用上
	1 字节限制 可以同时映射不超过 256 个客户端需要同时通信的内部服务

	某些服务可能因为业务互斥，或者自己与自己互斥，需要配置互斥表。例如，玩老虎机的同时，理论上讲不太可能同时进入捕鱼游戏
	对于 address 映射请求步骤来说，受到该互斥表影响

	不排除大厅通知到具体服务出 bug 的情况，有可能出现多个原本不可以同时进入的游戏，针对同一个玩家的同一个连接，同时映射了端口
	基于互斥表，gateway 可以删除冲突的映射关系，只留下最后/最新的那个




	gateway 启动后, 等待 client 连入, 同时试着拨号到 所有对外 service
	service 启动后，等待 gateway 连入

	gateway 连入 service 后, 向 service 发送 "gatewayId" + gatewayId
	service 收到后 令 gatewayId 与 gatewayPeer 建立映射并存储，方便查找定位
	gateway 通过某渠道已获取到所有 serviceId, 与 servicePeer 建立映射并存储，方便查找定位

	必须有一个服务的 serviceId 为 0. 该服务为 IP check / 均衡 类服务
	client 连入 gateway 后，gateway 向 0 号 service 发送 "accept" + clientId + ip
	当 gateway 上的 client 链路断开时, 凡是与该 client 开启过信道 的服务 均可收到 "disconnect" + clientId

	0 号 service 收到 "accept" 后, 确定该 client 下一步连接目标 service (“登录/大厅” 啥的 )
	并向该 service 发送 "accept" + gatewayId + clientId
	目标 service 收到后定位到 gateway 并发送: "open" + clientId 以开启信道 并令该 client 产生 accept 事件
	同时目标 service 自己也产生相应的 accept 事件

	client 在拨号连上 gateway 后，等待产生 accept 事件, 得到虚拟 peer 并用之继续通信
	首次产生的 accept 事件通常对应 login/lobby 等服务. 后续 accept 根据逻辑 或 serviceId 推断

	目标 service 在与 client 交互之后，如果发现 client 需要进一步与其他 service 交互,
	则继续通知其他 service 发送含有 gatewayId + clientId 的数据包（可能夹带相关上下文）
	其他 service 收到后定位到 gateway 并发送: "open" + clientId 以开启信道 并令该 client 产生 accept 事件



	指令列表：

	client -> gateway: 无

	gateway -> service_*: "gatewayId" + gatewayId						// 信息注册
	gateway -> service_0: "accept" + clientId + ip						// client 链路连入通知
	gateway -> service_?: "disconnect" + clientId						// client 链路断开通知

	service -> gateway: "open" + clientId								// 与 client 建立信道
	service -> gateway: "close" + clientId								// 关闭 与 client 间的信道
	service -> gateway: "kick" + clientId + delayMS						// 踢人下线

	gateway -> client: "open" + serviceId								// 通知 client 建立信道
	gateway -> client: "close" + serviceId								// 通知 client 关闭信道
	gateway -> client: "redirect" + ???									// 通知 client 该 gateway 爆满, 连别处

	service_? -> service_0: "serviceId" + serviceId						// service? 连接到 service0 自报家门
	service_0 -> service_?: "accept" + gatewayId + clientId				// service0 告知 service? "有玩家上来, 与之通信"










2.0 设计明细:

	4 类用例：client, front, rear, service。 都是多对多。 gateway 一分为二: front + rear

	包结构设计 继承 1.0:

	4. front -> rear
	senderId + data. data 为原本 client <-> gateway 或 gateway <-> service 要发的东西( 数据或指令 )
	senderId 长度为 4 字节, 用于 front & rear 之间转发数据 并对应到目标, 模拟事件
	方向不同, senderId 类型不同. front -> rear: senderId = clientId. rear -> front: senderId = serviceId
	
	5. front <-> rear
	0xFFFFFFFF + data.
	控制指令
	

	工作流程继承 1.0:

	设计风格为 轻front 重rear. 即 front 比较无脑，各种不判断，直接转发. 判断点集中在 rear.
	front 监听 client 接入 并拨号到 rear. rear 监听 front 接入 并拨号到 services.
	front 那边 clients 的 网络状况, rear 没有办法立刻得知, 需要网络同步
	rear 如果需要 kick client 也需要告知 front 来做, 会有延迟, 无法马上做到. 故 rear 需要记录 clientId 的有效性, 确保后续来自该 clientId 的包被忽略


	指令列表继承 1.0:

	front -> rear: "accept": clientId + "ip"							// 转发: 有 client 连上来
	front -> rear: "disconnect": clientId								// 转发: client 断开

	rear -> front: "open" + clientId + serviceId						// 通知: client 为指定 service 开端口
	rear -> front: "close" + clientId + serviceId						// 通知: client 为指定 service 关端口
	rear -> front: "kick" + clientId + delayMS							// 通知: client 断开

*/

int main() {
	return 0;
}
