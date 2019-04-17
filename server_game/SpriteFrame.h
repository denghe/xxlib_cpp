struct SpriteFrame : PKG::CatchFish::Configs::SpriteFrame {
	using BaseType = PKG::CatchFish::Configs::SpriteFrame;
	using BaseType::BaseType;

#ifdef CC_TARGET_PLATFORM
	cocos2d::SpriteFrame* spriteFrame = nullptr;
#endif

	virtual int InitCascade(void* const& o) noexcept override;
	~SpriteFrame();
};
using SpriteFrame_s = std::shared_ptr<SpriteFrame>;
using SpriteFrame_w = std::weak_ptr<SpriteFrame>;
