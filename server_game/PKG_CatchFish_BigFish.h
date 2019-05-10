// 将配置转为具体类型
PKG::CatchFish::Configs::BigFish* const& Cfg() const noexcept;

// 执行移动逻辑, 同时调用 childs Update
virtual int Update(int const& frameNumber) noexcept override;

// 碰撞检测。如果与传入子弹相撞则返回 1。同时判断 childs
virtual int HitCheck(Bullet* const& bullet) noexcept override;
