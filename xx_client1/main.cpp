#include <xx_uv_ext.h>

struct CorBase {
	virtual ~CorBase() {};
	virtual bool Update() = 0;	// return false: dispose
};

struct Ctx {
	xx::Uv uv;
	std::shared_ptr<xx::UvToGatewayDialer<xx::UvSerialBBufferSimulatePeer>> dialer;
	xx::UvSerialBBufferSimulatePeer_s simPeer;
	xx::BBuffer bb;
	Ctx() {
		xx::MakeTo(dialer, uv);
		dialer->onAcceptSimulatePeer = [&](xx::UvSerialBBufferSimulatePeer_s p) {
			simPeer = std::move(p);
			simPeer->onReceivePush = [&](xx::BBuffer& bb)->int {
				xx::CoutN("recv push ", bb);
				return 0;
			};
		};
		bb.WriteRoot(xx::Make<xx::BBuffer>());
	}

	inline bool PeerAlive() {
		return dialer && dialer->PeerAlive() >= 0;
	}

	inline bool SimPeerAlive() {
		return simPeer && !simPeer->Disposed();
	}

	std::vector<std::unique_ptr<CorBase>> cors;
	inline void Run() {
		while (cors.size()) {
			uv.Run(UV_RUN_ONCE);	// UV_RUN_NOWAIT
			std::ptrdiff_t idxBegin = cors.size() - 1;
			for (auto i = idxBegin; i >= 0; --i) {
				if (!cors[i]->Update()) {
					if (i < idxBegin) {
						std::swap(cors[i], cors[idxBegin]);
					}
					cors.resize(cors.size() - 1);
				}
			}
			//uv.Run(UV_RUN_ONCE);
			//usleep(100000);
		}
	}
};

struct Cor1 : CorBase {
	Ctx& ctx;
	Cor1(Ctx& ctx) : ctx(ctx) {}
	inline static void Create(Ctx& ctx) {
		ctx.cors.emplace_back(std::make_unique<Cor1>(ctx));
	}

	inline virtual bool Update() override {
		lineNumber = UpdateCore();
		return lineNumber != 0;
	}

	int lineNumber = 0;
	int UpdateCore();
};

struct Cor2 : CorBase {
	Ctx& ctx;
	Cor2(Ctx& ctx) : ctx(ctx) {}
	inline static void Create(Ctx& ctx) {
		ctx.cors.emplace_back(std::make_unique<Cor2>(ctx));
	}

	virtual bool Update() override {
		lineNumber = UpdateCore();
		return lineNumber != 0;
	}

	bool callbacked = false;
	int r = 0;

	int lineNumber = 0;
	int UpdateCore() {
		COR_BEGIN;

		callbacked = false;
		r = ctx.simPeer->SendRequest(ctx.bb, [this](xx::BBuffer* const& data)->int {
			if (data) {
				xx::CoutN("recv request ", *data);
			}
			else {
				xx::CoutN("recv request timeout.");
			}
			callbacked = true;
			return 0;
			}, 0);
		assert(!r);

		while (!callbacked) {
			COR_YIELD;
		}
		xx::CoutN("callbacked.");

		ctx.dialer->PeerDispose();

		COR_END;
	}
};

inline int Cor1::UpdateCore() {
	COR_BEGIN;

LabDial:
	COR_YIELD;
	xx::CoutN("dial...");
	ctx.dialer->PeerDispose();

	ctx.dialer->Dial("127.0.0.1", 12345);
	while (ctx.dialer->Busy()) {
		COR_YIELD;
	}

	xx::CoutN("step 2...");
	while (true) {
		COR_YIELD;
		if (!ctx.PeerAlive()) goto LabDial;

		if (ctx.SimPeerAlive()) {
			Cor2::Create(ctx);
			break;
		}
	}

	while (ctx.PeerAlive()) {
		COR_YIELD;
	}
	//goto LabDial;

	COR_END;
}

int main() {
	xx::IgnoreSignal();
	Ctx c;
	Cor1::Create(c);
	c.Run();
	return 0;
}
