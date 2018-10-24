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

// 用于 lobby 互联的 net client
xx::UvTcpClient_w lobbyClient;

// 用于 db 互联的 net client
xx::UvTcpClient_w dbClient;

// 应对 client 连入
xx::UvTcpListener_w listener;

// 缓存用户名, 密码的多主键字典. key1: int(id), key2: string(username)
xx::DictEx<PKG::DB::Account_p, int, xx::String_p> players(&mp);

// 包含一些常用函数( Cout, Kick, ToObject, SendError )
#include "pkg\helpers.h"

// 初始化 lobbyClient dbClient
void InitXxxxClients()
{
	rc = lobbyClient = uv.CreateTcpClient();
	lobbyClient->OnConnect = [](int status)
	{
		// 没连上
		if (status) return;

		// 连上之后自报家门
		lobbyClient->SendRequestEx(PKG::Server::Info::defaultInstance, [](xx::Object_p& pkg)
		{
			// 超时或收到异常包: 断开以便重连( 如果没释放且联通状态 )
			if ((!pkg || !pkg.Is<PKG::Success>()) && lobbyClient && lobbyClient->Alive())
			{
				lobbyClient->Disconnect(false);
			}
		});
	};

	rc = dbClient = uv.CreateTcpClient();
	dbClient->OnConnect = [](int status)
	{
		if (status) return;
		dbClient->SendRequestEx(PKG::Server::Info::defaultInstance, [](xx::Object_p& pkg)
		{
			if ((!pkg || !pkg.Is<PKG::Success>()) && dbClient && dbClient->Alive())
			{
				dbClient->Disconnect(false);
			}
		});
	};

	dbClient->OnReceivePackage = [](xx::BBuffer& bb) 
	{
		// todo: 密码变更通知
	};

	// 创建一个 timer. 每 0.2 秒检测一次. 如果网络断开就重连
	uv.CreateTimer(0, 200, []
	{
		if (lobbyClient && lobbyClient->Disconnected())
		{
			// 超时时间 1 秒
			lobbyClient->ConnectEx("192.168.1.254", 10001, 1000);
		}
		if (dbClient && dbClient->Disconnected())
		{
			dbClient->ConnectEx("192.168.1.254", 10002, 1000);
		}
	});
}

// 处理登陆请求( 实现代码在下面 )
void HandleAuth(xx::UvTcpPeer_w const& peer, uint32_t const& serial, PKG::Client_Login::Auth_p &req);

// 初始化监听器 for client 连入
void InitListener()
{
	rc = listener = uv.CreateTcpListener();
	rc = listener->Bind("0.0.0.0", 10000);
	rc = listener->Listen();

	listener->OnAccept = [](xx::UvTcpPeer_w peer)
	{
		Cout(peer->Ip(), " connected.");

		// 启用连接超时管理. 如果没有使用 TimeoutReset 续命, 连接将被 Release.
		peer->DelayRelease();

		// DelayRelease 函数里面已经设置了 OnTimeout = Release. 这里为了附加输出打印.
		peer->OnTimeout = [peer]() noexcept
		{
			Cout(peer->Ip(), " has been timeout kicked from server.");
			peer->Release();
		};

		// 绑定请求应答事件代码
		peer->OnReceiveRequest = [peer](uint32_t serial, xx::BBuffer& bb)
		{
			// 试着反序列化 bb 的数据为 object.
			decltype(auto) o = ToObject(peer, bb);

			// 反序列化失败: o 为空值. 
			if (!o)
			{
				// 释放连接并立即返回( 注意捕获列表会因为 Release 了 lambda 宿主而失效 )
				peer->Release();
				return;
			}

			switch (o.GetTypeId())
			{
			case xx::TypeId_v< PKG::Client_Login::Auth>:
			{
				HandleAuth(peer, serial, o.As<PKG::Client_Login::Auth>());
				break;
			}
			default:
			{
				Cout(peer->Ip(), " receive unhandled request.");
				break;
			}
			}
		};
	};
}

// 处理登陆请求
void HandleAuth(xx::UvTcpPeer_w const& peer, uint32_t const& serial, PKG::Client_Login::Auth_p &req)
{
	// 登录计数
	++peer->userNumber;

	// todo: 

	// peer 续命以等待 db 的回应
	peer->TimeoutReset();

	// 检查协议号: 对不上就 T
	if (!req->pkgMD5 || !req->pkgMD5->Equals(PKG::PkgGenMd5::value))
	{
		peer->Release();
		return;
	}

	// 粗略检查用户名合法性: 不可空或0长. 否则就 T
	if (!req->username || !req->username->dataLen)
	{
		peer->Release();
		return;
	}

	// 粗略检查密码合法性: 不可空. 可0长. 否则就 T
	if (!req->password)
	{
		peer->Release();
		return;
	}

	SendError(peer, serial, -1, "not impl");

	//// 试着在缓存中定位
	//int idx = players.Find<1>(req->username);

	//// 如果找到, 直接校验密码
	//if (idx != -1)
	//{
	//	// 密码不对
	//	if (!players.ValueAt(idx)->password->Equals(req->password))
	//	{
	//		
	//	}
	//}

	//// 检查数据库连没连上
	//if (!dbClient->Alive())
	//{
	//	SendError(peer, serial, -1, "dbClient disconnected.");
	//	return;
	//}

	//// 构造请求包
	//decltype(auto) pkg = mp.MPCreatePtr<PKG::Login_DB::GetAccount>();
	
	// todo: 向 dbClient 发 Auth 请求?


	// 登录成功
	// peer->userNumber = 0;   // 重置登录请求计数
}

// 初始化一些内容不变化的包默认实例备用
void InitPkgDefaultInstances()
{
	PKG::Success::defaultInstance.MPCreate(&mp);

	PKG::Error::defaultInstance.MPCreate(&mp);
	PKG::Error::defaultInstance->txt.MPCreate(&mp);

	PKG::Server::Info::defaultInstance.MPCreate(&mp);
	PKG::Server::Info::defaultInstance->type = PKG::Server::Types::Login;

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
