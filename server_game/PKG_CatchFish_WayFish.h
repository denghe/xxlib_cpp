// 执行移动逻辑( frameRatio 控制了 Move 次数 )
virtual int Update(int const& frameNumber) noexcept override;

// 沿着 Way 做一次移动
int Move() noexcept;
