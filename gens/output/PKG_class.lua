
PKG_PkgGenMd5_Value = '502f8afc15f8a5bc5f55bb013cf82d93'

PKG_Server_Types = {
    Unknown = 0,
    Login = 1,
    Lobby = 2,
    DB = 3,
    MAX = 4
}
--[[
操作成功( 默认 response 结果 )
]]
PKG_Success = {
    typeName = "PKG_Success",
    typeId = 3,
    Create = function()
        local o = {}
        o.__proto = PKG_Success
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
BBuffer.Register( PKG_Success )
--[[
出错( 通用 response 结果 )
]]
PKG_Error = {
    typeName = "PKG_Error",
    typeId = 4,
    Create = function()
        local o = {}
        o.__proto = PKG_Error
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        o.id = 0 -- Int32
        o.txt = null -- String
        return o
    end,
    FromBBuffer = function( bb, o )
        o.id = bb:ReadInt32()
        o.txt = bb:ReadObject()
    end,
    ToBBuffer = function( bb, o )
        bb:WriteInt32( o.id )
        bb:WriteObject( o.txt )
    end
}
BBuffer.Register( PKG_Error )
--[[
服务连接信息
]]
PKG_ConnInfo = {
    typeName = "PKG_ConnInfo",
    typeId = 6,
    Create = function()
        local o = {}
        o.__proto = PKG_ConnInfo
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        服务类型
        ]]
        o.type = 0 -- PKG_Server_Types
        --[[
        ipv4/6 地址
        ]]
        o.ip = null -- String
        --[[
        端口
        ]]
        o.port = 0 -- Int32
        --[[
        令牌
        ]]
        o.token = null -- String
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadInt32 = bb.ReadInt32
        local ReadObject = bb.ReadObject
        o.type = ReadInt32( bb )
        o.ip = ReadObject( bb )
        o.port = ReadInt32( bb )
        o.token = ReadObject( bb )
    end,
    ToBBuffer = function( bb, o )
        local WriteInt32 = bb.WriteInt32
        local WriteObject = bb.WriteObject
        WriteInt32( bb, o.type )
        WriteObject( bb, o.ip )
        WriteInt32( bb, o.port )
        WriteObject( bb, o.token )
    end
}
BBuffer.Register( PKG_ConnInfo )
--[[
并非一般的数据包. 仅用于声明各式 List<T>
]]
PKG_Collections = {
    typeName = "PKG_Collections",
    typeId = 7,
    Create = function()
        local o = {}
        o.__proto = PKG_Collections
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        o.ints = null -- List_Int32_
        o.longs = null -- List_Int64_
        o.strings = null -- List_String_
        o.objects = null -- List_Object_
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadObject = bb.ReadObject
        o.ints = ReadObject( bb )
        o.longs = ReadObject( bb )
        o.strings = ReadObject( bb )
        o.objects = ReadObject( bb )
    end,
    ToBBuffer = function( bb, o )
        local WriteObject = bb.WriteObject
        WriteObject( bb, o.ints )
        WriteObject( bb, o.longs )
        WriteObject( bb, o.strings )
        WriteObject( bb, o.objects )
    end
}
BBuffer.Register( PKG_Collections )
List_Int32_ = {
    typeName = "List_Int32_",
    typeId = 8,
    Create = function()
        local o = {}
        o.__proto = List_Int32_
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
BBuffer.Register( List_Int32_ )
List_Int64_ = {
    typeName = "List_Int64_",
    typeId = 9,
    Create = function()
        local o = {}
        o.__proto = List_Int64_
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
        local f = BBuffer.ReadInt64
		for i = 1, len do
			o[ i ] = f( bb )
		end
    end,
    ToBBuffer = function( bb, o )
        local len = #o
		bb:WriteUInt32( len )
        local f = BBuffer.WriteInt64
        for i = 1, len do
			f( bb, o[ i ] )
		end
    end
}
BBuffer.Register( List_Int64_ )
List_String_ = {
    typeName = "List_String_",
    typeId = 10,
    Create = function()
        local o = {}
        o.__proto = List_String_
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
BBuffer.Register( List_String_ )
List_Object_ = {
    typeName = "List_Object_",
    typeId = 11,
    Create = function()
        local o = {}
        o.__proto = List_Object_
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
BBuffer.Register( List_Object_ )
--[[
校验身份, 成功返回 ConnInfo, 内含下一步需要连接的服务的明细. 失败立即被 T
]]
PKG_Client_Login_Auth = {
    typeName = "PKG_Client_Login_Auth",
    typeId = 12,
    Create = function()
        local o = {}
        o.__proto = PKG_Client_Login_Auth
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        包版本校验
        ]]
        o.pkgMD5 = null -- String
        --[[
        用户名
        ]]
        o.username = null -- String
        --[[
        密码
        ]]
        o.password = null -- String
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadObject = bb.ReadObject
        o.pkgMD5 = ReadObject( bb )
        o.username = ReadObject( bb )
        o.password = ReadObject( bb )
    end,
    ToBBuffer = function( bb, o )
        local WriteObject = bb.WriteObject
        WriteObject( bb, o.pkgMD5 )
        WriteObject( bb, o.username )
        WriteObject( bb, o.password )
    end
}
BBuffer.Register( PKG_Client_Login_Auth )
--[[
首包. 进入大厅. 成功返回 Self( 含 Root 以及个人信息 ). 如果已经位于具体游戏中, 返回 ConnInfo. 失败立即被 T
]]
PKG_Client_Lobby_Enter = {
    typeName = "PKG_Client_Lobby_Enter",
    typeId = 13,
    Create = function()
        local o = {}
        o.__proto = PKG_Client_Lobby_Enter
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        o.token = null -- String
        return o
    end,
    FromBBuffer = function( bb, o )
        o.token = bb:ReadObject()
    end,
    ToBBuffer = function( bb, o )
        bb:WriteObject( o.token )
    end
}
BBuffer.Register( PKG_Client_Lobby_Enter )
--[[
进入 Game1, 位于 Root 时可发送, 返回 Game1. 失败立即被 T
]]
PKG_Client_Lobby_Enter_Game1 = {
    typeName = "PKG_Client_Lobby_Enter_Game1",
    typeId = 14,
    Create = function()
        local o = {}
        o.__proto = PKG_Client_Lobby_Enter_Game1
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
BBuffer.Register( PKG_Client_Lobby_Enter_Game1 )
--[[
进入 Game1 某个 Level, 位于 Game1 时可发送, 返回 Game1_Level. 失败立即被 T
]]
PKG_Client_Lobby_Enter_Game1_Level = {
    typeName = "PKG_Client_Lobby_Enter_Game1_Level",
    typeId = 15,
    Create = function()
        local o = {}
        o.__proto = PKG_Client_Lobby_Enter_Game1_Level
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        指定 Level id
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
BBuffer.Register( PKG_Client_Lobby_Enter_Game1_Level )
--[[
进入 Game1 某个 Level, 位于 Game1 时可发送, 返回 Game1_Level. 失败立即被 T
]]
PKG_Client_Lobby_Enter_Game1_Level_Desk = {
    typeName = "PKG_Client_Lobby_Enter_Game1_Level_Desk",
    typeId = 16,
    Create = function()
        local o = {}
        o.__proto = PKG_Client_Lobby_Enter_Game1_Level_Desk
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        指定 Desk id
        ]]
        o.id = 0 -- Int32
        --[[
        指定座次
        ]]
        o.seatIndex = 0 -- Int32
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadInt32 = bb.ReadInt32
        o.id = ReadInt32( bb )
        o.seatIndex = ReadInt32( bb )
    end,
    ToBBuffer = function( bb, o )
        local WriteInt32 = bb.WriteInt32
        WriteInt32( bb, o.id )
        WriteInt32( bb, o.seatIndex )
    end
}
BBuffer.Register( PKG_Client_Lobby_Enter_Game1_Level_Desk )
--[[
退回上一层. 失败立即被 T
]]
PKG_Client_Lobby_Back = {
    typeName = "PKG_Client_Lobby_Back",
    typeId = 17,
    Create = function()
        local o = {}
        o.__proto = PKG_Client_Lobby_Back
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
BBuffer.Register( PKG_Client_Lobby_Back )
--[[
玩家自己的数据
]]
PKG_Lobby_Client_Self = {
    typeName = "PKG_Lobby_Client_Self",
    typeId = 18,
    Create = function()
        local o = {}
        o.__proto = PKG_Lobby_Client_Self
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        玩家id
        ]]
        o.id = 0 -- Int32
        --[[
        有多少钱
        ]]
        o.money = 0 -- Double
        return o
    end,
    FromBBuffer = function( bb, o )
        o.id = bb:ReadInt32()
        o.money = bb:ReadDouble()
    end,
    ToBBuffer = function( bb, o )
        bb:WriteInt32( o.id )
        bb:WriteDouble( o.money )
    end
}
BBuffer.Register( PKG_Lobby_Client_Self )
--[[
其他玩家的数据
]]
PKG_Lobby_Client_Player = {
    typeName = "PKG_Lobby_Client_Player",
    typeId = 19,
    Create = function()
        local o = {}
        o.__proto = PKG_Lobby_Client_Player
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        玩家id
        ]]
        o.id = 0 -- Int32
        --[[
        名字
        ]]
        o.username = null -- String
        --[[
        特化: 当位于 Game1_Level_Desk.players 之中时的座次附加信息
        ]]
        o.game1_Level_Desk_SeatIndex = 0 -- Int32
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadInt32 = bb.ReadInt32
        o.id = ReadInt32( bb )
        o.username = bb:ReadObject()
        o.game1_Level_Desk_SeatIndex = ReadInt32( bb )
    end,
    ToBBuffer = function( bb, o )
        local WriteInt32 = bb.WriteInt32
        WriteInt32( bb, o.id )
        bb:WriteObject( o.username )
        WriteInt32( bb, o.game1_Level_Desk_SeatIndex )
    end
}
BBuffer.Register( PKG_Lobby_Client_Player )
--[[
大厅根部
]]
PKG_Lobby_Client_Root = {
    typeName = "PKG_Lobby_Client_Root",
    typeId = 20,
    Create = function()
        local o = {}
        o.__proto = PKG_Lobby_Client_Root
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        当前玩家可见的游戏列表
        ]]
        o.gameIds = null -- List_Int32_
        --[[
        玩家自己的数据
        ]]
        o.self = null -- PKG_Lobby_Client_Self
        return o
    end,
    FromBBuffer = function( bb, o )
        local ReadObject = bb.ReadObject
        o.gameIds = ReadObject( bb )
        o.self = ReadObject( bb )
    end,
    ToBBuffer = function( bb, o )
        local WriteObject = bb.WriteObject
        WriteObject( bb, o.gameIds )
        WriteObject( bb, o.self )
    end
}
BBuffer.Register( PKG_Lobby_Client_Root )
--[[
Game 特化: Game1 具体配置信息
]]
PKG_Lobby_Client_Game1 = {
    typeName = "PKG_Lobby_Client_Game1",
    typeId = 21,
    Create = function()
        local o = {}
        o.__proto = PKG_Lobby_Client_Game1
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        Game1 级别列表
        ]]
        o.levels = null -- List_PKG_Lobby_Client_Game1_Level_Info_
        return o
    end,
    FromBBuffer = function( bb, o )
        o.levels = bb:ReadObject()
    end,
    ToBBuffer = function( bb, o )
        bb:WriteObject( o.levels )
    end
}
BBuffer.Register( PKG_Lobby_Client_Game1 )
List_PKG_Lobby_Client_Game1_Level_Info_ = {
    typeName = "List_PKG_Lobby_Client_Game1_Level_Info_",
    typeId = 22,
    Create = function()
        local o = {}
        o.__proto = List_PKG_Lobby_Client_Game1_Level_Info_
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
BBuffer.Register( List_PKG_Lobby_Client_Game1_Level_Info_ )
--[[
Game1 级别的详细数据
]]
PKG_Lobby_Client_Game1_Level_Info = {
    typeName = "PKG_Lobby_Client_Game1_Level_Info",
    typeId = 23,
    Create = function()
        local o = {}
        o.__proto = PKG_Lobby_Client_Game1_Level_Info
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        级别编号
        ]]
        o.id = 0 -- Int32
        --[[
        准入门槛
        ]]
        o.minMoney = 0 -- Double
        return o
    end,
    FromBBuffer = function( bb, o )
        o.id = bb:ReadInt32()
        o.minMoney = bb:ReadDouble()
    end,
    ToBBuffer = function( bb, o )
        bb:WriteInt32( o.id )
        bb:WriteDouble( o.minMoney )
    end
}
BBuffer.Register( PKG_Lobby_Client_Game1_Level_Info )
--[[
Game1 级别的详细数据
]]
PKG_Lobby_Client_Game1_Level = {
    typeName = "PKG_Lobby_Client_Game1_Level",
    typeId = 24,
    Create = function()
        local o = {}
        o.__proto = PKG_Lobby_Client_Game1_Level
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        级别编号
        ]]
        o.id = 0 -- Int32
        --[[
        准入门槛
        ]]
        o.minMoney = 0 -- Double
        --[[
        该级别下所有桌子列表
        ]]
        o.desks = null -- List_PKG_Lobby_Client_Game1_Level_Desk_
        return o
    end,
    FromBBuffer = function( bb, o )
        o.id = bb:ReadInt32()
        o.minMoney = bb:ReadDouble()
        o.desks = bb:ReadObject()
    end,
    ToBBuffer = function( bb, o )
        bb:WriteInt32( o.id )
        bb:WriteDouble( o.minMoney )
        bb:WriteObject( o.desks )
    end
}
BBuffer.Register( PKG_Lobby_Client_Game1_Level )
List_PKG_Lobby_Client_Game1_Level_Desk_ = {
    typeName = "List_PKG_Lobby_Client_Game1_Level_Desk_",
    typeId = 25,
    Create = function()
        local o = {}
        o.__proto = List_PKG_Lobby_Client_Game1_Level_Desk_
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
BBuffer.Register( List_PKG_Lobby_Client_Game1_Level_Desk_ )
--[[
Game1 级别 下的 桌子 的详细数据
]]
PKG_Lobby_Client_Game1_Level_Desk = {
    typeName = "PKG_Lobby_Client_Game1_Level_Desk",
    typeId = 26,
    Create = function()
        local o = {}
        o.__proto = PKG_Lobby_Client_Game1_Level_Desk
        o.__index = o
        o.__newindex = o
		o.__isReleased = false
		o.Release = function()
			o.__isReleased = true
		end


        --[[
        桌子编号
        ]]
        o.id = 0 -- Int32
        --[[
        玩家列表
        ]]
        o.players = null -- List_PKG_Lobby_Client_Player_
        return o
    end,
    FromBBuffer = function( bb, o )
        o.id = bb:ReadInt32()
        o.players = bb:ReadObject()
    end,
    ToBBuffer = function( bb, o )
        bb:WriteInt32( o.id )
        bb:WriteObject( o.players )
    end
}
BBuffer.Register( PKG_Lobby_Client_Game1_Level_Desk )
List_PKG_Lobby_Client_Player_ = {
    typeName = "List_PKG_Lobby_Client_Player_",
    typeId = 27,
    Create = function()
        local o = {}
        o.__proto = List_PKG_Lobby_Client_Player_
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
BBuffer.Register( List_PKG_Lobby_Client_Player_ )