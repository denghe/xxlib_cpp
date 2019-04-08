
PKG_PkgGenMd5_Value = 'aca390891ccb0e557f0936eb8d9a69c6'

--[[
座位列表
]]
PKG_CatchFish_Sits = {
    --[[
    左下
    ]]
    LeftBottom = 0,
    --[[
    右下
    ]]
    RightBottom = 1,
    --[[
    右上
    ]]
    RightTop = 2,
    --[[
    左上
    ]]
    LeftTop = 3
}
--[[
通用返回
]]
PKG_Generic_Success = {
    typeName = "PKG_Generic_Success",
    typeId = 3,
    Create = function()
        local o = {}
        o.__proto = PKG_Generic_Success
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        return o
    end,
    FromBBuffer = function( bb, o )
    end,
    ToBBuffer = function( bb, o )
    end
}
BBuffer.Register( PKG_Generic_Success )
--[[
通用错误返回
]]
PKG_Generic_Error = {
    typeName = "PKG_Generic_Error",
    typeId = 4,
    Create = function()
        local o = {}
        o.__proto = PKG_Generic_Error
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        o.number = 0 -- Int64
        o.message = null -- String
        return o
    end,
    FromBBuffer = function( bb, o )
        o.number = bb:ReadInt64()
        o.message = bb:ReadObject()
    end,
    ToBBuffer = function( bb, o )
        bb:WriteInt64( o.number )
        bb:WriteObject( o.message )
    end
}
BBuffer.Register( PKG_Generic_Error )
--[[
心跳保持兼延迟测试 -- 请求
]]
PKG_Generic_Ping = {
    typeName = "PKG_Generic_Ping",
    typeId = 5,
    Create = function()
        local o = {}
        o.__proto = PKG_Generic_Ping
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        o.ticks = 0 -- Int64
        return o
    end,
    FromBBuffer = function( bb, o )
        o.ticks = bb:ReadInt64()
    end,
    ToBBuffer = function( bb, o )
        bb:WriteInt64( o.ticks )
    end
}
BBuffer.Register( PKG_Generic_Ping )
--[[
心跳保持兼延迟测试 -- 回应
]]
PKG_Generic_Pong = {
    typeName = "PKG_Generic_Pong",
    typeId = 6,
    Create = function()
        local o = {}
        o.__proto = PKG_Generic_Pong
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        o.ticks = 0 -- Int64
        return o
    end,
    FromBBuffer = function( bb, o )
        o.ticks = bb:ReadInt64()
    end,
    ToBBuffer = function( bb, o )
        bb:WriteInt64( o.ticks )
    end
}
BBuffer.Register( PKG_Generic_Pong )
--[[
申请进入游戏 成功
]]
PKG_CatchFish_Client_EnterSuccess = {
    typeName = "PKG_CatchFish_Client_EnterSuccess",
    typeId = 66,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Client_EnterSuccess
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        完整的游戏场景
        ]]
        o.scene = null -- PKG_CatchFish_Scene
        --[[
        玩家强引用容器
        ]]
        o.players = null -- List_PKG_CatchFish_Player_
        --[[
        指向当前玩家
        ]]
        o.self = MakeWeak() -- Weak_PKG_CatchFish_Player
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadObject = bb.ReadObject
        o.scene = ReadObject( bb )
        o.players = ReadObject( bb )
        o.self = MakeWeak( ReadObject( bb ) )
    end,
    ToBBuffer = function( bb, o )
        local WriteObject = bb.WriteObject
        WriteObject( bb, o.scene )
        WriteObject( bb, o.players )
        WriteObject( bb, o.self.Lock() )
    end
}
BBuffer.Register( PKG_CatchFish_Client_EnterSuccess )
--[[
场景
]]
PKG_CatchFish_Scene = {
    typeName = "PKG_CatchFish_Scene",
    typeId = 8,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Scene
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        游戏id
        ]]
        o.gameId = 0 -- Int32
        --[[
        级别id
        ]]
        o.levelId = 0 -- Int32
        --[[
        房间id
        ]]
        o.roomId = 0 -- Int32
        --[[
        准入金
        ]]
        o.minMoney = 0 -- Double
        --[[
        最低炮注( coin )( 针对普通炮台 )
        ]]
        o.minBet = 0 -- Int64
        --[[
        最高炮注( coin )( 针对普通炮台 )
        ]]
        o.maxBet = 0 -- Int64
        --[[
        进出游戏时 money 自动兑换成 coin 要 乘除 的系数
        ]]
        o.exchangeCoinRatio = 0 -- Int32
        --[[
        帧编号, 每帧 + 1. 用于同步
        ]]
        o.frameNumber = 0 -- Int32
        --[[
        本地鱼生成专用随机数发生器
        ]]
        o.rnd = null -- _xx_Random
        --[[
        自增id ( 从 1 开始, 用于填充 本地鱼 id )
        ]]
        o.autoIncId = 0 -- Int32
        --[[
        所有活鱼 ( 乱序 )
        ]]
        o.fishs = null -- List_PKG_CatchFish_Fish_
        --[[
        所有已创建非活鱼 ( 乱序 )
        ]]
        o.items = null -- List_PKG_CatchFish_Item_
        --[[
        所有鱼预约生成 ( 乱序 )
        ]]
        o.borns = null -- List_PKG_CatchFish_FishBorn_
        --[[
        当前关卡. endFrameNumber 到达时切换到下一关( clone from cfg.stages[(stage.id + 1) % cfg.stages.len] 并修正 各种 frameNumber )
        ]]
        o.stage = null -- PKG_CatchFish_Stages_Stage
        --[[
        空闲座位下标( 初始时填入 Sits.LeftBottom RightBottom LeftTop RightTop )
        ]]
        o.freeSits = null -- List_PKG_CatchFish_Sits_
        --[[
        所有玩家
        ]]
        o.players = null -- List_Weak_PKG_CatchFish_Player_
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadInt32 = bb.ReadInt32
        local ReadInt64 = bb.ReadInt64
        local ReadObject = bb.ReadObject
        o.gameId = ReadInt32( bb )
        o.levelId = ReadInt32( bb )
        o.roomId = ReadInt32( bb )
        o.minMoney = bb:ReadDouble()
        o.minBet = ReadInt64( bb )
        o.maxBet = ReadInt64( bb )
        o.exchangeCoinRatio = ReadInt32( bb )
        o.frameNumber = ReadInt32( bb )
        o.rnd = ReadObject( bb )
        o.autoIncId = ReadInt32( bb )
        o.fishs = ReadObject( bb )
        o.items = ReadObject( bb )
        o.borns = ReadObject( bb )
        o.stage = ReadObject( bb )
        o.freeSits = ReadObject( bb )
        o.players = ReadObject( bb )
    end,
    ToBBuffer = function( bb, o )
        local WriteInt32 = bb.WriteInt32
        local WriteInt64 = bb.WriteInt64
        local WriteObject = bb.WriteObject
        WriteInt32( bb, o.gameId )
        WriteInt32( bb, o.levelId )
        WriteInt32( bb, o.roomId )
        bb:WriteDouble( o.minMoney )
        WriteInt64( bb, o.minBet )
        WriteInt64( bb, o.maxBet )
        WriteInt32( bb, o.exchangeCoinRatio )
        WriteInt32( bb, o.frameNumber )
        WriteObject( bb, o.rnd )
        WriteInt32( bb, o.autoIncId )
        WriteObject( bb, o.fishs )
        WriteObject( bb, o.items )
        WriteObject( bb, o.borns )
        WriteObject( bb, o.stage )
        WriteObject( bb, o.freeSits )
        WriteObject( bb, o.players )
    end
}
BBuffer.Register( PKG_CatchFish_Scene )
List_PKG_CatchFish_Player_ = {
    typeName = "List_PKG_CatchFish_Player_",
    typeId = 67,
    Create = function()
        local o = {}
        o.__proto = List_PKG_CatchFish_Player_
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end

        return o
    end,
    FromBBuffer = function( bb, o )
		local len = bb:ReadUInt32()
        local f = BBuffer.ReadObject
		for i = 1, len do
			o[ i ] = f( bb )
		end
    end,
    ToBBuffer = function( bb, o )
        local len = #o
		bb:WriteUInt32( len )
        local f = BBuffer.WriteObject
        for i = 1, len do
			f( bb, o[ i ] )
		end
    end
}
BBuffer.Register( List_PKG_CatchFish_Player_ )
--[[
玩家 ( 存在于服务 players 容器. 被 Scene.players 弱引用 )
]]
PKG_CatchFish_Player = {
    typeName = "PKG_CatchFish_Player",
    typeId = 22,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Player
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        账号id. 用于定位玩家 ( 填充自 db )
        ]]
        o.id = 0 -- Int32
        --[[
        昵称 用于客户端显示 ( 填充自 db )
        ]]
        o.nickname = null -- String
        --[[
        头像id 用于客户端显示 ( 填充自 db )
        ]]
        o.avatar_id = 0 -- Int32
        --[[
        破产标识 ( 每帧检测一次总资产是否为 0, 是就标记之. 总资产包括 coin, 已爆出的 weapons, 已获得的附加炮台, 飞行中的 bullets )
        ]]
        o.noMoney = false -- Boolean
        --[[
        剩余金币值( 不代表玩家总资产 ). 当玩家进入到游戏时, 该值填充 money * exchangeCoinRatio. 玩家退出时, 做除法还原为 money.
        ]]
        o.coin = 0 -- Int64
        --[[
        座位
        ]]
        o.sit = 0 -- PKG_CatchFish_Sits
        --[[
        自动锁定状态
        ]]
        o.autoLock = false -- Boolean
        --[[
        自动开火状态
        ]]
        o.autoFire = false -- Boolean
        --[[
        锁定瞄准的鱼
        ]]
        o.aimFish = MakeWeak() -- Weak_PKG_CatchFish_Fish
        --[[
        自增id ( 从 1 开始, 用于填充 炮台, 子弹 id )
        ]]
        o.autoIncId = 0 -- Int32
        --[[
        炮台堆栈 ( 例如: 常规炮 打到 钻头, 钻头飞向玩家变为 钻头炮, 覆盖在常规炮上 )
        ]]
        o.cannons = null -- List_PKG_CatchFish_Cannon_
        --[[
        武器集合 ( 被打死的特殊鱼转为武器对象, 飞向玩家, 变炮消失前都在这里 )
        ]]
        o.weapons = null -- List_PKG_CatchFish_Weapon_
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadInt32 = bb.ReadInt32
        local ReadObject = bb.ReadObject
        local ReadBoolean = bb.ReadBoolean
        o.id = ReadInt32( bb )
        o.nickname = ReadObject( bb )
        o.avatar_id = ReadInt32( bb )
        o.noMoney = ReadBoolean( bb )
        o.coin = bb:ReadInt64()
        o.sit = ReadInt32( bb )
        o.autoLock = ReadBoolean( bb )
        o.autoFire = ReadBoolean( bb )
        o.aimFish = MakeWeak( ReadObject( bb ) )
        o.autoIncId = ReadInt32( bb )
        o.cannons = ReadObject( bb )
        o.weapons = ReadObject( bb )
    end,
    ToBBuffer = function( bb, o )
        local WriteInt32 = bb.WriteInt32
        local WriteObject = bb.WriteObject
        local WriteBoolean = bb.WriteBoolean
        WriteInt32( bb, o.id )
        WriteObject( bb, o.nickname )
        WriteInt32( bb, o.avatar_id )
        WriteBoolean( bb, o.noMoney )
        bb:WriteInt64( o.coin )
        WriteInt32( bb, o.sit )
        WriteBoolean( bb, o.autoLock )
        WriteBoolean( bb, o.autoFire )
        WriteObject( bb, o.aimFish.Lock() )
        WriteInt32( bb, o.autoIncId )
        WriteObject( bb, o.cannons )
        WriteObject( bb, o.weapons )
    end
}
BBuffer.Register( PKG_CatchFish_Player )
--[[
帧事件同步包
]]
PKG_CatchFish_Client_FrameEvents = {
    typeName = "PKG_CatchFish_Client_FrameEvents",
    typeId = 68,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Client_FrameEvents
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        帧编号
        ]]
        o.frameNumber = 0 -- Int32
        --[[
        帧事件集合
        ]]
        o.events = null -- List_PKG_CatchFish_Events_Event_
        return o
    end,
    FromBBuffer = function( bb, o )
        o.frameNumber = bb:ReadInt32()
        o.events = bb:ReadObject()
    end,
    ToBBuffer = function( bb, o )
        bb:WriteInt32( o.frameNumber )
        bb:WriteObject( o.events )
    end
}
BBuffer.Register( PKG_CatchFish_Client_FrameEvents )
List_PKG_CatchFish_Events_Event_ = {
    typeName = "List_PKG_CatchFish_Events_Event_",
    typeId = 69,
    Create = function()
        local o = {}
        o.__proto = List_PKG_CatchFish_Events_Event_
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end

        return o
    end,
    FromBBuffer = function( bb, o )
		local len = bb:ReadUInt32()
        local f = BBuffer.ReadObject
		for i = 1, len do
			o[ i ] = f( bb )
		end
    end,
    ToBBuffer = function( bb, o )
        local len = #o
		bb:WriteUInt32( len )
        local f = BBuffer.WriteObject
        for i = 1, len do
			f( bb, o[ i ] )
		end
    end
}
BBuffer.Register( List_PKG_CatchFish_Events_Event_ )
--[[
事件基类
]]
PKG_CatchFish_Events_Event = {
    typeName = "PKG_CatchFish_Events_Event",
    typeId = 28,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Events_Event
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        相关玩家id
        ]]
        o.id = 0 -- Int32
        return o
    end,
    FromBBuffer = function( bb, o )
        o.id = bb:ReadInt32()
    end,
    ToBBuffer = function( bb, o )
        bb:WriteInt32( o.id )
    end
}
BBuffer.Register( PKG_CatchFish_Events_Event )
--[[
申请进入游戏. 成功返回 EnterSuccess. 失败直接被 T
]]
PKG_Client_CatchFish_Enter = {
    typeName = "PKG_Client_CatchFish_Enter",
    typeId = 70,
    Create = function()
        local o = {}
        o.__proto = PKG_Client_CatchFish_Enter
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        return o
    end,
    FromBBuffer = function( bb, o )
    end,
    ToBBuffer = function( bb, o )
    end
}
BBuffer.Register( PKG_Client_CatchFish_Enter )
--[[
开火
]]
PKG_Client_CatchFish_Shoot = {
    typeName = "PKG_Client_CatchFish_Shoot",
    typeId = 71,
    Create = function()
        local o = {}
        o.__proto = PKG_Client_CatchFish_Shoot
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        o.frameNumber = 0 -- Int32
        o.cannonId = 0 -- Int32
        o.bulletId = 0 -- Int32
        o.pos = null -- _xx_Pos
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadInt32 = bb.ReadInt32
        o.frameNumber = ReadInt32( bb )
        o.cannonId = ReadInt32( bb )
        o.bulletId = ReadInt32( bb )
        o.pos = bb:ReadObject()
    end,
    ToBBuffer = function( bb, o )
        local WriteInt32 = bb.WriteInt32
        WriteInt32( bb, o.frameNumber )
        WriteInt32( bb, o.cannonId )
        WriteInt32( bb, o.bulletId )
        bb:WriteObject( o.pos )
    end
}
BBuffer.Register( PKG_Client_CatchFish_Shoot )
--[[
碰撞检测
]]
PKG_Client_CatchFish_Hit = {
    typeName = "PKG_Client_CatchFish_Hit",
    typeId = 72,
    Create = function()
        local o = {}
        o.__proto = PKG_Client_CatchFish_Hit
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        o.cannonId = 0 -- Int32
        o.bulletId = 0 -- Int32
        o.fishId = 0 -- Int32
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadInt32 = bb.ReadInt32
        o.cannonId = ReadInt32( bb )
        o.bulletId = ReadInt32( bb )
        o.fishId = ReadInt32( bb )
    end,
    ToBBuffer = function( bb, o )
        local WriteInt32 = bb.WriteInt32
        WriteInt32( bb, o.cannonId )
        WriteInt32( bb, o.bulletId )
        WriteInt32( bb, o.fishId )
    end
}
BBuffer.Register( PKG_Client_CatchFish_Hit )
_xx_Random = {
    typeName = "_xx_Random",
    typeId = 9,
    Create = function()
        local o = {}
        o.__proto = _xx_Random
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        return o
    end,
    FromBBuffer = function( bb, o )
    end,
    ToBBuffer = function( bb, o )
    end
}
BBuffer.Register( _xx_Random )
List_PKG_CatchFish_Fish_ = {
    typeName = "List_PKG_CatchFish_Fish_",
    typeId = 10,
    Create = function()
        local o = {}
        o.__proto = List_PKG_CatchFish_Fish_
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end

        return o
    end,
    FromBBuffer = function( bb, o )
		local len = bb:ReadUInt32()
        local f = BBuffer.ReadObject
		for i = 1, len do
			o[ i ] = f( bb )
		end
    end,
    ToBBuffer = function( bb, o )
        local len = #o
		bb:WriteUInt32( len )
        local f = BBuffer.WriteObject
        for i = 1, len do
			f( bb, o[ i ] )
		end
    end
}
BBuffer.Register( List_PKG_CatchFish_Fish_ )
--[[
鱼基类 ( 下列属性适合大多数鱼, 不一定适合部分 boss )
]]
PKG_CatchFish_Fish = {
    typeName = "PKG_CatchFish_Fish",
    typeId = 11,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Fish
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        配置id
        ]]
        o.cfgId = 0 -- Int32
        --[[
        币值 / 倍率
        ]]
        o.coin = 0 -- Int64
        --[[
        移动速度系数 ( 默认为 1 )
        ]]
        o.speedScale = 0 -- Single
        --[[
        运行时缩放比例( 通常为 1 )
        ]]
        o.scale = 0 -- Single
        --[[
        移动轨迹. 动态生成, 不引用自 cfg. 同步时被复制. 如果该值为空, 则启用 wayIndex ( 常见于非直线鱼 )
        ]]
        o.way = null -- PKG_CatchFish_Way
        --[[
        移动轨迹 于 cfg.ways 的下标. 启用优先级低于 way
        ]]
        o.wayIndex = 0 -- Int32
        --[[
        当前轨迹点下标
        ]]
        o.wayPointIndex = 0 -- Int32
        --[[
        当前轨迹点上的已前进距离
        ]]
        o.wayPointDistance = 0 -- Single
        --[[
        当前帧下标( 每帧循环累加 )
        ]]
        o.spriteFrameIndex = 0 -- Int32
        --[[
        帧比值, 平时为 1, 如果为 0 则表示鱼不动( 比如实现冰冻效果 ), 帧图也不更新. 如果大于 1, 则需要在 1 帧内多次驱动该鱼( 比如实现快速离场的效果 )
        ]]
        o.frameRatio = 0 -- Int32
        --[[
        是否为在鱼线上倒着移动( 默认否 )
        ]]
        o.reverse = false -- Boolean
        setmetatable( o, PKG_CatchFish_MoveItem.Create() )
        return o
    end,
    FromBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.FromBBuffer( bb, p )
        local ReadInt32 = bb.ReadInt32
        local ReadSingle = bb.ReadSingle
        o.cfgId = ReadInt32( bb )
        o.coin = bb:ReadInt64()
        o.speedScale = ReadSingle( bb )
        o.scale = ReadSingle( bb )
        o.way = bb:ReadObject()
        o.wayIndex = ReadInt32( bb )
        o.wayPointIndex = ReadInt32( bb )
        o.wayPointDistance = ReadSingle( bb )
        o.spriteFrameIndex = ReadInt32( bb )
        o.frameRatio = ReadInt32( bb )
        o.reverse = bb:ReadBoolean()
    end,
    ToBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.ToBBuffer( bb, p )
        local WriteInt32 = bb.WriteInt32
        local WriteSingle = bb.WriteSingle
        WriteInt32( bb, o.cfgId )
        bb:WriteInt64( o.coin )
        WriteSingle( bb, o.speedScale )
        WriteSingle( bb, o.scale )
        bb:WriteObject( o.way )
        WriteInt32( bb, o.wayIndex )
        WriteInt32( bb, o.wayPointIndex )
        WriteSingle( bb, o.wayPointDistance )
        WriteInt32( bb, o.spriteFrameIndex )
        WriteInt32( bb, o.frameRatio )
        bb:WriteBoolean( o.reverse )
    end
}
BBuffer.Register( PKG_CatchFish_Fish )
List_PKG_CatchFish_Item_ = {
    typeName = "List_PKG_CatchFish_Item_",
    typeId = 12,
    Create = function()
        local o = {}
        o.__proto = List_PKG_CatchFish_Item_
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end

        return o
    end,
    FromBBuffer = function( bb, o )
		local len = bb:ReadUInt32()
        local f = BBuffer.ReadObject
		for i = 1, len do
			o[ i ] = f( bb )
		end
    end,
    ToBBuffer = function( bb, o )
        local len = #o
		bb:WriteUInt32( len )
        local f = BBuffer.WriteObject
        for i = 1, len do
			f( bb, o[ i ] )
		end
    end
}
BBuffer.Register( List_PKG_CatchFish_Item_ )
--[[
场景元素的共通基类
]]
PKG_CatchFish_Item = {
    typeName = "PKG_CatchFish_Item",
    typeId = 13,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Item
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        自增id ( 服务器实时下发的id为负 )
        ]]
        o.id = 0 -- Int32
        --[[
        位于容器时的下标 ( 用于快速交换删除 )
        ]]
        o.indexAtContainer = 0 -- Int32
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadInt32 = bb.ReadInt32
        o.id = ReadInt32( bb )
        o.indexAtContainer = ReadInt32( bb )
    end,
    ToBBuffer = function( bb, o )
        local WriteInt32 = bb.WriteInt32
        WriteInt32( bb, o.id )
        WriteInt32( bb, o.indexAtContainer )
    end
}
BBuffer.Register( PKG_CatchFish_Item )
List_PKG_CatchFish_FishBorn_ = {
    typeName = "List_PKG_CatchFish_FishBorn_",
    typeId = 14,
    Create = function()
        local o = {}
        o.__proto = List_PKG_CatchFish_FishBorn_
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end

        return o
    end,
    FromBBuffer = function( bb, o )
		local len = bb:ReadUInt32()
        local f = BBuffer.ReadObject
		for i = 1, len do
			o[ i ] = f( bb )
		end
    end,
    ToBBuffer = function( bb, o )
        local len = #o
		bb:WriteUInt32( len )
        local f = BBuffer.WriteObject
        for i = 1, len do
			f( bb, o[ i ] )
		end
    end
}
BBuffer.Register( List_PKG_CatchFish_FishBorn_ )
--[[
预约出鱼
]]
PKG_CatchFish_FishBorn = {
    typeName = "PKG_CatchFish_FishBorn",
    typeId = 15,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_FishBorn
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        当 currentFrameNumber == beginFrameNumber 时，将 fish 放入 Scene.fishs 并自杀
        ]]
        o.fish = null -- PKG_CatchFish_Fish
        setmetatable( o, PKG_CatchFish_Timer.Create() )
        return o
    end,
    FromBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.FromBBuffer( bb, p )
        o.fish = bb:ReadObject()
    end,
    ToBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.ToBBuffer( bb, p )
        bb:WriteObject( o.fish )
    end
}
BBuffer.Register( PKG_CatchFish_FishBorn )
--[[
游戏关卡. 位于 Stage.timers 中的 timer, 使用 stageFrameNumber 来计算时间. 可弱引用 Stage 本身. 需要可以干净序列化
]]
PKG_CatchFish_Stages_Stage = {
    typeName = "PKG_CatchFish_Stages_Stage",
    typeId = 16,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Stages_Stage
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        同下标
        ]]
        o.id = 0 -- Int32
        --[[
        关卡帧编号( clone 后需清0. 每帧 +1 )
        ]]
        o.stageFrameNumber = 0 -- Int32
        --[[
        当前阶段结束时间点( clone 后需修正 )
        ]]
        o.endFrameNumber = 0 -- Int32
        --[[
        关卡元素集合
        ]]
        o.timers = null -- List_PKG_CatchFish_Timer_
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadInt32 = bb.ReadInt32
        o.id = ReadInt32( bb )
        o.stageFrameNumber = ReadInt32( bb )
        o.endFrameNumber = ReadInt32( bb )
        o.timers = bb:ReadObject()
    end,
    ToBBuffer = function( bb, o )
        local WriteInt32 = bb.WriteInt32
        WriteInt32( bb, o.id )
        WriteInt32( bb, o.stageFrameNumber )
        WriteInt32( bb, o.endFrameNumber )
        bb:WriteObject( o.timers )
    end
}
BBuffer.Register( PKG_CatchFish_Stages_Stage )
List_PKG_CatchFish_Sits_ = {
    typeName = "List_PKG_CatchFish_Sits_",
    typeId = 17,
    Create = function()
        local o = {}
        o.__proto = List_PKG_CatchFish_Sits_
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end

        return o
    end,
    FromBBuffer = function( bb, o )
		local len = bb:ReadUInt32()
        local f = BBuffer.ReadInt32
		for i = 1, len do
			o[ i ] = f( bb )
		end
    end,
    ToBBuffer = function( bb, o )
        local len = #o
		bb:WriteUInt32( len )
        local f = BBuffer.WriteInt32
        for i = 1, len do
			f( bb, o[ i ] )
		end
    end
}
BBuffer.Register( List_PKG_CatchFish_Sits_ )
List_Weak_PKG_CatchFish_Player_ = {
    typeName = "List_Weak_PKG_CatchFish_Player_",
    typeId = 18,
    Create = function()
        local o = {}
        o.__proto = List_Weak_PKG_CatchFish_Player_
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end

        return o
    end,
    FromBBuffer = function( bb, o )
		local len = bb:ReadUInt32()
        local f = BBuffer.ReadObject
		for i = 1, len do
			o[ i ] = MakeWeak( f( bb ) )
		end
    end,
    ToBBuffer = function( bb, o )
        local len = #o
		bb:WriteUInt32( len )
        local f = BBuffer.WriteObject
        for i = 1, len do
			f( bb, o[ i ].Lock() )
		end
    end
}
BBuffer.Register( List_Weak_PKG_CatchFish_Player_ )
List_PKG_CatchFish_Cannon_ = {
    typeName = "List_PKG_CatchFish_Cannon_",
    typeId = 23,
    Create = function()
        local o = {}
        o.__proto = List_PKG_CatchFish_Cannon_
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end

        return o
    end,
    FromBBuffer = function( bb, o )
		local len = bb:ReadUInt32()
        local f = BBuffer.ReadObject
		for i = 1, len do
			o[ i ] = f( bb )
		end
    end,
    ToBBuffer = function( bb, o )
        local len = #o
		bb:WriteUInt32( len )
        local f = BBuffer.WriteObject
        for i = 1, len do
			f( bb, o[ i ] )
		end
    end
}
BBuffer.Register( List_PKG_CatchFish_Cannon_ )
--[[
炮台基类. 下列属性适合大多数炮
]]
PKG_CatchFish_Cannon = {
    typeName = "PKG_CatchFish_Cannon",
    typeId = 19,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Cannon
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        自增id ( 服务器实时下发的id为负 )
        ]]
        o.id = 0 -- Int32
        --[[
        配置id
        ]]
        o.cfgId = 0 -- Int32
        --[[
        币值 / 倍率 ( 初始填充自 db. 玩家可调整数值. 范围限制为 Scene.minBet ~ maxBet )
        ]]
        o.coin = 0 -- Int64
        --[[
        炮管角度 ( 每次发射时都填充一下 )
        ]]
        o.angle = 0 -- Single
        --[[
        所有子弹
        ]]
        o.bullets = null -- List_PKG_CatchFish_Bullet_
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadInt32 = bb.ReadInt32
        o.id = ReadInt32( bb )
        o.cfgId = ReadInt32( bb )
        o.coin = bb:ReadInt64()
        o.angle = bb:ReadSingle()
        o.bullets = bb:ReadObject()
    end,
    ToBBuffer = function( bb, o )
        local WriteInt32 = bb.WriteInt32
        WriteInt32( bb, o.id )
        WriteInt32( bb, o.cfgId )
        bb:WriteInt64( o.coin )
        bb:WriteSingle( o.angle )
        bb:WriteObject( o.bullets )
    end
}
BBuffer.Register( PKG_CatchFish_Cannon )
List_PKG_CatchFish_Weapon_ = {
    typeName = "List_PKG_CatchFish_Weapon_",
    typeId = 24,
    Create = function()
        local o = {}
        o.__proto = List_PKG_CatchFish_Weapon_
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end

        return o
    end,
    FromBBuffer = function( bb, o )
		local len = bb:ReadUInt32()
        local f = BBuffer.ReadObject
		for i = 1, len do
			o[ i ] = f( bb )
		end
    end,
    ToBBuffer = function( bb, o )
        local len = #o
		bb:WriteUInt32( len )
        local f = BBuffer.WriteObject
        for i = 1, len do
			f( bb, o[ i ] )
		end
    end
}
BBuffer.Register( List_PKG_CatchFish_Weapon_ )
--[[
武器基类 ( 有一些特殊鱼死后会变做 某种武器 / 炮台，死时有个滞空展示时间，被用于解决网络同步延迟。所有端应该在展示时间结束前收到该预约。展示完成后武器将飞向炮台变为附加炮台 )
]]
PKG_CatchFish_Weapon = {
    typeName = "PKG_CatchFish_Weapon",
    typeId = 25,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Weapon
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        配置id
        ]]
        o.cfgId = 0 -- Int32
        --[[
        开始飞行的帧编号
        ]]
        o.flyFrameNumber = 0 -- Int32
        setmetatable( o, PKG_CatchFish_MoveItem.Create() )
        return o
    end,
    FromBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.FromBBuffer( bb, p )
        local ReadInt32 = bb.ReadInt32
        o.cfgId = ReadInt32( bb )
        o.flyFrameNumber = ReadInt32( bb )
    end,
    ToBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.ToBBuffer( bb, p )
        local WriteInt32 = bb.WriteInt32
        WriteInt32( bb, o.cfgId )
        WriteInt32( bb, o.flyFrameNumber )
    end
}
BBuffer.Register( PKG_CatchFish_Weapon )
List_PKG_CatchFish_Bullet_ = {
    typeName = "List_PKG_CatchFish_Bullet_",
    typeId = 20,
    Create = function()
        local o = {}
        o.__proto = List_PKG_CatchFish_Bullet_
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end

        return o
    end,
    FromBBuffer = function( bb, o )
		local len = bb:ReadUInt32()
        local f = BBuffer.ReadObject
		for i = 1, len do
			o[ i ] = f( bb )
		end
    end,
    ToBBuffer = function( bb, o )
        local len = #o
		bb:WriteUInt32( len )
        local f = BBuffer.WriteObject
        for i = 1, len do
			f( bb, o[ i ] )
		end
    end
}
BBuffer.Register( List_PKG_CatchFish_Bullet_ )
--[[
子弹基类
]]
PKG_CatchFish_Bullet = {
    typeName = "PKG_CatchFish_Bullet",
    typeId = 21,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Bullet
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        每帧的直线移动坐标增量( 60fps )
        ]]
        o.moveInc = null -- _xx_Pos
        --[[
        金币 / 倍率( 记录炮台开火时的 Bet 值 )
        ]]
        o.coin = 0 -- Int64
        setmetatable( o, PKG_CatchFish_MoveItem.Create() )
        return o
    end,
    FromBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.FromBBuffer( bb, p )
        o.moveInc = bb:ReadObject()
        o.coin = bb:ReadInt64()
    end,
    ToBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.ToBBuffer( bb, p )
        bb:WriteObject( o.moveInc )
        bb:WriteInt64( o.coin )
    end
}
BBuffer.Register( PKG_CatchFish_Bullet )
--[[
子弹 & 鱼 & 武器 的基类
]]
PKG_CatchFish_MoveItem = {
    typeName = "PKG_CatchFish_MoveItem",
    typeId = 26,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_MoveItem
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        中心点坐标
        ]]
        o.pos = null -- _xx_Pos
        --[[
        当前角度
        ]]
        o.angle = 0 -- Single
        setmetatable( o, PKG_CatchFish_Item.Create() )
        return o
    end,
    FromBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.FromBBuffer( bb, p )
        o.pos = bb:ReadObject()
        o.angle = bb:ReadSingle()
    end,
    ToBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.ToBBuffer( bb, p )
        bb:WriteObject( o.pos )
        bb:WriteSingle( o.angle )
    end
}
BBuffer.Register( PKG_CatchFish_MoveItem )
--[[
轨迹. 预约下发安全, 将复制轨迹完整数据
]]
PKG_CatchFish_Way = {
    typeName = "PKG_CatchFish_Way",
    typeId = 49,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Way
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        轨迹点集合
        ]]
        o.points = null -- List_PKG_CatchFish_WayPoint_
        --[[
        总距离长度( sum( points[all].distance ). 如果非循环线, 不包含最后一个点的距离值. )
        ]]
        o.distance = 0 -- Single
        --[[
        是否循环( 即移动到最后一个点之后又到第 1 个点, 永远走不完
        ]]
        o.loop = false -- Boolean
        return o
    end,
    FromBBuffer = function( bb, o )
        o.points = bb:ReadObject()
        o.distance = bb:ReadSingle()
        o.loop = bb:ReadBoolean()
    end,
    ToBBuffer = function( bb, o )
        bb:WriteObject( o.points )
        bb:WriteSingle( o.distance )
        bb:WriteBoolean( o.loop )
    end
}
BBuffer.Register( PKG_CatchFish_Way )
--[[
定时器基类
]]
PKG_CatchFish_Timer = {
    typeName = "PKG_CatchFish_Timer",
    typeId = 27,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Timer
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        开始 / 生效帧编号
        ]]
        o.beginFrameNumber = 0 -- Int32
        return o
    end,
    FromBBuffer = function( bb, o )
        o.beginFrameNumber = bb:ReadInt32()
    end,
    ToBBuffer = function( bb, o )
        bb:WriteInt32( o.beginFrameNumber )
    end
}
BBuffer.Register( PKG_CatchFish_Timer )
List_PKG_CatchFish_WayPoint_ = {
    typeName = "List_PKG_CatchFish_WayPoint_",
    typeId = 64,
    Create = function()
        local o = {}
        o.__proto = List_PKG_CatchFish_WayPoint_
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end

        return o
    end,
    FromBBuffer = function( bb, o )
		local len = bb:ReadUInt32()
        local f = BBuffer.ReadWayPoint
		for i = 1, len do
			o[ i ] = f( bb )
		end
    end,
    ToBBuffer = function( bb, o )
        local len = #o
		bb:WriteUInt32( len )
        local f = BBuffer.WriteWayPoint
        for i = 1, len do
			f( bb, o[ i ] )
		end
    end
}
BBuffer.Register( List_PKG_CatchFish_WayPoint_ )
--[[
通知: 玩家进入. 大部分字段从 Player 类复制. 添加了部分初始数值, 可还原出玩家类实例.
]]
PKG_CatchFish_Events_Enter = {
    typeName = "PKG_CatchFish_Events_Enter",
    typeId = 29,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Events_Enter
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        昵称
        ]]
        o.nickname = null -- String
        --[[
        头像id
        ]]
        o.avatar_id = 0 -- Int32
        --[[
        破产标识
        ]]
        o.noMoney = false -- Boolean
        --[[
        剩余金币值
        ]]
        o.coin = 0 -- Int64
        --[[
        座位
        ]]
        o.sit = 0 -- PKG_CatchFish_Sits
        --[[
        炮台配置id
        ]]
        o.cannonCfgId = 0 -- Int32
        --[[
        炮台币值
        ]]
        o.cannonCoin = 0 -- Int64
        setmetatable( o, PKG_CatchFish_Events_Event.Create() )
        return o
    end,
    FromBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.FromBBuffer( bb, p )
        local ReadInt32 = bb.ReadInt32
        local ReadInt64 = bb.ReadInt64
        o.nickname = bb:ReadObject()
        o.avatar_id = ReadInt32( bb )
        o.noMoney = bb:ReadBoolean()
        o.coin = ReadInt64( bb )
        o.sit = ReadInt32( bb )
        o.cannonCfgId = ReadInt32( bb )
        o.cannonCoin = ReadInt64( bb )
    end,
    ToBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.ToBBuffer( bb, p )
        local WriteInt32 = bb.WriteInt32
        local WriteInt64 = bb.WriteInt64
        bb:WriteObject( o.nickname )
        WriteInt32( bb, o.avatar_id )
        bb:WriteBoolean( o.noMoney )
        WriteInt64( bb, o.coin )
        WriteInt32( bb, o.sit )
        WriteInt32( bb, o.cannonCfgId )
        WriteInt64( bb, o.cannonCoin )
    end
}
BBuffer.Register( PKG_CatchFish_Events_Enter )
--[[
通知: 玩家离开
]]
PKG_CatchFish_Events_Leave = {
    typeName = "PKG_CatchFish_Events_Leave",
    typeId = 30,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Events_Leave
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        setmetatable( o, PKG_CatchFish_Events_Event.Create() )
        return o
    end,
    FromBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.FromBBuffer( bb, p )
    end,
    ToBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.ToBBuffer( bb, p )
    end
}
BBuffer.Register( PKG_CatchFish_Events_Leave )
--[[
通知: 玩家破产
]]
PKG_CatchFish_Events_NoMoney = {
    typeName = "PKG_CatchFish_Events_NoMoney",
    typeId = 31,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Events_NoMoney
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        setmetatable( o, PKG_CatchFish_Events_Event.Create() )
        return o
    end,
    FromBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.FromBBuffer( bb, p )
    end,
    ToBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.ToBBuffer( bb, p )
    end
}
BBuffer.Register( PKG_CatchFish_Events_NoMoney )
--[[
通知: 退钱( 常见于子弹打空 )
]]
PKG_CatchFish_Events_Refund = {
    typeName = "PKG_CatchFish_Events_Refund",
    typeId = 32,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Events_Refund
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        币值
        ]]
        o.coin = 0 -- Int64
        setmetatable( o, PKG_CatchFish_Events_Event.Create() )
        return o
    end,
    FromBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.FromBBuffer( bb, p )
        o.coin = bb:ReadInt64()
    end,
    ToBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.ToBBuffer( bb, p )
        bb:WriteInt64( o.coin )
    end
}
BBuffer.Register( PKG_CatchFish_Events_Refund )
--[[
通知: 鱼被打死
]]
PKG_CatchFish_Events_FishDead = {
    typeName = "PKG_CatchFish_Events_FishDead",
    typeId = 33,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Events_FishDead
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        鱼id
        ]]
        o.fishId = 0 -- Int32
        --[[
        子弹id
        ]]
        o.bulletId = 0 -- Int32
        --[[
        金币所得( fish.coin * bullet.coin 或 server 计算牵连鱼之后的综合结果 )
        ]]
        o.coin = 0 -- Int64
        --[[
        牵连死的鱼
        ]]
        o.fishDeads = null -- List_PKG_CatchFish_Events_FishDead_
        setmetatable( o, PKG_CatchFish_Events_Event.Create() )
        return o
    end,
    FromBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.FromBBuffer( bb, p )
        local ReadInt32 = bb.ReadInt32
        o.fishId = ReadInt32( bb )
        o.bulletId = ReadInt32( bb )
        o.coin = bb:ReadInt64()
        o.fishDeads = bb:ReadObject()
    end,
    ToBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.ToBBuffer( bb, p )
        local WriteInt32 = bb.WriteInt32
        WriteInt32( bb, o.fishId )
        WriteInt32( bb, o.bulletId )
        bb:WriteInt64( o.coin )
        bb:WriteObject( o.fishDeads )
    end
}
BBuffer.Register( PKG_CatchFish_Events_FishDead )
List_PKG_CatchFish_Events_FishDead_ = {
    typeName = "List_PKG_CatchFish_Events_FishDead_",
    typeId = 34,
    Create = function()
        local o = {}
        o.__proto = List_PKG_CatchFish_Events_FishDead_
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end

        return o
    end,
    FromBBuffer = function( bb, o )
		local len = bb:ReadUInt32()
        local f = BBuffer.ReadObject
		for i = 1, len do
			o[ i ] = f( bb )
		end
    end,
    ToBBuffer = function( bb, o )
        local len = #o
		bb:WriteUInt32( len )
        local f = BBuffer.WriteObject
        for i = 1, len do
			f( bb, o[ i ] )
		end
    end
}
BBuffer.Register( List_PKG_CatchFish_Events_FishDead_ )
--[[
通知: 下发已生效 Weapon, 需要判断 flyFrameNumber, 放入 player.weapon 队列
]]
PKG_CatchFish_Events_PushWeapon = {
    typeName = "PKG_CatchFish_Events_PushWeapon",
    typeId = 35,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Events_PushWeapon
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        已于 server 端构造好的, 无牵挂的, 能干净下发的实例
        ]]
        o.weapon = null -- PKG_CatchFish_Weapon
        setmetatable( o, PKG_CatchFish_Events_Event.Create() )
        return o
    end,
    FromBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.FromBBuffer( bb, p )
        o.weapon = bb:ReadObject()
    end,
    ToBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.ToBBuffer( bb, p )
        bb:WriteObject( o.weapon )
    end
}
BBuffer.Register( PKG_CatchFish_Events_PushWeapon )
--[[
预约: 出鱼( 需判定 beginFrameNumber ), 放入 scene.timers 队列
]]
PKG_CatchFish_Events_PushFish = {
    typeName = "PKG_CatchFish_Events_PushFish",
    typeId = 36,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Events_PushFish
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        已于 server 端构造好的, 无牵挂的, 能干净下发的实例
        ]]
        o.born = null -- PKG_CatchFish_FishBorn
        setmetatable( o, PKG_CatchFish_Events_Event.Create() )
        return o
    end,
    FromBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.FromBBuffer( bb, p )
        o.born = bb:ReadObject()
    end,
    ToBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.ToBBuffer( bb, p )
        bb:WriteObject( o.born )
    end
}
BBuffer.Register( PKG_CatchFish_Events_PushFish )
--[[
转发: 开启开火锁定
]]
PKG_CatchFish_Events_OpenAutoLock = {
    typeName = "PKG_CatchFish_Events_OpenAutoLock",
    typeId = 37,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Events_OpenAutoLock
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        setmetatable( o, PKG_CatchFish_Events_Event.Create() )
        return o
    end,
    FromBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.FromBBuffer( bb, p )
    end,
    ToBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.ToBBuffer( bb, p )
    end
}
BBuffer.Register( PKG_CatchFish_Events_OpenAutoLock )
--[[
转发: 玩家锁定后瞄准某鱼
]]
PKG_CatchFish_Events_Aim = {
    typeName = "PKG_CatchFish_Events_Aim",
    typeId = 38,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Events_Aim
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        被瞄准的鱼id
        ]]
        o.fishId = 0 -- Int32
        setmetatable( o, PKG_CatchFish_Events_Event.Create() )
        return o
    end,
    FromBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.FromBBuffer( bb, p )
        o.fishId = bb:ReadInt32()
    end,
    ToBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.ToBBuffer( bb, p )
        bb:WriteInt32( o.fishId )
    end
}
BBuffer.Register( PKG_CatchFish_Events_Aim )
--[[
转发: 玩家开火解除锁定
]]
PKG_CatchFish_Events_CloseAutoLock = {
    typeName = "PKG_CatchFish_Events_CloseAutoLock",
    typeId = 39,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Events_CloseAutoLock
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        setmetatable( o, PKG_CatchFish_Events_Event.Create() )
        return o
    end,
    FromBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.FromBBuffer( bb, p )
    end,
    ToBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.ToBBuffer( bb, p )
    end
}
BBuffer.Register( PKG_CatchFish_Events_CloseAutoLock )
--[[
转发: 玩家自动开火
]]
PKG_CatchFish_Events_OpenAutoFire = {
    typeName = "PKG_CatchFish_Events_OpenAutoFire",
    typeId = 40,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Events_OpenAutoFire
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        setmetatable( o, PKG_CatchFish_Events_Event.Create() )
        return o
    end,
    FromBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.FromBBuffer( bb, p )
    end,
    ToBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.ToBBuffer( bb, p )
    end
}
BBuffer.Register( PKG_CatchFish_Events_OpenAutoFire )
--[[
转发: 玩家解除自动开火
]]
PKG_CatchFish_Events_CloseAutoFire = {
    typeName = "PKG_CatchFish_Events_CloseAutoFire",
    typeId = 41,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Events_CloseAutoFire
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        setmetatable( o, PKG_CatchFish_Events_Event.Create() )
        return o
    end,
    FromBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.FromBBuffer( bb, p )
    end,
    ToBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.ToBBuffer( bb, p )
    end
}
BBuffer.Register( PKG_CatchFish_Events_CloseAutoFire )
--[[
转发: 发子弹( 单次 ). 非特殊子弹, 只可能是 cannons[0] 原始炮台发射
]]
PKG_CatchFish_Events_Fire = {
    typeName = "PKG_CatchFish_Events_Fire",
    typeId = 42,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Events_Fire
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        起始帧编号 ( 来自客户端 )
        ]]
        o.frameNumber = 0 -- Int32
        --[[
        子弹id
        ]]
        o.bulletId = 0 -- Int32
        --[[
        子弹的发射目标坐标
        ]]
        o.tarPos = null -- _xx_Pos
        --[[
        子弹的发射角度
        ]]
        o.tarAngle = 0 -- Single
        --[[
        币值 / 倍率
        ]]
        o.coin = 0 -- Int64
        setmetatable( o, PKG_CatchFish_Events_Event.Create() )
        return o
    end,
    FromBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.FromBBuffer( bb, p )
        local ReadInt32 = bb.ReadInt32
        o.frameNumber = ReadInt32( bb )
        o.bulletId = ReadInt32( bb )
        o.tarPos = bb:ReadObject()
        o.tarAngle = bb:ReadSingle()
        o.coin = bb:ReadInt64()
    end,
    ToBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.ToBBuffer( bb, p )
        local WriteInt32 = bb.WriteInt32
        WriteInt32( bb, o.frameNumber )
        WriteInt32( bb, o.bulletId )
        bb:WriteObject( o.tarPos )
        bb:WriteSingle( o.tarAngle )
        bb:WriteInt64( o.coin )
    end
}
BBuffer.Register( PKG_CatchFish_Events_Fire )
--[[
转发: 切换炮台
]]
PKG_CatchFish_Events_CannonSwitch = {
    typeName = "PKG_CatchFish_Events_CannonSwitch",
    typeId = 43,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Events_CannonSwitch
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        炮台配置id
        ]]
        o.cfgId = 0 -- Int32
        setmetatable( o, PKG_CatchFish_Events_Event.Create() )
        return o
    end,
    FromBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.FromBBuffer( bb, p )
        o.cfgId = bb:ReadInt32()
    end,
    ToBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.ToBBuffer( bb, p )
        bb:WriteInt32( o.cfgId )
    end
}
BBuffer.Register( PKG_CatchFish_Events_CannonSwitch )
--[[
转发: 切换炮台倍率
]]
PKG_CatchFish_Events_CannonCoinChange = {
    typeName = "PKG_CatchFish_Events_CannonCoinChange",
    typeId = 44,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Events_CannonCoinChange
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        币值 / 倍率
        ]]
        o.coin = 0 -- Int64
        setmetatable( o, PKG_CatchFish_Events_Event.Create() )
        return o
    end,
    FromBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.FromBBuffer( bb, p )
        o.coin = bb:ReadInt64()
    end,
    ToBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.ToBBuffer( bb, p )
        bb:WriteInt64( o.coin )
    end
}
BBuffer.Register( PKG_CatchFish_Events_CannonCoinChange )
List_PKG_CatchFish_Timer_ = {
    typeName = "List_PKG_CatchFish_Timer_",
    typeId = 45,
    Create = function()
        local o = {}
        o.__proto = List_PKG_CatchFish_Timer_
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end

        return o
    end,
    FromBBuffer = function( bb, o )
		local len = bb:ReadUInt32()
        local f = BBuffer.ReadObject
		for i = 1, len do
			o[ i ] = f( bb )
		end
    end,
    ToBBuffer = function( bb, o )
        local len = #o
		bb:WriteUInt32( len )
        local f = BBuffer.WriteObject
        for i = 1, len do
			f( bb, o[ i ] )
		end
    end
}
BBuffer.Register( List_PKG_CatchFish_Timer_ )
--[[
服务器本地脚本( 关卡元素 )
]]
PKG_CatchFish_Stages_Script = {
    typeName = "PKG_CatchFish_Stages_Script",
    typeId = 46,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Stages_Script
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        o.lineNumber = 0 -- Int32
        setmetatable( o, PKG_CatchFish_Timer.Create() )
        return o
    end,
    FromBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.FromBBuffer( bb, p )
        o.lineNumber = bb:ReadInt32()
    end,
    ToBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.ToBBuffer( bb, p )
        bb:WriteInt32( o.lineNumber )
    end
}
BBuffer.Register( PKG_CatchFish_Stages_Script )
--[[
游戏配置主体
]]
PKG_CatchFish_Configs_Config = {
    typeName = "PKG_CatchFish_Configs_Config",
    typeId = 47,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Configs_Config
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        所有预生成轨迹( 轨迹创建后先填充到这, 再与具体的鱼 bind, 以达到重用的目的 )
        ]]
        o.ways = null -- List_PKG_CatchFish_Way_
        --[[
        所有鱼的配置信息
        ]]
        o.fishs = null -- List_PKG_CatchFish_Configs_Fish_
        --[[
        所有炮台的配置信息
        ]]
        o.cannons = null -- List_PKG_CatchFish_Configs_Cannon_
        --[[
        所有武器的配置信息
        ]]
        o.weapons = null -- List_PKG_CatchFish_Configs_Weapon_
        --[[
        循环关卡数据( Scene 初次创建时，从 stages[0] clone. 可以在内存中 cache 序列化后的 binary )
        ]]
        o.stages = null -- List_PKG_CatchFish_Stages_Stage_
        --[[
        基于设计尺寸为 1280 x 720, 屏中心为 0,0 点的 4 个玩家炮台的坐标( 0: 左下  1: 右下    2: 右上  3: 左上 )
        ]]
        o.sitPositons = null -- List__xx_Pos_
        --[[
        锁定点击范围 ( 增加容错, 不必点的太精确. 点击作用是 枚举该范围内出现的鱼, 找出并选取 touchRank 最大值那个 )
        ]]
        o.aimTouchRadius = 0 -- Single
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadObject = bb.ReadObject
        o.ways = ReadObject( bb )
        o.fishs = ReadObject( bb )
        o.cannons = ReadObject( bb )
        o.weapons = ReadObject( bb )
        o.stages = ReadObject( bb )
        o.sitPositons = ReadObject( bb )
        o.aimTouchRadius = bb:ReadSingle()
    end,
    ToBBuffer = function( bb, o )
        local WriteObject = bb.WriteObject
        WriteObject( bb, o.ways )
        WriteObject( bb, o.fishs )
        WriteObject( bb, o.cannons )
        WriteObject( bb, o.weapons )
        WriteObject( bb, o.stages )
        WriteObject( bb, o.sitPositons )
        bb:WriteSingle( o.aimTouchRadius )
    end
}
BBuffer.Register( PKG_CatchFish_Configs_Config )
List_PKG_CatchFish_Way_ = {
    typeName = "List_PKG_CatchFish_Way_",
    typeId = 48,
    Create = function()
        local o = {}
        o.__proto = List_PKG_CatchFish_Way_
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end

        return o
    end,
    FromBBuffer = function( bb, o )
		local len = bb:ReadUInt32()
        local f = BBuffer.ReadObject
		for i = 1, len do
			o[ i ] = f( bb )
		end
    end,
    ToBBuffer = function( bb, o )
        local len = #o
		bb:WriteUInt32( len )
        local f = BBuffer.WriteObject
        for i = 1, len do
			f( bb, o[ i ] )
		end
    end
}
BBuffer.Register( List_PKG_CatchFish_Way_ )
List_PKG_CatchFish_Configs_Fish_ = {
    typeName = "List_PKG_CatchFish_Configs_Fish_",
    typeId = 50,
    Create = function()
        local o = {}
        o.__proto = List_PKG_CatchFish_Configs_Fish_
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end

        return o
    end,
    FromBBuffer = function( bb, o )
		local len = bb:ReadUInt32()
        local f = BBuffer.ReadObject
		for i = 1, len do
			o[ i ] = f( bb )
		end
    end,
    ToBBuffer = function( bb, o )
        local len = #o
		bb:WriteUInt32( len )
        local f = BBuffer.WriteObject
        for i = 1, len do
			f( bb, o[ i ] )
		end
    end
}
BBuffer.Register( List_PKG_CatchFish_Configs_Fish_ )
--[[
鱼配置基类 ( 派生类中不再包含 sprite frame 相关, 以便于资源加载管理扫描 )
]]
PKG_CatchFish_Configs_Fish = {
    typeName = "PKG_CatchFish_Configs_Fish",
    typeId = 51,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Configs_Fish
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        金币 / 倍率随机范围 ( 最小值 )
        ]]
        o.minCoin = 0 -- Int64
        --[[
        金币 / 倍率随机范围 ( 最大值 )
        ]]
        o.maxCoin = 0 -- Int64
        --[[
        基于整个鱼的最大晃动范围的圆形碰撞检测半径( 2 判. <= 0 则直接进行 3 判: 物理检测 )
        ]]
        o.maxDetectRadius = 0 -- Single
        --[[
        必然命中的最小检测半径( 1 判. <= 0 则直接进行 2 判. 如果 bulletRadius + minDetectRadius > 子弹中心到鱼中心的距离 就认为命中 )
        ]]
        o.minDetectRadius = 0 -- Single
        --[[
        与该鱼绑定的默认路径集合( 不含鱼阵的路径 ), 为随机路径创造便利
        ]]
        o.ways = null -- List_PKG_CatchFish_Way_
        --[[
        移动帧集合 ( 部分鱼可能具有多种移动状态, 硬编码确定下标范围 )
        ]]
        o.moveFrames = null -- List_PKG_CatchFish_Configs_FishSpriteFrame_
        --[[
        鱼死帧集合
        ]]
        o.dieFrames = null -- List_PKG_CatchFish_Configs_SpriteFrame_
        --[[
        点选优先级说明参数, 越大越优先
        ]]
        o.touchRank = 0 -- Int32
        --[[
        影子显示时的放大系数. 平时与 scale 相等. 部分 boss 影子比身体小.
        ]]
        o.shadowScale = 0 -- Single
        --[[
        影子的偏移坐标
        ]]
        o.shadowOffset = null -- _xx_Pos
        setmetatable( o, PKG_CatchFish_Configs_Item.Create() )
        return o
    end,
    FromBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.FromBBuffer( bb, p )
        local ReadInt64 = bb.ReadInt64
        local ReadSingle = bb.ReadSingle
        local ReadObject = bb.ReadObject
        o.minCoin = ReadInt64( bb )
        o.maxCoin = ReadInt64( bb )
        o.maxDetectRadius = ReadSingle( bb )
        o.minDetectRadius = ReadSingle( bb )
        o.ways = ReadObject( bb )
        o.moveFrames = ReadObject( bb )
        o.dieFrames = ReadObject( bb )
        o.touchRank = bb:ReadInt32()
        o.shadowScale = ReadSingle( bb )
        o.shadowOffset = ReadObject( bb )
    end,
    ToBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.ToBBuffer( bb, p )
        local WriteInt64 = bb.WriteInt64
        local WriteSingle = bb.WriteSingle
        local WriteObject = bb.WriteObject
        WriteInt64( bb, o.minCoin )
        WriteInt64( bb, o.maxCoin )
        WriteSingle( bb, o.maxDetectRadius )
        WriteSingle( bb, o.minDetectRadius )
        WriteObject( bb, o.ways )
        WriteObject( bb, o.moveFrames )
        WriteObject( bb, o.dieFrames )
        bb:WriteInt32( o.touchRank )
        WriteSingle( bb, o.shadowScale )
        WriteObject( bb, o.shadowOffset )
    end
}
BBuffer.Register( PKG_CatchFish_Configs_Fish )
List_PKG_CatchFish_Configs_Cannon_ = {
    typeName = "List_PKG_CatchFish_Configs_Cannon_",
    typeId = 52,
    Create = function()
        local o = {}
        o.__proto = List_PKG_CatchFish_Configs_Cannon_
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end

        return o
    end,
    FromBBuffer = function( bb, o )
		local len = bb:ReadUInt32()
        local f = BBuffer.ReadObject
		for i = 1, len do
			o[ i ] = f( bb )
		end
    end,
    ToBBuffer = function( bb, o )
        local len = #o
		bb:WriteUInt32( len )
        local f = BBuffer.WriteObject
        for i = 1, len do
			f( bb, o[ i ] )
		end
    end
}
BBuffer.Register( List_PKG_CatchFish_Configs_Cannon_ )
--[[
炮台 & 子弹配置基类
]]
PKG_CatchFish_Configs_Cannon = {
    typeName = "PKG_CatchFish_Configs_Cannon",
    typeId = 53,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Configs_Cannon
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        初始角度
        ]]
        o.angle = 0 -- Int32
        --[[
        炮管长度
        ]]
        o.muzzleLen = 0 -- Single
        --[[
        拥有的数量( -1: 无限 )
        ]]
        o.quantity = 0 -- Int32
        --[[
        同屏颗数限制 ( 到达上限就不允许继续发射 )
        ]]
        o.numLimit = 0 -- Int32
        --[[
        发射间隔帧数
        ]]
        o.shootCD = 0 -- Int32
        --[[
        子弹检测半径
        ]]
        o.radius = 0 -- Int32
        --[[
        子弹最大 / 显示半径
        ]]
        o.maxRadius = 0 -- Int32
        --[[
        子弹每帧前进距离
        ]]
        o.distance = 0 -- Single
        setmetatable( o, PKG_CatchFish_Configs_Item.Create() )
        return o
    end,
    FromBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.FromBBuffer( bb, p )
        local ReadInt32 = bb.ReadInt32
        local ReadSingle = bb.ReadSingle
        o.angle = ReadInt32( bb )
        o.muzzleLen = ReadSingle( bb )
        o.quantity = ReadInt32( bb )
        o.numLimit = ReadInt32( bb )
        o.shootCD = ReadInt32( bb )
        o.radius = ReadInt32( bb )
        o.maxRadius = ReadInt32( bb )
        o.distance = ReadSingle( bb )
    end,
    ToBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.ToBBuffer( bb, p )
        local WriteInt32 = bb.WriteInt32
        local WriteSingle = bb.WriteSingle
        WriteInt32( bb, o.angle )
        WriteSingle( bb, o.muzzleLen )
        WriteInt32( bb, o.quantity )
        WriteInt32( bb, o.numLimit )
        WriteInt32( bb, o.shootCD )
        WriteInt32( bb, o.radius )
        WriteInt32( bb, o.maxRadius )
        WriteSingle( bb, o.distance )
    end
}
BBuffer.Register( PKG_CatchFish_Configs_Cannon )
List_PKG_CatchFish_Configs_Weapon_ = {
    typeName = "List_PKG_CatchFish_Configs_Weapon_",
    typeId = 54,
    Create = function()
        local o = {}
        o.__proto = List_PKG_CatchFish_Configs_Weapon_
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end

        return o
    end,
    FromBBuffer = function( bb, o )
		local len = bb:ReadUInt32()
        local f = BBuffer.ReadObject
		for i = 1, len do
			o[ i ] = f( bb )
		end
    end,
    ToBBuffer = function( bb, o )
        local len = #o
		bb:WriteUInt32( len )
        local f = BBuffer.WriteObject
        for i = 1, len do
			f( bb, o[ i ] )
		end
    end
}
BBuffer.Register( List_PKG_CatchFish_Configs_Weapon_ )
--[[
打爆部分特殊鱼出现的特殊武器配置基类
]]
PKG_CatchFish_Configs_Weapon = {
    typeName = "PKG_CatchFish_Configs_Weapon",
    typeId = 55,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Configs_Weapon
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        每帧移动距离
        ]]
        o.distance = 0 -- Single
        --[[
        展示时长 ( 帧数 )
        ]]
        o.showNumFrames = 0 -- Single
        --[[
        飞到玩家坐标之后变化出来的炮台 cfg 之基类
        ]]
        o.cannon = null -- PKG_CatchFish_Configs_Cannon
        setmetatable( o, PKG_CatchFish_Configs_Item.Create() )
        return o
    end,
    FromBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.FromBBuffer( bb, p )
        local ReadSingle = bb.ReadSingle
        o.distance = ReadSingle( bb )
        o.showNumFrames = ReadSingle( bb )
        o.cannon = bb:ReadObject()
    end,
    ToBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.ToBBuffer( bb, p )
        local WriteSingle = bb.WriteSingle
        WriteSingle( bb, o.distance )
        WriteSingle( bb, o.showNumFrames )
        bb:WriteObject( o.cannon )
    end
}
BBuffer.Register( PKG_CatchFish_Configs_Weapon )
List_PKG_CatchFish_Stages_Stage_ = {
    typeName = "List_PKG_CatchFish_Stages_Stage_",
    typeId = 56,
    Create = function()
        local o = {}
        o.__proto = List_PKG_CatchFish_Stages_Stage_
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end

        return o
    end,
    FromBBuffer = function( bb, o )
		local len = bb:ReadUInt32()
        local f = BBuffer.ReadObject
		for i = 1, len do
			o[ i ] = f( bb )
		end
    end,
    ToBBuffer = function( bb, o )
        local len = #o
		bb:WriteUInt32( len )
        local f = BBuffer.WriteObject
        for i = 1, len do
			f( bb, o[ i ] )
		end
    end
}
BBuffer.Register( List_PKG_CatchFish_Stages_Stage_ )
List__xx_Pos_ = {
    typeName = "List__xx_Pos_",
    typeId = 57,
    Create = function()
        local o = {}
        o.__proto = List__xx_Pos_
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end

        return o
    end,
    FromBBuffer = function( bb, o )
		local len = bb:ReadUInt32()
        local f = BBuffer.ReadPos
		for i = 1, len do
			o[ i ] = f( bb )
		end
    end,
    ToBBuffer = function( bb, o )
        local len = #o
		bb:WriteUInt32( len )
        local f = BBuffer.WritePos
        for i = 1, len do
			f( bb, o[ i ] )
		end
    end
}
BBuffer.Register( List__xx_Pos_ )
--[[
配置基类
]]
PKG_CatchFish_Configs_Item = {
    typeName = "PKG_CatchFish_Configs_Item",
    typeId = 58,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Configs_Item
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        内部编号. 通常等同于所在容器下标
        ]]
        o.id = 0 -- Int32
        --[[
        放大系数( 影响各种判定, 坐标计算 )
        ]]
        o.scale = 0 -- Single
        --[[
        初始z轴( 部分 boss 可能临时改变自己的 z )
        ]]
        o.zOrder = 0 -- Int32
        --[[
        帧集合 ( 用于贴图动态加载 / 卸载管理. 派生类所有帧都应该在此放一份 )
        ]]
        o.frames = null -- List_PKG_CatchFish_Configs_SpriteFrame_
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadInt32 = bb.ReadInt32
        o.id = ReadInt32( bb )
        o.scale = bb:ReadSingle()
        o.zOrder = ReadInt32( bb )
        o.frames = bb:ReadObject()
    end,
    ToBBuffer = function( bb, o )
        local WriteInt32 = bb.WriteInt32
        WriteInt32( bb, o.id )
        bb:WriteSingle( o.scale )
        WriteInt32( bb, o.zOrder )
        bb:WriteObject( o.frames )
    end
}
BBuffer.Register( PKG_CatchFish_Configs_Item )
List_PKG_CatchFish_Configs_SpriteFrame_ = {
    typeName = "List_PKG_CatchFish_Configs_SpriteFrame_",
    typeId = 59,
    Create = function()
        local o = {}
        o.__proto = List_PKG_CatchFish_Configs_SpriteFrame_
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end

        return o
    end,
    FromBBuffer = function( bb, o )
		local len = bb:ReadUInt32()
        local f = BBuffer.ReadObject
		for i = 1, len do
			o[ i ] = f( bb )
		end
    end,
    ToBBuffer = function( bb, o )
        local len = #o
		bb:WriteUInt32( len )
        local f = BBuffer.WriteObject
        for i = 1, len do
			f( bb, o[ i ] )
		end
    end
}
BBuffer.Register( List_PKG_CatchFish_Configs_SpriteFrame_ )
--[[
精灵帧
]]
PKG_CatchFish_Configs_SpriteFrame = {
    typeName = "PKG_CatchFish_Configs_SpriteFrame",
    typeId = 60,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Configs_SpriteFrame
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        plist资源名
        ]]
        o.plistName = null -- String
        --[[
        帧名
        ]]
        o.frameName = null -- String
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadObject = bb.ReadObject
        o.plistName = ReadObject( bb )
        o.frameName = ReadObject( bb )
    end,
    ToBBuffer = function( bb, o )
        local WriteObject = bb.WriteObject
        WriteObject( bb, o.plistName )
        WriteObject( bb, o.frameName )
    end
}
BBuffer.Register( PKG_CatchFish_Configs_SpriteFrame )
List_PKG_CatchFish_Configs_FishSpriteFrame_ = {
    typeName = "List_PKG_CatchFish_Configs_FishSpriteFrame_",
    typeId = 61,
    Create = function()
        local o = {}
        o.__proto = List_PKG_CatchFish_Configs_FishSpriteFrame_
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end

        return o
    end,
    FromBBuffer = function( bb, o )
		local len = bb:ReadUInt32()
        local f = BBuffer.ReadObject
		for i = 1, len do
			o[ i ] = f( bb )
		end
    end,
    ToBBuffer = function( bb, o )
        local len = #o
		bb:WriteUInt32( len )
        local f = BBuffer.WriteObject
        for i = 1, len do
			f( bb, o[ i ] )
		end
    end
}
BBuffer.Register( List_PKG_CatchFish_Configs_FishSpriteFrame_ )
--[[
带物理检测区和锁定线等附加数据的鱼移动帧动画
]]
PKG_CatchFish_Configs_FishSpriteFrame = {
    typeName = "PKG_CatchFish_Configs_FishSpriteFrame",
    typeId = 62,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Configs_FishSpriteFrame
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        指向精灵帧
        ]]
        o.frame = null -- PKG_CatchFish_Configs_SpriteFrame
        --[[
        指向物理建模
        ]]
        o.physics = null -- PKG_CatchFish_Configs_Physics
        --[[
        首选锁定点( 如果该点还在屏幕上, 则 lock 准星一直在其上 )
        ]]
        o.lockPoint = null -- _xx_Pos
        --[[
        锁定点集合( 串成一条线的锁定点. 当首选锁定点不在屏上时, 使用该线与所在屏的边线的交点作为锁定点 )
        ]]
        o.lockPoints = null -- List__xx_Pos_
        --[[
        本帧动画切到下一帧动画后应该移动的距离( 受 Fish.speedScale 影响 )
        ]]
        o.moveDistance = 0 -- Single
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadObject = bb.ReadObject
        o.frame = ReadObject( bb )
        o.physics = ReadObject( bb )
        o.lockPoint = ReadObject( bb )
        o.lockPoints = ReadObject( bb )
        o.moveDistance = bb:ReadSingle()
    end,
    ToBBuffer = function( bb, o )
        local WriteObject = bb.WriteObject
        WriteObject( bb, o.frame )
        WriteObject( bb, o.physics )
        WriteObject( bb, o.lockPoint )
        WriteObject( bb, o.lockPoints )
        bb:WriteSingle( o.moveDistance )
    end
}
BBuffer.Register( PKG_CatchFish_Configs_FishSpriteFrame )
--[[
物理建模 for 鱼与子弹碰撞检测
]]
PKG_CatchFish_Configs_Physics = {
    typeName = "PKG_CatchFish_Configs_Physics",
    typeId = 65,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Configs_Physics
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        基于当前帧图的多边形碰撞顶点包围区( 由多个凸多边形组合而成, 用于物理建模碰撞判定 )
        ]]
        o.polygons = null -- List_List__xx_Pos__
        return o
    end,
    FromBBuffer = function( bb, o )
        o.polygons = bb:ReadObject()
    end,
    ToBBuffer = function( bb, o )
        bb:WriteObject( o.polygons )
    end
}
BBuffer.Register( PKG_CatchFish_Configs_Physics )
List_List__xx_Pos__ = {
    typeName = "List_List__xx_Pos__",
    typeId = 63,
    Create = function()
        local o = {}
        o.__proto = List_List__xx_Pos__
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end

        return o
    end,
    FromBBuffer = function( bb, o )
		local len = bb:ReadUInt32()
        local f = BBuffer.ReadList
		for i = 1, len do
			o[ i ] = f( bb )
		end
    end,
    ToBBuffer = function( bb, o )
        local len = #o
		bb:WriteUInt32( len )
        local f = BBuffer.WriteList
        for i = 1, len do
			f( bb, o[ i ] )
		end
    end
}
BBuffer.Register( List_List__xx_Pos__ )