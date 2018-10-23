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
xx::UvTcpListener_w listenerServer;

// 用于 lobby 互联的 net client
xx::UvTcpClient_w lobbyClient;

// 应对 client 连入
xx::UvTcpListener_w listener;

// 包含一些常用函数( Cout, Kick, ToObject, SendError )
#include "pkg\helpers.h"

// 初始化监听器 for client 连入
void InitListener()
{
	rc = listener = uv.CreateTcpListener();
	rc = listener->Bind("0.0.0.0", 10000);
	rc = listener->Listen();

	listener->OnAccept = [](xx::UvTcpPeer_w peer)
	{
		Cout(peer->Ip(), " connected.");

		// 启用连接超时管理( 在收到合法包时再次延时 )
		peer->DelayRelease();
		peer->OnTimeout = [peer]() noexcept 
		{
			Cout(peer->Ip(), " has been timeout kicked from server.");
			peer->Release();
		};

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
	InitListener();

	// 进入 uv 主循环
	return uv.Run();
}
