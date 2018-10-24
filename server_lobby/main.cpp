#pragma execution_character_set("utf-8")

// 加载 xxlib & uv 环境
#include "xx_uv.h"

// 加载包生成物
#include "pkg\PKG_class.h"

// 简化 assert 写法
xx::RtvChecker rc;

// 主线程内存池
xx::MemPool mp;

// 主线程 uv 环境
xx::UvLoop uv(&mp);

// 面向 server 互联
xx::UvTcpListener_w serverListener;

// 登陆服务连上来之后 peer 存在这里
xx::UvTcpPeer_w loginPeer;

// 应对 client 连入
xx::UvTcpListener_w clientListener;

// 包含一些常用函数( Cout, Kick, ToObject, SendError )
#include "pkg\helpers.h"


// 初始化监听器 for server 互连
void InitServerListener()
{
	// 创建, 设置参数, 开始监听
	rc = serverListener = uv.CreateTcpListener();
	rc = serverListener->Bind("0.0.0.0", 11111);
	rc = serverListener->Listen();

	// 接受连接时继续 bind
	serverListener->OnAccept = [](xx::UvTcpPeer_w peer)
	{
		Cout(peer->Ip(), " connected.");

		// 设置 请求 处理函数
		peer->OnReceiveRequest = [peer](uint32_t serial, xx::BBuffer& bb)
		{
			// 试着解包
			decltype(auto) o = ToObject(peer, bb);
			if (!o) return;

			// 按包 id 路由
			switch (o.GetTypeId())
			{
			case xx::TypeId_v<PKG::Server::Info>:
			{
				// 硬转为目标类型, 方便读取
				auto& p = o.As<PKG::Server::Info>();

				// 前置检查: 必须是首包( 可使用 userNumber 来做标记 )
				if (peer->userNumber)
				{
					SendError(peer, serial, -1, "lobby: is not first package. pkg:", o);
					return;
				}
				// 标记为已收到过包
				peer->userNumber = 1;

				// 按 server type 路由
				switch (p->type)
				{
				case PKG::Server::Types::Lobby:
				{
					// 如果旧连接已存在
					if (loginPeer)
					{
						// 推送告知
						SendError(loginPeer, 0, -2, "lobby: u has been replaced by other same type server.");

						// 延迟 5 秒 kick 以确保对方已收到
						Kick(loginPeer, 5);
					}

					// 存当前连接
					loginPeer = peer;

					// 发送处理成功
					peer->SendResponse(serial, PKG::Success::defaultInstance);
					return;
				}
				// ...
				default:
					SendError(peer, serial, -3, "lobby: unknown server type. pkg:", o);
					Kick(loginPeer, 5);
					return;
				}
			}
			// ...
			default:
			{
				SendError(peer, serial, -4, "lobby: recv unhandled pkg: ", o);
				Kick(loginPeer, 5);
				return;
			}
			}
		};
	};
}

// 初始化监听器 for client 连入
void InitClientListener()
{
	rc = clientListener = uv.CreateTcpListener();
	rc = clientListener->Bind("0.0.0.0", 11111);
	rc = clientListener->Listen();

	clientListener->OnAccept = [](xx::UvTcpPeer_w peer)
	{
		Cout(peer->Ip(), " connected.");

		// 启用连接超时管理( 在收到合法包时再次延时 )
		peer->DelayRelease();

		peer->OnReceiveRequest = [peer](uint32_t serial, xx::BBuffer& bb)
		{
			decltype(auto) o = ToObject(peer, bb);
			if (!o) return;

			// todo
		};
	};
}

// 初始化一些包默认实例备用
void InitPkgDefaultInstances()
{
	PKG::Success::defaultInstance.MPCreate(&mp);

	PKG::Error::defaultInstance.MPCreate(&mp);
	PKG::Error::defaultInstance->txt.MPCreate(&mp);

	// ...
}

int main(int argc, char* argv[])
{
	// 注册类型关系
	PKG::AllTypesRegister();

	// 初始化 rpc 超时管理器
	uv.InitRpcTimeoutManager();

	// 初始化 peer 超时管理器
	uv.InitPeerTimeoutManager();

	InitPkgDefaultInstances();
	InitServerListener();
	InitClientListener();

	// 进入 uv 主循环
	return uv.Run();
}
