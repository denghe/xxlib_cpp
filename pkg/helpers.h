#pragma once

// 减少控制台输出打字量( 基于全局 mp )
template<typename...Args>
void Cout(Args const&...args)
{
	xx::String s(&mp);
	s.Append(args...);
	std::cout << s << std::endl;
}

// 踢 peer 下线
void Kick(xx::UvTcpPeer_w const& peer, int delaySec = 0)
{
	if (!peer) return;
	if (delaySec)
	{
		// 清掉所有回调, 直接 release
		peer->ClearHandlers();
		Cout(peer->Ip(), " has been kicked from server.");
		peer->Release();
	}
	else
	{
		// 清掉所有回调, 延迟几秒 release
		peer->DelayRelease(delaySec, true);
		peer->OnDispose = [peer] 
		{
			Cout(peer->Ip(), " has been delay kicked from server.");
		};
	}
}


// 收到的数据试读出 obj. 如果失败, 直接 kick
xx::Object_p ToObject(xx::UvTcpPeer_w const& peer, xx::BBuffer& bb)
{
	xx::Object_p o;
	if (bb.ReadRoot(o) || !o)
	{
		Cout("recv: ", o);
		Kick(peer);
	}
	Cout("recv: ", o);
	return o;
}


// 快捷构造错误包数据并发送回应/推送( serial == 0 则为推送 )
template<typename...Args>
void SendError(xx::UvTcpPeer_w const& peer, uint32_t const& serial, Args const&...args)
{
	if (!peer) return;
	auto& pkg = PKG::Error::defaultInstance;
	pkg->id = -1;
	pkg->txt->Clear();
	pkg->txt->Append(args...);
	if (serial)
	{
		peer->SendResponse(serial, pkg);
	}
	else
	{
		peer->Send(pkg);
	}
}
