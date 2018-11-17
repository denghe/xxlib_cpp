using System;
namespace PKG
{
    public static class PkgGenMd5
    {
        public const string value = "4859722ae0bbf6c7888c94291c313f3d"; 
    }

namespace Server
{
    public enum Types : int
    {
        Unknown = 0,
        Login = 1,
        Lobby = 2,
        DB = 3,
        MAX = 4,
    }
}
    /// <summary>
    /// 操作成功( 默认 response 结果 )
    /// </summary>
    public partial class Success : xx.Object
    {

        public override ushort GetPackageId()
        {
            return xx.TypeId<Success>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Success\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
        }
    }
    /// <summary>
    /// 出错( 通用 response 结果 )
    /// </summary>
    public partial class Error : xx.Object
    {
        public int id;
        public string txt;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Error>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.id);
            bb.Write(this.txt);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.id);
            bb.readLengthLimit = 0;
            bb.Read(ref this.txt);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Error\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"id\":" + id.ToString());
            if (txt != null) s.Append(", \"txt\":\"" + txt.ToString() + "\"");
            else s.Append(", \"txt\":nil");
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
        }
    }
    /// <summary>
    /// 服务连接信息
    /// </summary>
    public partial class ConnInfo : xx.Object
    {
        /// <summary>
        /// 服务类型
        /// </summary>
        public Server.Types type;
        /// <summary>
        /// ipv4/6 地址
        /// </summary>
        public string ip;
        /// <summary>
        /// 端口
        /// </summary>
        public int port;
        /// <summary>
        /// 令牌
        /// </summary>
        public string token;

        public override ushort GetPackageId()
        {
            return xx.TypeId<ConnInfo>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write((int)this.type);
            bb.Write(this.ip);
            bb.Write(this.port);
            bb.Write(this.token);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            {
                int tmp = 0;
                bb.Read(ref tmp);
                this.type = (Server.Types)tmp;
            }
            bb.readLengthLimit = 0;
            bb.Read(ref this.ip);
            bb.Read(ref this.port);
            bb.readLengthLimit = 0;
            bb.Read(ref this.token);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"ConnInfo\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"type\":" + type.ToString());
            if (ip != null) s.Append(", \"ip\":\"" + ip.ToString() + "\"");
            else s.Append(", \"ip\":nil");
            s.Append(", \"port\":" + port.ToString());
            if (token != null) s.Append(", \"token\":\"" + token.ToString() + "\"");
            else s.Append(", \"token\":nil");
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
        }
    }
    /// <summary>
    /// 并非一般的数据包. 仅用于声明各式 List<T>
    /// </summary>
    public partial class Collections : xx.Object
    {
        public xx.List<int> ints;
        public xx.List<long> longs;
        public xx.List<string> strings;
        public xx.List<xx.Object> objects;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Collections>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.ints);
            bb.Write(this.longs);
            bb.Write(this.strings);
            bb.Write(this.objects);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.readLengthLimit = 0;
            bb.Read(ref this.ints);
            bb.readLengthLimit = 0;
            bb.Read(ref this.longs);
            bb.readLengthLimit = 0;
            bb.Read(ref this.strings);
            bb.readLengthLimit = 0;
            bb.Read(ref this.objects);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Collections\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"ints\":" + (ints == null ? "nil" : ints.ToString()));
            s.Append(", \"longs\":" + (longs == null ? "nil" : longs.ToString()));
            s.Append(", \"strings\":" + (strings == null ? "nil" : strings.ToString()));
            s.Append(", \"objects\":" + (objects == null ? "nil" : objects.ToString()));
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
        }
    }
