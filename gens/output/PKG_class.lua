
PKG_PkgGenMd5_Value = '8a0380cef9eda62800e6c66e92235f9f'

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
PKG_Test_Foo = {
    typeName = "PKG_Test_Foo",
    typeId = 3,
    Create = function()
        local o = {}
        o.__proto = PKG_Test_Foo
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
BBuffer.Register( PKG_Test_Foo )
PKG_Test_Player = {
    typeName = "PKG_Test_Player",
    typeId = 4,
    Create = function()
        local o = {}
        o.__proto = PKG_Test_Player
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        o.id = 0 -- Int32
        o.owner = MakeRef() -- Ref_PKG_Test_Scene
        return o
    end,
    FromBBuffer = function( bb, o )
        o.id = bb:ReadInt32()
        o.owner = MakeRef( ReadObject( bb ) )
    end,
    ToBBuffer = function( bb, o )
        bb:WriteInt32( o.id )
        bb:WriteObject( o.owner.Lock() )
    end
}
BBuffer.Register( PKG_Test_Player )
PKG_Test_Scene = {
    typeName = "PKG_Test_Scene",
    typeId = 5,
    Create = function()
        local o = {}
        o.__proto = PKG_Test_Scene
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        o.fishs = null -- List_PKG_Test_Fish_
        o.players = null -- List_Ref_PKG_Test_Player_
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadObject = bb.ReadObject
        o.fishs = ReadObject( bb )
        o.players = ReadObject( bb )
    end,
    ToBBuffer = function( bb, o )
        local WriteObject = bb.WriteObject
        WriteObject( bb, o.fishs )
        WriteObject( bb, o.players )
    end
}
BBuffer.Register( PKG_Test_Scene )
List_PKG_Test_Fish_ = {
    typeName = "List_PKG_Test_Fish_",
    typeId = 6,
    Create = function()
        local o = {}
        o.__proto = List_PKG_Test_Fish_
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
BBuffer.Register( List_PKG_Test_Fish_ )
PKG_Test_Fish = {
    typeName = "PKG_Test_Fish",
    typeId = 7,
    Create = function()
        local o = {}
        o.__proto = PKG_Test_Fish
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


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
BBuffer.Register( PKG_Test_Fish )
List_Ref_PKG_Test_Player_ = {
    typeName = "List_Ref_PKG_Test_Player_",
    typeId = 8,
    Create = function()
        local o = {}
        o.__proto = List_Ref_PKG_Test_Player_
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
			o[ i ] = MakeRef( f( bb ) )
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
BBuffer.Register( List_Ref_PKG_Test_Player_ )
PKG_Test_EnterSuccess = {
    typeName = "PKG_Test_EnterSuccess",
    typeId = 9,
    Create = function()
        local o = {}
        o.__proto = PKG_Test_EnterSuccess
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        o.players = null -- List_PKG_Test_Player_
        o.scene = null -- PKG_Test_Scene
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadObject = bb.ReadObject
        o.players = ReadObject( bb )
        o.scene = ReadObject( bb )
    end,
    ToBBuffer = function( bb, o )
        local WriteObject = bb.WriteObject
        WriteObject( bb, o.players )
        WriteObject( bb, o.scene )
    end
}
BBuffer.Register( PKG_Test_EnterSuccess )
List_PKG_Test_Player_ = {
    typeName = "List_PKG_Test_Player_",
    typeId = 10,
    Create = function()
        local o = {}
        o.__proto = List_PKG_Test_Player_
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
BBuffer.Register( List_PKG_Test_Player_ )
--[[
通用返回
]]
PKG_Generic_Success = {
    typeName = "PKG_Generic_Success",
    typeId = 11,
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
    typeId = 12,
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
    typeId = 13,
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
    typeId = 14,
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
帧动画基本信息
]]
PKG_CatchFishConfig_SpriteFrameBase = {
    typeName = "PKG_CatchFishConfig_SpriteFrameBase",
    typeId = 15,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFishConfig_SpriteFrameBase
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        贴图名. 通过遍历扫描去重之后, 结合关卡数据, 可以针对即将出现的鱼以及短期内不再出现的鱼做异步加载/卸载
        ]]
        o.textureName = null -- String
        --[[
        帧名
        ]]
        o.frameName = null -- String
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadObject = bb.ReadObject
        o.textureName = ReadObject( bb )
        o.frameName = ReadObject( bb )
    end,
    ToBBuffer = function( bb, o )
        local WriteObject = bb.WriteObject
        WriteObject( bb, o.textureName )
        WriteObject( bb, o.frameName )
    end
}
BBuffer.Register( PKG_CatchFishConfig_SpriteFrameBase )
--[[
帧动画
]]
PKG_CatchFishConfig_SpriteFrame = {
    typeName = "PKG_CatchFishConfig_SpriteFrame",
    typeId = 16,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFishConfig_SpriteFrame
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        基于当前帧图的多边形碰撞顶点包围区( 由多个凸多边形组合而成, 用于物理建模 )
        ]]
        o.polygons = null -- List_List__xx_Pos__
        --[[
        首选锁定点( 如果该点还在屏幕上, 则 lock 准星一直在其上 )
        ]]
        o.lockPoint = null -- _xx_Pos
        --[[
        锁定点集合( 串成一条线的锁定点. 当首选锁定点不在屏上时, 使用该线与所在屏的边线的交点作为锁定点 )
        ]]
        o.lockPoints = null -- List__xx_Pos_
        --[[
        这一帧切到下一帧后应该移动的距离( 受 Fish.speedScale 影响 )
        ]]
        o.distance = 0 -- Single
        setmetatable( o, PKG_CatchFishConfig_SpriteFrameBase.Create() )
        return o
    end,
    FromBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.FromBBuffer( bb, p )
        local ReadObject = bb.ReadObject
        o.polygons = ReadObject( bb )
        o.lockPoint = ReadObject( bb )
        o.lockPoints = ReadObject( bb )
        o.distance = bb:ReadSingle()
    end,
    ToBBuffer = function( bb, o )
        local p = getmetatable( o )
        p.__proto.ToBBuffer( bb, p )
        local WriteObject = bb.WriteObject
        WriteObject( bb, o.polygons )
        WriteObject( bb, o.lockPoint )
        WriteObject( bb, o.lockPoints )
        bb:WriteSingle( o.distance )
    end
}
BBuffer.Register( PKG_CatchFishConfig_SpriteFrame )
List_List__xx_Pos__ = {
    typeName = "List_List__xx_Pos__",
    typeId = 17,
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
List__xx_Pos_ = {
    typeName = "List__xx_Pos_",
    typeId = 18,
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
鱼配置信息基类 ( 部分 boss 可能具有更多 frame, state 等配置参数 )
]]
PKG_CatchFishConfig_Fish = {
    typeName = "PKG_CatchFishConfig_Fish",
    typeId = 19,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFishConfig_Fish
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        配置id
        ]]
        o.id = 0 -- Int32
        --[[
        鱼名
        ]]
        o.name = null -- String
        --[[
        金币 / 倍率 ( 最小值 )
        ]]
        o.minCoin = 0 -- Int64
        --[[
        金币 / 倍率 ( 最大值 )
        ]]
        o.maxCoin = 0 -- Int64
        --[[
        基于整个鱼的最大晃动范围的圆形碰撞检测半径( 粗判 )
        ]]
        o.maxDetectRadius = 0 -- Single
        --[[
        鱼移动帧集合
        ]]
        o.moveFrames = null -- List_PKG_CatchFishConfig_SpriteFrame_
        --[[
        鱼死帧集合
        ]]
        o.dieFrames = null -- List_PKG_CatchFishConfig_SpriteFrameBase_
        --[[
        显示放大系数. 创建精灵时先设起. 后面不用反复改
        ]]
        o.scale = 0 -- Single
        --[[
        屏幕显示 z 轴( 决定显示覆盖顺序 )
        ]]
        o.zOrder = 0 -- Int32
        --[[
        点选优先级说明参数, 越大越优先
        ]]
        o.hitRank = 0 -- Int32
        --[[
        影子显示时的放大系数. 平时与 scale 相等. 部分 boss 影子比身体小.
        ]]
        o.shadowScale = 0 -- Single
        --[[
        影子的偏移坐标
        ]]
        o.shadowOffset = null -- _xx_Pos
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadInt32 = bb.ReadInt32
        local ReadObject = bb.ReadObject
        local ReadInt64 = bb.ReadInt64
        local ReadSingle = bb.ReadSingle
        o.id = ReadInt32( bb )
        o.name = ReadObject( bb )
        o.minCoin = ReadInt64( bb )
        o.maxCoin = ReadInt64( bb )
        o.maxDetectRadius = ReadSingle( bb )
        o.moveFrames = ReadObject( bb )
        o.dieFrames = ReadObject( bb )
        o.scale = ReadSingle( bb )
        o.zOrder = ReadInt32( bb )
        o.hitRank = ReadInt32( bb )
        o.shadowScale = ReadSingle( bb )
        o.shadowOffset = ReadObject( bb )
    end,
    ToBBuffer = function( bb, o )
        local WriteInt32 = bb.WriteInt32
        local WriteObject = bb.WriteObject
        local WriteInt64 = bb.WriteInt64
        local WriteSingle = bb.WriteSingle
        WriteInt32( bb, o.id )
        WriteObject( bb, o.name )
        WriteInt64( bb, o.minCoin )
        WriteInt64( bb, o.maxCoin )
        WriteSingle( bb, o.maxDetectRadius )
        WriteObject( bb, o.moveFrames )
        WriteObject( bb, o.dieFrames )
        WriteSingle( bb, o.scale )
        WriteInt32( bb, o.zOrder )
        WriteInt32( bb, o.hitRank )
        WriteSingle( bb, o.shadowScale )
        WriteObject( bb, o.shadowOffset )
    end
}
BBuffer.Register( PKG_CatchFishConfig_Fish )
List_PKG_CatchFishConfig_SpriteFrame_ = {
    typeName = "List_PKG_CatchFishConfig_SpriteFrame_",
    typeId = 20,
    Create = function()
        local o = {}
        o.__proto = List_PKG_CatchFishConfig_SpriteFrame_
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
BBuffer.Register( List_PKG_CatchFishConfig_SpriteFrame_ )
List_PKG_CatchFishConfig_SpriteFrameBase_ = {
    typeName = "List_PKG_CatchFishConfig_SpriteFrameBase_",
    typeId = 21,
    Create = function()
        local o = {}
        o.__proto = List_PKG_CatchFishConfig_SpriteFrameBase_
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
BBuffer.Register( List_PKG_CatchFishConfig_SpriteFrameBase_ )
PKG_CatchFishConfig_Cannon = {
    typeName = "PKG_CatchFishConfig_Cannon",
    typeId = 22,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFishConfig_Cannon
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        配置id
        ]]
        o.id = 0 -- Int32
        --[[
        对应子弹的配置
        ]]
        o.bullet = null -- PKG_CatchFishConfig_Bullet
        return o
    end,
    FromBBuffer = function( bb, o )
        o.id = bb:ReadInt32()
        o.bullet = bb:ReadObject()
    end,
    ToBBuffer = function( bb, o )
        bb:WriteInt32( o.id )
        bb:WriteObject( o.bullet )
    end
}
BBuffer.Register( PKG_CatchFishConfig_Cannon )
PKG_CatchFishConfig_Bullet = {
    typeName = "PKG_CatchFishConfig_Bullet",
    typeId = 23,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFishConfig_Bullet
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        配置id
        ]]
        o.id = 0 -- Int32
        --[[
        开火火焰帧集合
        ]]
        o.fireFrames = null -- List_PKG_CatchFishConfig_SpriteFrameBase_
        --[[
        子弹移动帧集合
        ]]
        o.moveFrames = null -- List_PKG_CatchFishConfig_SpriteFrameBase_
        --[[
        子弹爆炸帧集合
        ]]
        o.boomFrames = null -- List_PKG_CatchFishConfig_SpriteFrameBase_
        --[[
        渔网帧集合
        ]]
        o.fishingNetsFrames = null -- List_PKG_CatchFishConfig_SpriteFrameBase_
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadObject = bb.ReadObject
        o.id = bb:ReadInt32()
        o.fireFrames = ReadObject( bb )
        o.moveFrames = ReadObject( bb )
        o.boomFrames = ReadObject( bb )
        o.fishingNetsFrames = ReadObject( bb )
    end,
    ToBBuffer = function( bb, o )
        local WriteObject = bb.WriteObject
        bb:WriteInt32( o.id )
        WriteObject( bb, o.fireFrames )
        WriteObject( bb, o.moveFrames )
        WriteObject( bb, o.boomFrames )
        WriteObject( bb, o.fishingNetsFrames )
    end
}
BBuffer.Register( PKG_CatchFishConfig_Bullet )
--[[
游戏配置信息( 配置信息并不会随着网络同步而下发, 反序列化后需要手工还原 )
]]
PKG_CatchFishConfig_Config = {
    typeName = "PKG_CatchFishConfig_Config",
    typeId = 24,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFishConfig_Config
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        所有鱼的配置信息
        ]]
        o.fishs = null -- List_PKG_CatchFishConfig_Fish_
        --[[
        所有炮台的配置信息
        ]]
        o.cannons = null -- List_PKG_CatchFishConfig_Cannon_
        --[[
        基于设计尺寸为 1280 x 720, 屏中心为 0,0 点的 4 个玩家炮台的坐标( 0: 左下  1: 右下    2: 右上  3: 左上 )
        ]]
        o.sitPositons = null -- List__xx_Pos_
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadObject = bb.ReadObject
        o.fishs = ReadObject( bb )
        o.cannons = ReadObject( bb )
        o.sitPositons = ReadObject( bb )
    end,
    ToBBuffer = function( bb, o )
        local WriteObject = bb.WriteObject
        WriteObject( bb, o.fishs )
        WriteObject( bb, o.cannons )
        WriteObject( bb, o.sitPositons )
    end
}
BBuffer.Register( PKG_CatchFishConfig_Config )
List_PKG_CatchFishConfig_Fish_ = {
    typeName = "List_PKG_CatchFishConfig_Fish_",
    typeId = 25,
    Create = function()
        local o = {}
        o.__proto = List_PKG_CatchFishConfig_Fish_
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
BBuffer.Register( List_PKG_CatchFishConfig_Fish_ )
List_PKG_CatchFishConfig_Cannon_ = {
    typeName = "List_PKG_CatchFishConfig_Cannon_",
    typeId = 26,
    Create = function()
        local o = {}
        o.__proto = List_PKG_CatchFishConfig_Cannon_
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
BBuffer.Register( List_PKG_CatchFishConfig_Cannon_ )
--[[
场景基础配置参数 ( 主要来自 db )
]]
PKG_CatchFish_SceneConfig = {
    typeName = "PKG_CatchFish_SceneConfig",
    typeId = 27,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_SceneConfig
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
        最低炮注( coin )
        ]]
        o.minBet = 0 -- Int64
        --[[
        最高炮注( coin )
        ]]
        o.maxBet = 0 -- Int64
        --[[
        进出游戏时 money 自动兑换成 coin 要 乘除 的系数
        ]]
        o.exchangeCoinRatio = 0 -- Int32
        --[[
        子弹颗数限制 ( 分别针对每个炮台 )
        ]]
        o.maxBulletsPerCannon = 0 -- Int64
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadInt32 = bb.ReadInt32
        local ReadInt64 = bb.ReadInt64
        o.gameId = ReadInt32( bb )
        o.levelId = ReadInt32( bb )
        o.roomId = ReadInt32( bb )
        o.minMoney = bb:ReadDouble()
        o.minBet = ReadInt64( bb )
        o.maxBet = ReadInt64( bb )
        o.exchangeCoinRatio = ReadInt32( bb )
        o.maxBulletsPerCannon = ReadInt64( bb )
    end,
    ToBBuffer = function( bb, o )
        local WriteInt32 = bb.WriteInt32
        local WriteInt64 = bb.WriteInt64
        WriteInt32( bb, o.gameId )
        WriteInt32( bb, o.levelId )
        WriteInt32( bb, o.roomId )
        bb:WriteDouble( o.minMoney )
        WriteInt64( bb, o.minBet )
        WriteInt64( bb, o.maxBet )
        WriteInt32( bb, o.exchangeCoinRatio )
        WriteInt64( bb, o.maxBulletsPerCannon )
    end
}
BBuffer.Register( PKG_CatchFish_SceneConfig )
--[[
场景
]]
PKG_CatchFish_Scene = {
    typeName = "PKG_CatchFish_Scene",
    typeId = 28,
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
        场景基础配置参数 ( 主要来自 db )
        ]]
        o.cfg = null -- PKG_CatchFish_SceneConfig
        --[[
        帧编号, 每帧 + 1. 用于同步
        ]]
        o.frameNumber = 0 -- Int32
        --[[
        本地鱼生成专用随机数发生器
        ]]
        o.rnd = null -- _xx_Random
        --[[
        自增id ( 从 1 开始, 用于本地鱼生成 id 填充 )
        ]]
        o.autoIncId = 0 -- Int32
        --[[
        自减id ( 从 -1 开始, 用于服务器下发鱼生成 id 填充 )
        ]]
        o.autoDecId = 0 -- Int32
        --[[
        所有鱼 ( 乱序 )
        ]]
        o.fishss = null -- List_PKG_CatchFish_Fish_
        --[[
        空闲座位下标( 初始时填入 Sits.LeftBottom RightBottom LeftTop RightTop )
        ]]
        o.freeSits = null -- List_PKG_CatchFish_Sits_
        --[[
        所有玩家
        ]]
        o.players = null -- List_Ref_PKG_CatchFish_Player_
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadObject = bb.ReadObject
        local ReadInt32 = bb.ReadInt32
        o.cfg = ReadObject( bb )
        o.frameNumber = ReadInt32( bb )
        o.rnd = ReadObject( bb )
        o.autoIncId = ReadInt32( bb )
        o.autoDecId = ReadInt32( bb )
        o.fishss = ReadObject( bb )
        o.freeSits = ReadObject( bb )
        o.players = ReadObject( bb )
    end,
    ToBBuffer = function( bb, o )
        local WriteObject = bb.WriteObject
        local WriteInt32 = bb.WriteInt32
        WriteObject( bb, o.cfg )
        WriteInt32( bb, o.frameNumber )
        WriteObject( bb, o.rnd )
        WriteInt32( bb, o.autoIncId )
        WriteInt32( bb, o.autoDecId )
        WriteObject( bb, o.fishss )
        WriteObject( bb, o.freeSits )
        WriteObject( bb, o.players )
    end
}
BBuffer.Register( PKG_CatchFish_Scene )
_xx_Random = {
    typeName = "_xx_Random",
    typeId = 29,
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
    typeId = 30,
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
鱼基类
]]
PKG_CatchFish_Fish = {
    typeName = "PKG_CatchFish_Fish",
    typeId = 31,
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
        自增id
        ]]
        o.id = 0 -- Int32
        --[[
        位于容器时的下标 ( 用于快速交换删除 )
        ]]
        o.indexAtContainer = 0 -- Int32
        --[[
        中心点坐标
        ]]
        o.pos = null -- _xx_Pos
        --[[
        配置id( 用来还原配置 )
        ]]
        o.cfgId = 0 -- Int32
        --[[
        移动速度系数 ( 默认为 1 )
        ]]
        o.speedScale = 0 -- Single
        --[[
        碰撞 | 显示 体积系数 ( 默认为 1 )
        ]]
        o.sizeScale = 0 -- Single
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadInt32 = bb.ReadInt32
        local ReadSingle = bb.ReadSingle
        o.id = ReadInt32( bb )
        o.indexAtContainer = ReadInt32( bb )
        o.pos = bb:ReadObject()
        o.cfgId = ReadInt32( bb )
        o.speedScale = ReadSingle( bb )
        o.sizeScale = ReadSingle( bb )
    end,
    ToBBuffer = function( bb, o )
        local WriteInt32 = bb.WriteInt32
        local WriteSingle = bb.WriteSingle
        WriteInt32( bb, o.id )
        WriteInt32( bb, o.indexAtContainer )
        bb:WriteObject( o.pos )
        WriteInt32( bb, o.cfgId )
        WriteSingle( bb, o.speedScale )
        WriteSingle( bb, o.sizeScale )
    end
}
BBuffer.Register( PKG_CatchFish_Fish )
List_PKG_CatchFish_Sits_ = {
    typeName = "List_PKG_CatchFish_Sits_",
    typeId = 32,
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
List_Ref_PKG_CatchFish_Player_ = {
    typeName = "List_Ref_PKG_CatchFish_Player_",
    typeId = 33,
    Create = function()
        local o = {}
        o.__proto = List_Ref_PKG_CatchFish_Player_
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
			o[ i ] = MakeRef( f( bb ) )
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
BBuffer.Register( List_Ref_PKG_CatchFish_Player_ )
--[[
炮台 ( 基类 )
]]
PKG_CatchFish_Cannon = {
    typeName = "PKG_CatchFish_Cannon",
    typeId = 34,
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
        炮倍率 ( 初始填充自 db. 范围限制为 Scene.minBet ~ maxBet )
        ]]
        o.cannonPower = 0 -- Int64
        --[[
        炮管角度 ( 每次发射时都填充一下 )
        ]]
        o.cannonAngle = 0 -- Single
        --[[
        子弹的自增流水号
        ]]
        o.autoIncId = 0 -- Int32
        --[[
        所有子弹
        ]]
        o.bullets = null -- List_PKG_CatchFish_Bullet_
        return o
    end,
    FromBBuffer = function( bb, o )
        o.cannonPower = bb:ReadInt64()
        o.cannonAngle = bb:ReadSingle()
        o.autoIncId = bb:ReadInt32()
        o.bullets = bb:ReadObject()
    end,
    ToBBuffer = function( bb, o )
        bb:WriteInt64( o.cannonPower )
        bb:WriteSingle( o.cannonAngle )
        bb:WriteInt32( o.autoIncId )
        bb:WriteObject( o.bullets )
    end
}
BBuffer.Register( PKG_CatchFish_Cannon )
List_PKG_CatchFish_Bullet_ = {
    typeName = "List_PKG_CatchFish_Bullet_",
    typeId = 35,
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
    typeId = 36,
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
        自增id
        ]]
        o.id = 0 -- Int32
        --[[
        位于容器时的下标 ( 用于快速交换删除 )
        ]]
        o.indexAtContainer = 0 -- Int32
        --[[
        中心点坐标
        ]]
        o.pos = null -- _xx_Pos
        --[[
        每帧的直线移动坐标增量( 60fps )
        ]]
        o.moveInc = null -- _xx_Pos
        --[[
        金币 / 倍率( 记录炮台开火时的 Bet 值 )
        ]]
        o.coin = 0 -- Int64
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadInt32 = bb.ReadInt32
        local ReadObject = bb.ReadObject
        o.id = ReadInt32( bb )
        o.indexAtContainer = ReadInt32( bb )
        o.pos = ReadObject( bb )
        o.moveInc = ReadObject( bb )
        o.coin = bb:ReadInt64()
    end,
    ToBBuffer = function( bb, o )
        local WriteInt32 = bb.WriteInt32
        local WriteObject = bb.WriteObject
        WriteInt32( bb, o.id )
        WriteInt32( bb, o.indexAtContainer )
        WriteObject( bb, o.pos )
        WriteObject( bb, o.moveInc )
        bb:WriteInt64( o.coin )
    end
}
BBuffer.Register( PKG_CatchFish_Bullet )
--[[
玩家 ( 存在于服务 players 容器. 被 Scene.players 弱引用 )
]]
PKG_CatchFish_Player = {
    typeName = "PKG_CatchFish_Player",
    typeId = 37,
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
        当 Client 通过 Lobby 服务到 Game 发 Enter 时, Game 需要生成一个 token 以便 Client Enter 时传入以校验身份
        ]]
        o.token = null -- String
        --[[
        开炮等行为花掉的金币数汇总 ( 统计 )
        ]]
        o.consumeCoin = 0 -- Int64
        --[[
        当前显示游戏币数( 不代表玩家总资产 ). 当玩家进入到游戏时, 该值填充 money * exchangeCoinRatio. 玩家退出时, 做除法还原为 money.
        ]]
        o.coin = 0 -- Int64
        --[[
        所在场景
        ]]
        o.scene = MakeRef() -- Ref_PKG_CatchFish_Scene
        --[[
        座位
        ]]
        o.sit = 0 -- PKG_CatchFish_Sits
        --[[
        破产标识 ( 每帧开始检测一次是否破产, 是就标记之 )
        ]]
        o.isBankRuptcy = false -- Boolean
        --[[
        开火锁定状态
        ]]
        o.fireLocking = false -- Boolean
        --[[
        自动开火状态
        ]]
        o.automating = false -- Boolean
        --[[
        开火锁定瞄准的鱼
        ]]
        o.aimFish = MakeRef() -- Ref_PKG_CatchFish_Fish
        --[[
        自增id ( 从 1 开始, 用于 子弹或炮台 id 填充 )
        ]]
        o.autoIncId = 0 -- Int32
        --[[
        炮台集合 ( 常规炮 打到 钻头, 钻头飞向玩家变为 钻头炮, 覆盖在常规炮上 )
        ]]
        o.cannons = null -- List_PKG_CatchFish_Cannon_
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadInt32 = bb.ReadInt32
        local ReadObject = bb.ReadObject
        local ReadInt64 = bb.ReadInt64
        local ReadBoolean = bb.ReadBoolean
        o.id = ReadInt32( bb )
        o.nickname = ReadObject( bb )
        o.avatar_id = ReadInt32( bb )
        o.token = ReadObject( bb )
        o.consumeCoin = ReadInt64( bb )
        o.coin = ReadInt64( bb )
        o.scene = MakeRef( ReadObject( bb ) )
        o.sit = ReadInt32( bb )
        o.isBankRuptcy = ReadBoolean( bb )
        o.fireLocking = ReadBoolean( bb )
        o.automating = ReadBoolean( bb )
        o.aimFish = MakeRef( ReadObject( bb ) )
        o.autoIncId = ReadInt32( bb )
        o.cannons = ReadObject( bb )
    end,
    ToBBuffer = function( bb, o )
        local WriteInt32 = bb.WriteInt32
        local WriteObject = bb.WriteObject
        local WriteInt64 = bb.WriteInt64
        local WriteBoolean = bb.WriteBoolean
        WriteInt32( bb, o.id )
        WriteObject( bb, o.nickname )
        WriteInt32( bb, o.avatar_id )
        WriteObject( bb, o.token )
        WriteInt64( bb, o.consumeCoin )
        WriteInt64( bb, o.coin )
        WriteObject( bb, o.scene.Lock() )
        WriteInt32( bb, o.sit )
        WriteBoolean( bb, o.isBankRuptcy )
        WriteBoolean( bb, o.fireLocking )
        WriteBoolean( bb, o.automating )
        WriteObject( bb, o.aimFish.Lock() )
        WriteInt32( bb, o.autoIncId )
        WriteObject( bb, o.cannons )
    end
}
BBuffer.Register( PKG_CatchFish_Player )
List_PKG_CatchFish_Cannon_ = {
    typeName = "List_PKG_CatchFish_Cannon_",
    typeId = 38,
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
子弹 & 鱼 & 武器 的基类
]]
PKG_CatchFish_MoveObject = {
    typeName = "PKG_CatchFish_MoveObject",
    typeId = 39,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_MoveObject
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        自增id
        ]]
        o.id = 0 -- Int32
        --[[
        位于容器时的下标 ( 用于快速交换删除 )
        ]]
        o.indexAtContainer = 0 -- Int32
        --[[
        中心点坐标
        ]]
        o.pos = null -- _xx_Pos
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadInt32 = bb.ReadInt32
        o.id = ReadInt32( bb )
        o.indexAtContainer = ReadInt32( bb )
        o.pos = bb:ReadObject()
    end,
    ToBBuffer = function( bb, o )
        local WriteInt32 = bb.WriteInt32
        WriteInt32( bb, o.id )
        WriteInt32( bb, o.indexAtContainer )
        bb:WriteObject( o.pos )
    end
}
BBuffer.Register( PKG_CatchFish_MoveObject )
--[[
武器 ( 有一些特殊鱼死后会变做 某种武器的长相，并花一段世家飞向玩家炮台 )
]]
PKG_CatchFish_Weapon = {
    typeName = "PKG_CatchFish_Weapon",
    typeId = 40,
    Create = function()
        local o = {}
        o.__proto = PKG_CatchFish_Weapon
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
BBuffer.Register( PKG_CatchFish_Weapon )