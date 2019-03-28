
PKG_PkgGenMd5_Value = '47f1f7e598b70be608364d963a2c7534'

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