namespace Client_Login
{
    /// <summary>
    /// 校验身份, 成功返回 ConnInfo, 内含下一步需要连接的服务的明细. 失败立即被 T
    /// </summary>
    public partial class Auth : xx.Object
    {
        /// <summary>
        /// 包版本校验
        /// </summary>
        public string pkgMD5;
        /// <summary>
        /// 用户名
        /// </summary>
        public string username;
        /// <summary>
        /// 密码
        /// </summary>
        public string password;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Auth>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.pkgMD5);
            bb.Write(this.username);
            bb.Write(this.password);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.readLengthLimit = 64;
            bb.Read(ref this.pkgMD5);
            bb.readLengthLimit = 64;
            bb.Read(ref this.username);
            bb.readLengthLimit = 64;
            bb.Read(ref this.password);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Client_Login.Auth\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            if (pkgMD5 != null) s.Append(", \"pkgMD5\":\"" + pkgMD5.ToString() + "\"");
            else s.Append(", \"pkgMD5\":nil");
            if (username != null) s.Append(", \"username\":\"" + username.ToString() + "\"");
            else s.Append(", \"username\":nil");
            if (password != null) s.Append(", \"password\":\"" + password.ToString() + "\"");
            else s.Append(", \"password\":nil");
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
        }
    }
}
namespace Server
{
    /// <summary>
    /// 服务间互表身份的首包
    /// </summary>
    public partial class Info : xx.Object
    {
        public Server.Types type;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Info>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write((int)this.type);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            {
                int tmp = 0;
                bb.Read(ref tmp);
                this.type = (Server.Types)tmp;
            }
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Server.Info\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"type\":" + type.ToString());
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
        }
    }
}
namespace Client_Lobby
{
    /// <summary>
    /// 首包. 进入大厅. 成功返回 Self( 含 Root 以及个人信息 ). 如果已经位于具体游戏中, 返回 ConnInfo. 失败立即被 T
    /// </summary>
    public partial class Enter : xx.Object
    {
        public string token;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Enter>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.token);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.readLengthLimit = 64;
            bb.Read(ref this.token);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Client_Lobby.Enter\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            if (token != null) s.Append(", \"token\":\"" + token.ToString() + "\"");
            else s.Append(", \"token\":nil");
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
        }
    }
    /// <summary>
    /// 进入 Game1, 位于 Root 时可发送, 返回 Game1. 失败立即被 T
    /// </summary>
    public partial class Enter_Game1 : xx.Object
    {

        public override ushort GetPackageId()
        {
            return xx.TypeId<Enter_Game1>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Client_Lobby.Enter_Game1\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
        }
    }
    /// <summary>
    /// 进入 Game1 某个 Level, 位于 Game1 时可发送, 返回 Game1_Level. 失败立即被 T
    /// </summary>
    public partial class Enter_Game1_Level : xx.Object
    {
        /// <summary>
        /// 指定 Level id
        /// </summary>
        public int id;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Enter_Game1_Level>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.id);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.id);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Client_Lobby.Enter_Game1_Level\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"id\":" + id.ToString());
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
        }
    }
    /// <summary>
    /// 进入 Game1 某个 Level, 位于 Game1 时可发送, 返回 Game1_Level. 失败立即被 T
    /// </summary>
    public partial class Enter_Game1_Level_Desk : xx.Object
    {
        /// <summary>
        /// 指定 Desk id
        /// </summary>
        public int id;
        /// <summary>
        /// 指定座次
        /// </summary>
        public int seatIndex;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Enter_Game1_Level_Desk>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.id);
            bb.Write(this.seatIndex);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.id);
            bb.Read(ref this.seatIndex);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Client_Lobby.Enter_Game1_Level_Desk\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"id\":" + id.ToString());
            s.Append(", \"seatIndex\":" + seatIndex.ToString());
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
        }
    }
    /// <summary>
    /// 退回上一层. 失败立即被 T
    /// </summary>
    public partial class Back : xx.Object
    {

        public override ushort GetPackageId()
        {
            return xx.TypeId<Back>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Client_Lobby.Back\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
        }
    }
}
namespace Lobby_Client
{
    /// <summary>
    /// 玩家自己的数据
    /// </summary>
    public partial class Self : xx.Object
    {
        /// <summary>
        /// 玩家id
        /// </summary>
        public int id;
        /// <summary>
        /// 有多少钱
        /// </summary>
        public double money;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Self>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.id);
            bb.Write(this.money);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.id);
            bb.Read(ref this.money);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Lobby_Client.Self\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"id\":" + id.ToString());
            s.Append(", \"money\":" + money.ToString());
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
        }
    }
    /// <summary>
    /// 其他玩家的数据
    /// </summary>
    public partial class Player : xx.Object
    {
        /// <summary>
        /// 玩家id
        /// </summary>
        public int id;
        /// <summary>
        /// 名字
        /// </summary>
        public string username;
        /// <summary>
        /// 特化: 当位于 Game1_Level_Desk.players 之中时的座次附加信息
        /// </summary>
        public int game1_Level_Desk_SeatIndex;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Player>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.id);
            bb.Write(this.username);
            bb.Write(this.game1_Level_Desk_SeatIndex);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.id);
            bb.readLengthLimit = 0;
            bb.Read(ref this.username);
            bb.Read(ref this.game1_Level_Desk_SeatIndex);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Lobby_Client.Player\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"id\":" + id.ToString());
            if (username != null) s.Append(", \"username\":\"" + username.ToString() + "\"");
            else s.Append(", \"username\":nil");
            s.Append(", \"game1_Level_Desk_SeatIndex\":" + game1_Level_Desk_SeatIndex.ToString());
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
        }
    }
    /// <summary>
    /// 大厅根部
    /// </summary>
    public partial class Root : xx.Object
    {
        /// <summary>
        /// 当前玩家可见的游戏列表
        /// </summary>
        public xx.List<int> gameIds;
        /// <summary>
        /// 玩家自己的数据
        /// </summary>
        public Lobby_Client.Self self;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Root>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.gameIds);
            bb.Write(this.self);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.readLengthLimit = 0;
            bb.Read(ref this.gameIds);
            bb.Read(ref this.self);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Lobby_Client.Root\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"gameIds\":" + (gameIds == null ? "nil" : gameIds.ToString()));
            s.Append(", \"self\":" + (self == null ? "nil" : self.ToString()));
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
        }
    }
    /// <summary>
    /// Game 特化: Game1 具体配置信息
    /// </summary>
    public partial class Game1 : xx.Object
    {
        /// <summary>
        /// Game1 级别列表
        /// </summary>
        public xx.List<Lobby_Client.Game1_Level_Info> levels;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Game1>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.levels);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.readLengthLimit = 0;
            bb.Read(ref this.levels);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Lobby_Client.Game1\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"levels\":" + (levels == null ? "nil" : levels.ToString()));
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
        }
    }
    /// <summary>
    /// Game1 级别的详细数据
    /// </summary>
    public partial class Game1_Level_Info : xx.Object
    {
        /// <summary>
        /// 级别编号
        /// </summary>
        public int id;
        /// <summary>
        /// 准入门槛
        /// </summary>
        public double minMoney;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Game1_Level_Info>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.id);
            bb.Write(this.minMoney);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.id);
            bb.Read(ref this.minMoney);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Lobby_Client.Game1_Level_Info\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"id\":" + id.ToString());
            s.Append(", \"minMoney\":" + minMoney.ToString());
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
        }
    }
    /// <summary>
    /// Game1 级别的详细数据
    /// </summary>
    public partial class Game1_Level : xx.Object
    {
        /// <summary>
        /// 级别编号
        /// </summary>
        public int id;
        /// <summary>
        /// 准入门槛
        /// </summary>
        public double minMoney;
        /// <summary>
        /// 该级别下所有桌子列表
        /// </summary>
        public xx.List<Lobby_Client.Game1_Level_Desk> desks;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Game1_Level>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.id);
            bb.Write(this.minMoney);
            bb.Write(this.desks);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.id);
            bb.Read(ref this.minMoney);
            bb.readLengthLimit = 0;
            bb.Read(ref this.desks);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Lobby_Client.Game1_Level\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"id\":" + id.ToString());
            s.Append(", \"minMoney\":" + minMoney.ToString());
            s.Append(", \"desks\":" + (desks == null ? "nil" : desks.ToString()));
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
        }
    }
    /// <summary>
    /// Game1 级别 下的 桌子 的详细数据
    /// </summary>
    public partial class Game1_Level_Desk : xx.Object
    {
        /// <summary>
        /// 桌子编号
        /// </summary>
        public int id;
        /// <summary>
        /// 玩家列表
        /// </summary>
        public xx.List<Lobby_Client.Player> players;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Game1_Level_Desk>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.id);
            bb.Write(this.players);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.id);
            bb.readLengthLimit = 0;
            bb.Read(ref this.players);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Lobby_Client.Game1_Level_Desk\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"id\":" + id.ToString());
            s.Append(", \"players\":" + (players == null ? "nil" : players.ToString()));
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
        }
    }
}
namespace Lobby
{
    /// <summary>
    /// 玩家明细
    /// </summary>
    public partial class Player : xx.Object
    {
        /// <summary>
        /// 玩家id
        /// </summary>
        public int id;
        /// <summary>
        /// 名字
        /// </summary>
        public string username;
        /// <summary>
        /// 有多少钱
        /// </summary>
        public double money;
        /// <summary>
        /// 当前位置
        /// </summary>
        public Lobby.Place place;
        /// <summary>
        /// 位于 players 数组中的下标( 便于交换删除 )
        /// </summary>
        public int indexAtContainer;
        /// <summary>
        /// 特化: 当位于 Game1_Level_Desk.players 之中时的座次附加信息
        /// </summary>
        public int game1_Level_Desk_SeatIndex;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Player>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.id);
            bb.Write(this.username);
            bb.Write(this.money);
            bb.Write(this.place);
            bb.Write(this.indexAtContainer);
            bb.Write(this.game1_Level_Desk_SeatIndex);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.id);
            bb.readLengthLimit = 0;
            bb.Read(ref this.username);
            bb.Read(ref this.money);
            bb.Read(ref this.place);
            bb.Read(ref this.indexAtContainer);
            bb.Read(ref this.game1_Level_Desk_SeatIndex);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Lobby.Player\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"id\":" + id.ToString());
            if (username != null) s.Append(", \"username\":\"" + username.ToString() + "\"");
            else s.Append(", \"username\":nil");
            s.Append(", \"money\":" + money.ToString());
            s.Append(", \"place\":" + (place == null ? "nil" : place.ToString()));
            s.Append(", \"indexAtContainer\":" + indexAtContainer.ToString());
            s.Append(", \"game1_Level_Desk_SeatIndex\":" + game1_Level_Desk_SeatIndex.ToString());
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
        }
    }
    /// <summary>
    /// 玩家容器基类
    /// </summary>
    public partial class Place : xx.Object
    {
        /// <summary>
        /// 指向上层容器( Root 没有上层容器 )
        /// </summary>
        public Lobby.Place parent;
        /// <summary>
        /// 玩家容器基类
        /// </summary>
        public xx.List<Lobby.Player> players;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Place>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.parent);
            bb.Write(this.players);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.parent);
            bb.readLengthLimit = 0;
            bb.Read(ref this.players);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Lobby.Place\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"parent\":" + (parent == null ? "nil" : parent.ToString()));
            s.Append(", \"players\":" + (players == null ? "nil" : players.ToString()));
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
        }
    }
    /// <summary>
    /// 大厅根部
    /// </summary>
    public partial class Root : Lobby.Place
    {
        /// <summary>
        /// 所有游戏列表
        /// </summary>
        public xx.List<Lobby.Game> games;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Root>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.games);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.readLengthLimit = 0;
            bb.Read(ref this.games);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Lobby.Root\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"games\":" + (games == null ? "nil" : games.ToString()));
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
            base.MySqlAppend(sb, ignoreReadOnly);
        }
    }
    /// <summary>
    /// 游戏基类
    /// </summary>
    public partial class Game : Lobby.Place
    {
        /// <summary>
        /// 游戏id
        /// </summary>
        public int id;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Game>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.id);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.Read(ref this.id);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Lobby.Game\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"id\":" + id.ToString());
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
            base.MySqlAppend(sb, ignoreReadOnly);
        }
    }
    /// <summary>
    /// Game 特化: Game1 具体配置信息
    /// </summary>
    public partial class Game1 : Lobby.Game
    {
        /// <summary>
        /// Game1 级别列表
        /// </summary>
        public xx.List<Lobby.Game1_Level> levels;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Game1>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.levels);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.readLengthLimit = 0;
            bb.Read(ref this.levels);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Lobby.Game1\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"levels\":" + (levels == null ? "nil" : levels.ToString()));
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
            base.MySqlAppend(sb, ignoreReadOnly);
        }
    }
    /// <summary>
    /// Game1 级别的详细数据
    /// </summary>
    public partial class Game1_Level : Lobby.Place
    {
        /// <summary>
        /// 级别编号
        /// </summary>
        public int id;
        /// <summary>
        /// 准入门槛
        /// </summary>
        public double minMoney;
        /// <summary>
        /// 该级别下所有桌子列表
        /// </summary>
        public xx.List<Lobby.Game1_Level_Desk> desks;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Game1_Level>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.id);
            bb.Write(this.minMoney);
            bb.Write(this.desks);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.Read(ref this.id);
            bb.Read(ref this.minMoney);
            bb.readLengthLimit = 0;
            bb.Read(ref this.desks);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Lobby.Game1_Level\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"id\":" + id.ToString());
            s.Append(", \"minMoney\":" + minMoney.ToString());
            s.Append(", \"desks\":" + (desks == null ? "nil" : desks.ToString()));
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
            base.MySqlAppend(sb, ignoreReadOnly);
        }
    }
    /// <summary>
    /// Game1 级别 下的 桌子 的详细数据
    /// </summary>
    public partial class Game1_Level_Desk : Lobby.Place
    {
        /// <summary>
        /// 桌子编号
        /// </summary>
        public int id;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Game1_Level_Desk>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.id);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.Read(ref this.id);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Lobby.Game1_Level_Desk\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"id\":" + id.ToString());
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
            base.MySqlAppend(sb, ignoreReadOnly);
        }
    }
}
namespace Login_DB
{
    /// <summary>
    /// 根据用户名获取用户信息. 找到就返回 DB.Account. 找不到就返回 Error
    /// </summary>
    public partial class GetAccount : xx.Object
    {
        public string username;

