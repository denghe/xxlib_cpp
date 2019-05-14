
PKG_PkgGenMd5_Value = '2fed7bdba66514a0298110a17ec1b22c'

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