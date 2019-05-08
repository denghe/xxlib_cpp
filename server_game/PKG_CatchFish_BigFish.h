// 直线慢速移动. 如果移到头, 就再随机一根路径重新移动. 同时调用子的 Move
virtual int Move() noexcept override;

// 碰撞检测。如果与传入子弹相撞则返回 1。同时判断 childs
virtual int HitCheck(Bullet* const& bullet) noexcept;