        public override ushort GetPackageId()
        {
            return xx.TypeId<GetAccount>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.username);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.readLengthLimit = 0;
            bb.Read(ref this.username);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Login_DB.GetAccount\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            if (username != null) s.Append(", \"username\":\"" + username.ToString() + "\"");
            else s.Append(", \"username\":nil");
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
        }
    }
}
namespace DB
{
    public partial class Account : xx.Object
    {
        public int id;
        public string username;
        public string password;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Account>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.id);
            bb.Write(this.username);
            bb.Write(this.password);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.id);
            bb.readLengthLimit = 0;
            bb.Read(ref this.username);
            bb.readLengthLimit = 0;
            bb.Read(ref this.password);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"DB.Account\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"id\":" + id.ToString());
            if (username != null) s.Append(", \"username\":\"" + username.ToString() + "\"");
            else s.Append(", \"username\":nil");
            if (password != null) s.Append(", \"password\":\"" + password.ToString() + "\"");
            else s.Append(", \"password\":nil");
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public override void MySqlAppend(System.Text.StringBuilder sb, bool ignoreReadOnly)
        {
        }
    }
}
    public static class AllTypes
    {
        public static void Register()
        {
            xx.Object.RegisterInternals();
            xx.Object.Register<Success>(3);
            xx.Object.Register<Error>(4);
            xx.Object.Register<ConnInfo>(6);
            xx.Object.Register<Collections>(7);
            xx.Object.Register<xx.List<int>>(8);
            xx.Object.Register<xx.List<long>>(9);
            xx.Object.Register<xx.List<string>>(10);
            xx.Object.Register<xx.List<xx.Object>>(11);
            xx.Object.Register<Client_Login.Auth>(12);
            xx.Object.Register<Server.Info>(5);
            xx.Object.Register<Client_Lobby.Enter>(13);
            xx.Object.Register<Client_Lobby.Enter_Game1>(14);
            xx.Object.Register<Client_Lobby.Enter_Game1_Level>(15);
            xx.Object.Register<Client_Lobby.Enter_Game1_Level_Desk>(16);
            xx.Object.Register<Client_Lobby.Back>(17);
            xx.Object.Register<Lobby_Client.Self>(18);
            xx.Object.Register<Lobby_Client.Player>(19);
            xx.Object.Register<Lobby_Client.Root>(20);
            xx.Object.Register<Lobby_Client.Game1>(21);
            xx.Object.Register<xx.List<Lobby_Client.Game1_Level_Info>>(22);
            xx.Object.Register<Lobby_Client.Game1_Level_Info>(23);
            xx.Object.Register<Lobby_Client.Game1_Level>(24);
            xx.Object.Register<xx.List<Lobby_Client.Game1_Level_Desk>>(25);
            xx.Object.Register<Lobby_Client.Game1_Level_Desk>(26);
            xx.Object.Register<xx.List<Lobby_Client.Player>>(27);
            xx.Object.Register<Lobby.Player>(28);
            xx.Object.Register<Lobby.Place>(29);
            xx.Object.Register<xx.List<Lobby.Player>>(30);
            xx.Object.Register<Lobby.Root>(31);
            xx.Object.Register<xx.List<Lobby.Game>>(32);
            xx.Object.Register<Lobby.Game>(33);
            xx.Object.Register<Lobby.Game1>(34);
            xx.Object.Register<xx.List<Lobby.Game1_Level>>(35);
            xx.Object.Register<Lobby.Game1_Level>(36);
            xx.Object.Register<xx.List<Lobby.Game1_Level_Desk>>(37);
            xx.Object.Register<Lobby.Game1_Level_Desk>(38);
            xx.Object.Register<Login_DB.GetAccount>(39);
            xx.Object.Register<DB.Account>(40);
        }
    }
}
