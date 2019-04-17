struct Config : PKG::CatchFish::Configs::Config {
	using BaseType = PKG::CatchFish::Configs::Config;
	using BaseType::BaseType;

	// 用直线连接进出屏幕的两个点, 返回轨迹
	static PKG::CatchFish::Way_s MakeBeeline(std::pair<xx::Pos, xx::Pos> const& inOutPos) noexcept;

	// 用曲线连接进出屏幕的两个点( sin 曲线, x 轴穿过两点, 按x轴步进结合最小变化角度合并轨迹点, y 值 * ratio ), 返回轨迹
	static PKG::CatchFish::Way_s MakeCurve(std::pair<xx::Pos, xx::Pos> const& inOutPos, float const& xStep, xx::Pos const& ratio) noexcept;

	// 预生成一些东西( ways, ... )
	virtual int InitCascade(void* const& o = nullptr) noexcept override;
};
using Config_s = std::shared_ptr<Config>;
