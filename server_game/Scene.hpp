inline int Scene::InitCascade(void* const& o) noexcept {
	// 将 scene 指针刷下去保存在类里面以便于使用
	return this->BaseType::InitCascade(this);
}

inline int Scene::Update(int const&) noexcept {
	// 遍历更新. 倒序扫描, 交换删除. 如果存在内部乱序删除的情况, 则需要 名单机制 或 标记机制 在更新结束之后挨个删掉
	auto&& fs = *this->fishs;
	if (fs.len) {
		for (size_t i = fs.len - 1; i != -1; --i) {
			if (fs[i]->Update(frameNumber)) {
				fs[fs.len - 1]->indexAtContainer = (int)i;
				fs.SwapRemoveAt(i);
			}
		}
	}
	auto&& ps = *this->players;
	if (ps.len) {
		for (size_t i = ps.len - 1; i != -1; --i) {
			if (ps[i].lock()->Update(frameNumber)) {
				ps.SwapRemoveAt(i);
			}
		}
	}
	// todo: foreach ..... call Update

	++frameNumber;
	return 0;
};
