using System;
namespace PKG
{
    public static class PkgGenMd5
    {
        public const string value = "a27747518b336fe920df86ae0fc4d523"; 
    }

namespace CatchFish
{
    /// <summary>
    /// 座位列表
    /// </summary>
    public enum Sits : int
    {
        /// <summary>
        /// 左下
        /// </summary>
        LeftBottom = 0,
        /// <summary>
        /// 右下
        /// </summary>
        RightBottom = 1,
        /// <summary>
        /// 右上
        /// </summary>
        RightTop = 2,
        /// <summary>
        /// 左上
        /// </summary>
        LeftTop = 3,
    }
}
namespace Generic
{
    /// <summary>
    /// 通用返回
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

            s.Append("{ \"pkgTypeName\":\"Generic.Success\", \"pkgTypeId\":" + GetPackageId());
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
    /// 通用错误返回
    /// </summary>
    public partial class Error : xx.Object
    {
        public long number;
        public string message;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Error>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.number);
            bb.Write(this.message);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.number);
            bb.readLengthLimit = 0;
            bb.Read(ref this.message);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Generic.Error\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"number\":" + number.ToString());
            if (message != null) s.Append(", \"message\":\"" + message.ToString() + "\"");
            else s.Append(", \"message\":nil");
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
    /// 心跳保持兼延迟测试 -- 请求
    /// </summary>
    public partial class Ping : xx.Object
    {
        public long ticks;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Ping>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.ticks);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.ticks);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Generic.Ping\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"ticks\":" + ticks.ToString());
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
    /// 心跳保持兼延迟测试 -- 回应
    /// </summary>
    public partial class Pong : xx.Object
    {
        public long ticks;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Pong>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.ticks);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.ticks);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Generic.Pong\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"ticks\":" + ticks.ToString());
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
namespace CatchFish_Client
{
    /// <summary>
    /// 申请进入游戏 成功
    /// </summary>
    public partial class EnterSuccess : xx.Object
    {
        /// <summary>
        /// 完整的游戏场景
        /// </summary>
        public CatchFish.Scene scene;
        /// <summary>
        /// 玩家强引用容器
        /// </summary>
        public xx.List<CatchFish.Player> players;
        /// <summary>
        /// 指向当前玩家
        /// </summary>
        public xx.Weak<CatchFish.Player> self;
        /// <summary>
        /// 当前 token( 为简化设计先放这. 正常情况下是前置服务告知 )
        /// </summary>
        public string token;

        public override ushort GetPackageId()
        {
            return xx.TypeId<EnterSuccess>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.scene);
            bb.Write(this.players);
            bb.Write(this.self);
            bb.Write(this.token);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.scene);
            bb.readLengthLimit = 0;
            bb.Read(ref this.players);
            bb.Read(ref this.self);
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

            s.Append("{ \"pkgTypeName\":\"CatchFish_Client.EnterSuccess\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"scene\":" + (scene == null ? "nil" : scene.ToString()));
            s.Append(", \"players\":" + (players == null ? "nil" : players.ToString()));
            s.Append(", \"self\":" + self.ToString());
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
    /// 帧事件同步包
    /// </summary>
    public partial class FrameEvents : xx.Object
    {
        /// <summary>
        /// 帧编号
        /// </summary>
        public int frameNumber;
        /// <summary>
        /// 帧事件集合
        /// </summary>
        public xx.List<CatchFish.Events.Event> events;

        public override ushort GetPackageId()
        {
            return xx.TypeId<FrameEvents>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.frameNumber);
            bb.Write(this.events);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.frameNumber);
            bb.readLengthLimit = 0;
            bb.Read(ref this.events);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish_Client.FrameEvents\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"frameNumber\":" + frameNumber.ToString());
            s.Append(", \"events\":" + (events == null ? "nil" : events.ToString()));
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
namespace Client_CatchFish
{
    /// <summary>
    /// 申请进入游戏. 成功返回 EnterSuccess. 失败直接被 T
    /// </summary>
    public partial class Enter : xx.Object
    {
        /// <summary>
        /// 传递先前保存的 token 以便断线重连. 没有传空
        /// </summary>
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

            s.Append("{ \"pkgTypeName\":\"Client_CatchFish.Enter\", \"pkgTypeId\":" + GetPackageId());
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
    /// 调整炮台倍率
    /// </summary>
    public partial class Bet : xx.Object
    {
        public int cannonId;
        public long coin;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Bet>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.cannonId);
            bb.Write(this.coin);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.cannonId);
            bb.Read(ref this.coin);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Client_CatchFish.Bet\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"cannonId\":" + cannonId.ToString());
            s.Append(", \"coin\":" + coin.ToString());
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
    /// 开火
    /// </summary>
    public partial class Fire : xx.Object
    {
        public int frameNumber;
        public int cannonId;
        public int bulletId;
        public xx.Pos pos;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Fire>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.frameNumber);
            bb.Write(this.cannonId);
            bb.Write(this.bulletId);
            ((xx.IObject)this.pos).ToBBuffer(bb);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.frameNumber);
            bb.Read(ref this.cannonId);
            bb.Read(ref this.bulletId);
            ((xx.IObject)this.pos).FromBBuffer(bb);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Client_CatchFish.Fire\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"frameNumber\":" + frameNumber.ToString());
            s.Append(", \"cannonId\":" + cannonId.ToString());
            s.Append(", \"bulletId\":" + bulletId.ToString());
            s.Append(", \"pos\":" + pos.ToString());
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
    /// 碰撞检测
    /// </summary>
    public partial class Hit : xx.Object
    {
        public int cannonId;
        public int bulletId;
        public int fishId;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Hit>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.cannonId);
            bb.Write(this.bulletId);
            bb.Write(this.fishId);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.cannonId);
            bb.Read(ref this.bulletId);
            bb.Read(ref this.fishId);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Client_CatchFish.Hit\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"cannonId\":" + cannonId.ToString());
            s.Append(", \"bulletId\":" + bulletId.ToString());
            s.Append(", \"fishId\":" + fishId.ToString());
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
namespace CatchFish_Calc
{
    /// <summary>
    /// 鱼死计算
    /// </summary>
    public partial class HitCheck : xx.Object
    {
        /// <summary>
        /// 碰撞数据流
        /// </summary>
        public xx.List<Calc.CatchFish.Hit> hits;

        public override ushort GetPackageId()
        {
            return xx.TypeId<HitCheck>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.hits);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.readLengthLimit = 0;
            bb.Read(ref this.hits);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish_Calc.HitCheck\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"hits\":" + (hits == null ? "nil" : hits.ToString()));
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
namespace Calc_CatchFish
{
    /// <summary>
    /// 鱼死计算结果
    /// </summary>
    public partial class HitCheckResult : xx.Object
    {
        /// <summary>
        /// 死鱼列表
        /// </summary>
        public xx.List<Calc.CatchFish.Fish> fishs;
        /// <summary>
        /// 打空的子弹列表
        /// </summary>
        public xx.List<Calc.CatchFish.Bullet> bullets;

        public override ushort GetPackageId()
        {
            return xx.TypeId<HitCheckResult>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.fishs);
            bb.Write(this.bullets);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.readLengthLimit = 0;
            bb.Read(ref this.fishs);
            bb.readLengthLimit = 0;
            bb.Read(ref this.bullets);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Calc_CatchFish.HitCheckResult\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"fishs\":" + (fishs == null ? "nil" : fishs.ToString()));
            s.Append(", \"bullets\":" + (bullets == null ? "nil" : bullets.ToString()));
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
namespace Calc
{
    /// <summary>
    /// 直接向总输入追加数据( 应对点杀之类需求 )
    /// </summary>
    public partial class Push : xx.Object
    {
        public long value;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Push>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.value);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.value);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Calc.Push\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"value\":" + value.ToString());
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
    /// 直接向总输出追加数据( 应对点送之类需求 )
    /// </summary>
    public partial class Pop : xx.Object
    {
        public long value;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Pop>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.value);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.value);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"Calc.Pop\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"value\":" + value.ToString());
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
namespace Calc.CatchFish
{
    /// <summary>
    /// 打击明细
    /// </summary>
    public partial struct Hit : xx.IObject
    {
        /// <summary>
        /// 鱼id 鱼主键1/1
        /// </summary>
        public int fishId;
        /// <summary>
        /// 鱼币值
        /// </summary>
        public long fishCoin;
        /// <summary>
        /// 玩家id 子弹主键1/2
        /// </summary>
        public int playerId;
        /// <summary>
        /// 子弹id 子弹主键2/2
        /// </summary>
        public int bulletId;
        /// <summary>
        /// 子弹数量( 炸弹, 强火力数量会超过 1 )
        /// </summary>
        public int bulletCount;
        /// <summary>
        /// 子弹币值
        /// </summary>
        public long bulletCoin;

        public ushort GetPackageId()
        {
            return xx.TypeId<Hit>.value;
        }

        public void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.fishId);
            bb.Write(this.fishCoin);
            bb.Write(this.playerId);
            bb.Write(this.bulletId);
            bb.Write(this.bulletCount);
            bb.Write(this.bulletCoin);
        }

        public void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.fishId);
            bb.Read(ref this.fishCoin);
            bb.Read(ref this.playerId);
            bb.Read(ref this.bulletId);
            bb.Read(ref this.bulletCount);
            bb.Read(ref this.bulletCoin);
        }
        public void ToString(System.Text.StringBuilder s)
        {
            s.Append("{ \"pkgTypeName\":\"Calc.CatchFish.Hit\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");
        }
        public void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"fishId\":" + fishId.ToString());
            s.Append(", \"fishCoin\":" + fishCoin.ToString());
            s.Append(", \"playerId\":" + playerId.ToString());
            s.Append(", \"bulletId\":" + bulletId.ToString());
            s.Append(", \"bulletCount\":" + bulletCount.ToString());
            s.Append(", \"bulletCoin\":" + bulletCoin.ToString());
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
    }
    /// <summary>
    /// 鱼被打死的明细
    /// </summary>
    public partial struct Fish : xx.IObject
    {
        public int fishId;
        public int playerId;
        public int bulletId;

        public ushort GetPackageId()
        {
            return xx.TypeId<Fish>.value;
        }

        public void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.fishId);
            bb.Write(this.playerId);
            bb.Write(this.bulletId);
        }

        public void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.fishId);
            bb.Read(ref this.playerId);
            bb.Read(ref this.bulletId);
        }
        public void ToString(System.Text.StringBuilder s)
        {
            s.Append("{ \"pkgTypeName\":\"Calc.CatchFish.Fish\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");
        }
        public void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"fishId\":" + fishId.ToString());
            s.Append(", \"playerId\":" + playerId.ToString());
            s.Append(", \"bulletId\":" + bulletId.ToString());
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
    }
    /// <summary>
    /// 子弹打空明细
    /// </summary>
    public partial struct Bullet : xx.IObject
    {
        public int playerId;
        public int bulletId;
        public int bulletCount;

        public ushort GetPackageId()
        {
            return xx.TypeId<Bullet>.value;
        }

        public void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.playerId);
            bb.Write(this.bulletId);
            bb.Write(this.bulletCount);
        }

        public void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.playerId);
            bb.Read(ref this.bulletId);
            bb.Read(ref this.bulletCount);
        }
        public void ToString(System.Text.StringBuilder s)
        {
            s.Append("{ \"pkgTypeName\":\"Calc.CatchFish.Bullet\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");
        }
        public void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"playerId\":" + playerId.ToString());
            s.Append(", \"bulletId\":" + bulletId.ToString());
            s.Append(", \"bulletCount\":" + bulletCount.ToString());
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
    }
}
namespace CatchFish
{
    /// <summary>
    /// 场景
    /// </summary>
    public partial class Scene : xx.Object
    {
        /// <summary>
        /// 游戏id
        /// </summary>
        public int gameId;
        /// <summary>
        /// 级别id
        /// </summary>
        public int levelId;
        /// <summary>
        /// 房间id
        /// </summary>
        public int roomId;
        /// <summary>
        /// 准入金
        /// </summary>
        public double minMoney;
        /// <summary>
        /// 最低炮注( coin )( 针对普通炮台 )
        /// </summary>
        public long minBet;
        /// <summary>
        /// 最高炮注( coin )( 针对普通炮台 )
        /// </summary>
        public long maxBet;
        /// <summary>
        /// 加减炮注跨度( coin )( 针对普通炮台 )
        /// </summary>
        public long stepBet;
        /// <summary>
        /// 进出游戏时 money 自动兑换成 coin 要 乘除 的系数
        /// </summary>
        public int exchangeCoinRatio;
        /// <summary>
        /// 帧编号, 每帧 + 1. 用于同步
        /// </summary>
        public int frameNumber;
        /// <summary>
        /// 本地鱼生成专用随机数发生器
        /// </summary>
        public xx.Random rnd;
        /// <summary>
        /// 自增id ( 从 1 开始, 用于填充 本地鱼 id )
        /// </summary>
        public int autoIncId;
        /// <summary>
        /// 所有活鱼 ( 乱序 )
        /// </summary>
        public xx.List<CatchFish.Fish> fishs;
        /// <summary>
        /// 所有已创建非活鱼 ( 乱序 )
        /// </summary>
        public xx.List<CatchFish.Item> items;
        /// <summary>
        /// 所有鱼预约生成 ( 乱序 )
        /// </summary>
        public xx.List<CatchFish.FishBorn> borns;
        /// <summary>
        /// 当前关卡. endFrameNumber 到达时切换到下一关( clone from cfg.stages[(stage.id + 1) % cfg.stages.len] 并修正 各种 frameNumber )
        /// </summary>
        public CatchFish.Stages.Stage stage;
        /// <summary>
        /// 空闲座位下标( 初始时填入 Sits.LeftBottom RightBottom LeftTop RightTop )
        /// </summary>
        public xx.List<CatchFish.Sits> freeSits;
        /// <summary>
        /// 所有玩家( 弱引用. 具体容器在 Scene 之外 )
        /// </summary>
        public xx.List<xx.Weak<CatchFish.Player>> players;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Scene>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.gameId);
            bb.Write(this.levelId);
            bb.Write(this.roomId);
            bb.Write(this.minMoney);
            bb.Write(this.minBet);
            bb.Write(this.maxBet);
            bb.Write(this.stepBet);
            bb.Write(this.exchangeCoinRatio);
            bb.Write(this.frameNumber);
            bb.Write(this.rnd);
            bb.Write(this.autoIncId);
            bb.Write(this.fishs);
            bb.Write(this.items);
            bb.Write(this.borns);
            bb.Write(this.stage);
            bb.Write(this.freeSits);
            bb.Write(this.players);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.gameId);
            bb.Read(ref this.levelId);
            bb.Read(ref this.roomId);
            bb.Read(ref this.minMoney);
            bb.Read(ref this.minBet);
            bb.Read(ref this.maxBet);
            bb.Read(ref this.stepBet);
            bb.Read(ref this.exchangeCoinRatio);
            bb.Read(ref this.frameNumber);
            bb.Read(ref this.rnd);
            bb.Read(ref this.autoIncId);
            bb.readLengthLimit = 0;
            bb.Read(ref this.fishs);
            bb.readLengthLimit = 0;
            bb.Read(ref this.items);
            bb.readLengthLimit = 0;
            bb.Read(ref this.borns);
            bb.Read(ref this.stage);
            bb.readLengthLimit = 0;
            bb.Read(ref this.freeSits);
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

            s.Append("{ \"pkgTypeName\":\"CatchFish.Scene\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"gameId\":" + gameId.ToString());
            s.Append(", \"levelId\":" + levelId.ToString());
            s.Append(", \"roomId\":" + roomId.ToString());
            s.Append(", \"minMoney\":" + minMoney.ToString());
            s.Append(", \"minBet\":" + minBet.ToString());
            s.Append(", \"maxBet\":" + maxBet.ToString());
            s.Append(", \"stepBet\":" + stepBet.ToString());
            s.Append(", \"exchangeCoinRatio\":" + exchangeCoinRatio.ToString());
            s.Append(", \"frameNumber\":" + frameNumber.ToString());
            s.Append(", \"rnd\":" + (rnd == null ? "nil" : rnd.ToString()));
            s.Append(", \"autoIncId\":" + autoIncId.ToString());
            s.Append(", \"fishs\":" + (fishs == null ? "nil" : fishs.ToString()));
            s.Append(", \"items\":" + (items == null ? "nil" : items.ToString()));
            s.Append(", \"borns\":" + (borns == null ? "nil" : borns.ToString()));
            s.Append(", \"stage\":" + (stage == null ? "nil" : stage.ToString()));
            s.Append(", \"freeSits\":" + (freeSits == null ? "nil" : freeSits.ToString()));
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
    /// 场景元素的共通基类
    /// </summary>
    public partial class Item : xx.Object
    {
        /// <summary>
        /// 标识码
        /// </summary>
        public int id;
        /// <summary>
        /// 位于容器时的下标 ( 用于快速交换删除. 部分类型不一定用到 )
        /// </summary>
        public int indexAtContainer;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Item>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.id);
            bb.Write(this.indexAtContainer);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.id);
            bb.Read(ref this.indexAtContainer);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Item\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"id\":" + id.ToString());
            s.Append(", \"indexAtContainer\":" + indexAtContainer.ToString());
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
    /// 玩家 ( 存在于服务 players 容器. 被 Scene.players 弱引用 )
    /// </summary>
    public partial class Player : CatchFish.Item
    {
        /// <summary>
        /// 昵称 用于客户端显示 ( 填充自 db )
        /// </summary>
        public string nickname;
        /// <summary>
        /// 头像id 用于客户端显示 ( 填充自 db )
        /// </summary>
        public int avatar_id;
        /// <summary>
        /// 破产标识 ( 每帧检测一次总资产是否为 0, 是就标记之. 总资产包括 coin, 已爆出的 weapons, 已获得的附加炮台, 飞行中的 bullets )
        /// </summary>
        public bool noMoney = false;
        /// <summary>
        /// 剩余金币值( 不代表玩家总资产 ). 当玩家进入到游戏时, 该值填充 money * exchangeCoinRatio. 玩家退出时, 做除法还原为 money.
        /// </summary>
        public long coin;
        /// <summary>
        /// 座位
        /// </summary>
        public CatchFish.Sits sit;
        /// <summary>
        /// 自动锁定状态
        /// </summary>
        public bool autoLock = false;
        /// <summary>
        /// 自动开火状态
        /// </summary>
        public bool autoFire = false;
        /// <summary>
        /// 锁定瞄准的鱼
        /// </summary>
        public xx.Weak<CatchFish.Fish> aimFish;
        /// <summary>
        /// 自增id ( 从 1 开始, 用于填充 炮台, 子弹 id )
        /// </summary>
        public int autoIncId;
        /// <summary>
        /// 炮台堆栈 ( 例如: 常规炮 打到 钻头, 钻头飞向玩家变为 钻头炮, 覆盖在常规炮上 )
        /// </summary>
        public xx.List<CatchFish.Cannon> cannons;
        /// <summary>
        /// 武器集合 ( 被打死的特殊鱼转为武器对象, 飞向玩家, 变炮消失前都在这里 )
        /// </summary>
        public xx.List<CatchFish.Weapon> weapons;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Player>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.nickname);
            bb.Write(this.avatar_id);
            bb.Write(this.noMoney);
            bb.Write(this.coin);
            bb.Write((int)this.sit);
            bb.Write(this.autoLock);
            bb.Write(this.autoFire);
            bb.Write(this.aimFish);
            bb.Write(this.autoIncId);
            bb.Write(this.cannons);
            bb.Write(this.weapons);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.readLengthLimit = 0;
            bb.Read(ref this.nickname);
            bb.Read(ref this.avatar_id);
            bb.Read(ref this.noMoney);
            bb.Read(ref this.coin);
            {
                int tmp = 0;
                bb.Read(ref tmp);
                this.sit = (CatchFish.Sits)tmp;
            }
            bb.Read(ref this.autoLock);
            bb.Read(ref this.autoFire);
            bb.Read(ref this.aimFish);
            bb.Read(ref this.autoIncId);
            bb.readLengthLimit = 0;
            bb.Read(ref this.cannons);
            bb.readLengthLimit = 0;
            bb.Read(ref this.weapons);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Player\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            if (nickname != null) s.Append(", \"nickname\":\"" + nickname.ToString() + "\"");
            else s.Append(", \"nickname\":nil");
            s.Append(", \"avatar_id\":" + avatar_id.ToString());
            s.Append(", \"noMoney\":" + noMoney.ToString());
            s.Append(", \"coin\":" + coin.ToString());
            s.Append(", \"sit\":" + sit.ToString());
            s.Append(", \"autoLock\":" + autoLock.ToString());
            s.Append(", \"autoFire\":" + autoFire.ToString());
            s.Append(", \"aimFish\":" + aimFish.ToString());
            s.Append(", \"autoIncId\":" + autoIncId.ToString());
            s.Append(", \"cannons\":" + (cannons == null ? "nil" : cannons.ToString()));
            s.Append(", \"weapons\":" + (weapons == null ? "nil" : weapons.ToString()));
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
    /// 炮台基类. 下列属性适合大多数炮
    /// </summary>
    public partial class Cannon : CatchFish.Item
    {
        /// <summary>
        /// 配置id
        /// </summary>
        public int cfgId;
        /// <summary>
        /// 币值 / 倍率 ( 初始填充自 db. 玩家可调整数值. 范围限制为 Scene.minBet ~ maxBet )
        /// </summary>
        public long coin;
        /// <summary>
        /// 炮管角度 ( 每次发射时都填充一下 )
        /// </summary>
        public float angle;
        /// <summary>
        /// 所有子弹
        /// </summary>
        public xx.List<CatchFish.Bullet> bullets;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Cannon>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.cfgId);
            bb.Write(this.coin);
            bb.Write(this.angle);
            bb.Write(this.bullets);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.Read(ref this.cfgId);
            bb.Read(ref this.coin);
            bb.Read(ref this.angle);
            bb.readLengthLimit = 0;
            bb.Read(ref this.bullets);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Cannon\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"cfgId\":" + cfgId.ToString());
            s.Append(", \"coin\":" + coin.ToString());
            s.Append(", \"angle\":" + angle.ToString());
            s.Append(", \"bullets\":" + (bullets == null ? "nil" : bullets.ToString()));
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
    /// 子弹 & 鱼 & 武器 的基类
    /// </summary>
    public partial class MoveItem : CatchFish.Item
    {
        /// <summary>
        /// 中心点坐标
        /// </summary>
        public xx.Pos pos;
        /// <summary>
        /// 当前角度
        /// </summary>
        public float angle;
        /// <summary>
        /// 每帧的直线移动坐标增量( 不一定用得上 )
        /// </summary>
        public xx.Pos moveInc;

        public override ushort GetPackageId()
        {
            return xx.TypeId<MoveItem>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            ((xx.IObject)this.pos).ToBBuffer(bb);
            bb.Write(this.angle);
            ((xx.IObject)this.moveInc).ToBBuffer(bb);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            ((xx.IObject)this.pos).FromBBuffer(bb);
            bb.Read(ref this.angle);
            ((xx.IObject)this.moveInc).FromBBuffer(bb);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.MoveItem\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"pos\":" + pos.ToString());
            s.Append(", \"angle\":" + angle.ToString());
            s.Append(", \"moveInc\":" + moveInc.ToString());
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
    /// 子弹基类
    /// </summary>
    public partial class Bullet : CatchFish.MoveItem
    {
        /// <summary>
        /// 金币 / 倍率( 记录炮台开火时的 Bet 值 )
        /// </summary>
        public long coin;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Bullet>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.coin);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.Read(ref this.coin);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Bullet\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"coin\":" + coin.ToString());
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
    /// 鱼基类( 支持每帧 pos += moveInc 简单移动 )
    /// </summary>
    public partial class Fish : CatchFish.MoveItem
    {
        /// <summary>
        /// 配置id
        /// </summary>
        public int cfgId;
        /// <summary>
        /// 币值 / 倍率
        /// </summary>
        public long coin;
        /// <summary>
        /// 移动速度系数 ( 默认为 1 )
        /// </summary>
        public float speedScale;
        /// <summary>
        /// 运行时缩放比例( 通常为 1 )
        /// </summary>
        public float scale;
        /// <summary>
        /// 当前帧下标( 循环累加 )
        /// </summary>
        public int spriteFrameIndex;
        /// <summary>
        /// 帧比值, 平时为 1, 如果为 0 则表示鱼不动( 比如实现冰冻效果 ), 帧图也不更新. 如果大于 1, 则需要在 1 帧内多次驱动该鱼( 比如实现快速离场的效果 )
        /// </summary>
        public int frameRatio;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Fish>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.cfgId);
            bb.Write(this.coin);
            bb.Write(this.speedScale);
            bb.Write(this.scale);
            bb.Write(this.spriteFrameIndex);
            bb.Write(this.frameRatio);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.Read(ref this.cfgId);
            bb.Read(ref this.coin);
            bb.Read(ref this.speedScale);
            bb.Read(ref this.scale);
            bb.Read(ref this.spriteFrameIndex);
            bb.Read(ref this.frameRatio);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Fish\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"cfgId\":" + cfgId.ToString());
            s.Append(", \"coin\":" + coin.ToString());
            s.Append(", \"speedScale\":" + speedScale.ToString());
            s.Append(", \"scale\":" + scale.ToString());
            s.Append(", \"spriteFrameIndex\":" + spriteFrameIndex.ToString());
            s.Append(", \"frameRatio\":" + frameRatio.ToString());
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
    /// 武器基类 ( 有一些特殊鱼死后会变做 某种武器 / 炮台，死时有个滞空展示时间，被用于解决网络同步延迟。所有端应该在展示时间结束前收到该预约。展示完成后武器将飞向炮台变为附加炮台 )
    /// </summary>
    public partial class Weapon : CatchFish.MoveItem
    {
        /// <summary>
        /// 配置id
        /// </summary>
        public int cfgId;
        /// <summary>
        /// 开始飞行的帧编号
        /// </summary>
        public int flyFrameNumber;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Weapon>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.cfgId);
            bb.Write(this.flyFrameNumber);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.Read(ref this.cfgId);
            bb.Read(ref this.flyFrameNumber);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Weapon\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"cfgId\":" + cfgId.ToString());
            s.Append(", \"flyFrameNumber\":" + flyFrameNumber.ToString());
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
    /// 预约出鱼
    /// </summary>
    public partial class FishBorn : xx.Object
    {
        /// <summary>
        /// 开始 / 生效帧编号
        /// </summary>
        public int beginFrameNumber;
        /// <summary>
        /// 当 currentFrameNumber == beginFrameNumber 时，将 fish 放入 Scene.fishs 并自杀
        /// </summary>
        public CatchFish.Fish fish;

        public override ushort GetPackageId()
        {
            return xx.TypeId<FishBorn>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.beginFrameNumber);
            bb.Write(this.fish);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.beginFrameNumber);
            bb.Read(ref this.fish);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.FishBorn\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"beginFrameNumber\":" + beginFrameNumber.ToString());
            s.Append(", \"fish\":" + (fish == null ? "nil" : fish.ToString()));
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
    /// 路点
    /// </summary>
    public partial struct WayPoint : xx.IObject
    {
        /// <summary>
        /// 坐标
        /// </summary>
        public xx.Pos pos;
        /// <summary>
        /// 角度
        /// </summary>
        public float angle;
        /// <summary>
        /// 当前点到下一个点的物理/逻辑距离( 下一个点可能是相同坐标, 停在原地转身的效果 )
        /// </summary>
        public float distance;

        public ushort GetPackageId()
        {
            return xx.TypeId<WayPoint>.value;
        }

        public void ToBBuffer(xx.BBuffer bb)
        {
            ((xx.IObject)this.pos).ToBBuffer(bb);
            bb.Write(this.angle);
            bb.Write(this.distance);
        }

        public void FromBBuffer(xx.BBuffer bb)
        {
            ((xx.IObject)this.pos).FromBBuffer(bb);
            bb.Read(ref this.angle);
            bb.Read(ref this.distance);
        }
        public void ToString(System.Text.StringBuilder s)
        {
            s.Append("{ \"pkgTypeName\":\"CatchFish.WayPoint\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");
        }
        public void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"pos\":" + pos.ToString());
            s.Append(", \"angle\":" + angle.ToString());
            s.Append(", \"distance\":" + distance.ToString());
        }
        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
    }
    /// <summary>
    /// 路径. 预约下发安全, 将复制路径完整数据
    /// </summary>
    public partial class Way : xx.Object
    {
        /// <summary>
        /// 路点集合
        /// </summary>
        public xx.List<CatchFish.WayPoint> points;
        /// <summary>
        /// 总距离长度( sum( points[all].distance ). 如果非循环线, 不包含最后一个点的距离值. )
        /// </summary>
        public float distance;
        /// <summary>
        /// 是否循环( 即移动到最后一个点之后又到第 1 个点, 永远走不完
        /// </summary>
        public bool loop = false;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Way>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.points);
            bb.Write(this.distance);
            bb.Write(this.loop);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.readLengthLimit = 0;
            bb.Read(ref this.points);
            bb.Read(ref this.distance);
            bb.Read(ref this.loop);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Way\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"points\":" + (points == null ? "nil" : points.ToString()));
            s.Append(", \"distance\":" + distance.ToString());
            s.Append(", \"loop\":" + loop.ToString());
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
    /// 基于路径移动的鱼基类
    /// </summary>
    public partial class WayFish : CatchFish.Fish
    {
        /// <summary>
        /// 移动路径. 动态生成, 不引用自 cfg. 同步时被复制. 如果该值为空, 则启用 wayTypeIndex / wayIndex
        /// </summary>
        public CatchFish.Way way;
        /// <summary>
        /// cfg.ways[wayTypeIndex]
        /// </summary>
        public int wayTypeIndex;
        /// <summary>
        /// cfg.ways[wayTypeIndex][wayIndex]
        /// </summary>
        public int wayIndex;
        /// <summary>
        /// 当前路点下标
        /// </summary>
        public int wayPointIndex;
        /// <summary>
        /// 当前路点上的已前进距离
        /// </summary>
        public float wayPointDistance;
        /// <summary>
        /// 是否为在路径上倒着移动( 默认否 )
        /// </summary>
        public bool reverse = false;

        public override ushort GetPackageId()
        {
            return xx.TypeId<WayFish>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.way);
            bb.Write(this.wayTypeIndex);
            bb.Write(this.wayIndex);
            bb.Write(this.wayPointIndex);
            bb.Write(this.wayPointDistance);
            bb.Write(this.reverse);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.Read(ref this.way);
            bb.Read(ref this.wayTypeIndex);
            bb.Read(ref this.wayIndex);
            bb.Read(ref this.wayPointIndex);
            bb.Read(ref this.wayPointDistance);
            bb.Read(ref this.reverse);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.WayFish\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"way\":" + (way == null ? "nil" : way.ToString()));
            s.Append(", \"wayTypeIndex\":" + wayTypeIndex.ToString());
            s.Append(", \"wayIndex\":" + wayIndex.ToString());
            s.Append(", \"wayPointIndex\":" + wayPointIndex.ToString());
            s.Append(", \"wayPointDistance\":" + wayPointDistance.ToString());
            s.Append(", \"reverse\":" + reverse.ToString());
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
    /// 围绕目标鱼 圆周 旋转的小鱼( 实现自己的 Move 函数并附加几个计算参数, 被 BigFish Move 调用 )
    /// </summary>
    public partial class RoundFish : CatchFish.Fish
    {
        /// <summary>
        /// 目标大鱼到当前小鱼的角度
        /// </summary>
        public float tarAngle;

        public override ushort GetPackageId()
        {
            return xx.TypeId<RoundFish>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.tarAngle);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.Read(ref this.tarAngle);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.RoundFish\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"tarAngle\":" + tarAngle.ToString());
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
    /// 一只大鱼, 身边围了几只小鱼. 分摊伤害. 随机直线慢移. 自动再生. 切换关卡时快速逃离
    /// </summary>
    public partial class BigFish : CatchFish.Fish
    {
        /// <summary>
        /// 围在身边的小鱼( Update, HitCheck 时级联处理 )
        /// </summary>
        public xx.List<CatchFish.RoundFish> childs;

        public override ushort GetPackageId()
        {
            return xx.TypeId<BigFish>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.childs);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.readLengthLimit = 0;
            bb.Read(ref this.childs);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.BigFish\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"childs\":" + (childs == null ? "nil" : childs.ToString()));
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
namespace CatchFish.Events
{
    /// <summary>
    /// 事件基类
    /// </summary>
    public partial class Event : xx.Object
    {
        /// <summary>
        /// 相关玩家id
        /// </summary>
        public int playerId;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Event>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.playerId);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.playerId);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Events.Event\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"playerId\":" + playerId.ToString());
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
    /// 通知: 玩家进入. 大部分字段从 Player 类复制. 添加了部分初始数值, 可还原出玩家类实例.
    /// </summary>
    public partial class Enter : CatchFish.Events.Event
    {
        /// <summary>
        /// 昵称
        /// </summary>
        public string nickname;
        /// <summary>
        /// 头像id
        /// </summary>
        public int avatar_id;
        /// <summary>
        /// 破产标识
        /// </summary>
        public bool noMoney = false;
        /// <summary>
        /// 剩余金币值
        /// </summary>
        public long coin;
        /// <summary>
        /// 座位
        /// </summary>
        public CatchFish.Sits sit;
        /// <summary>
        /// 炮台配置id
        /// </summary>
        public int cannonCfgId;
        /// <summary>
        /// 炮台币值
        /// </summary>
        public long cannonCoin;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Enter>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.nickname);
            bb.Write(this.avatar_id);
            bb.Write(this.noMoney);
            bb.Write(this.coin);
            bb.Write((int)this.sit);
            bb.Write(this.cannonCfgId);
            bb.Write(this.cannonCoin);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.readLengthLimit = 0;
            bb.Read(ref this.nickname);
            bb.Read(ref this.avatar_id);
            bb.Read(ref this.noMoney);
            bb.Read(ref this.coin);
            {
                int tmp = 0;
                bb.Read(ref tmp);
                this.sit = (CatchFish.Sits)tmp;
            }
            bb.Read(ref this.cannonCfgId);
            bb.Read(ref this.cannonCoin);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Events.Enter\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            if (nickname != null) s.Append(", \"nickname\":\"" + nickname.ToString() + "\"");
            else s.Append(", \"nickname\":nil");
            s.Append(", \"avatar_id\":" + avatar_id.ToString());
            s.Append(", \"noMoney\":" + noMoney.ToString());
            s.Append(", \"coin\":" + coin.ToString());
            s.Append(", \"sit\":" + sit.ToString());
            s.Append(", \"cannonCfgId\":" + cannonCfgId.ToString());
            s.Append(", \"cannonCoin\":" + cannonCoin.ToString());
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
    /// 通知: 玩家离开
    /// </summary>
    public partial class Leave : CatchFish.Events.Event
    {

        public override ushort GetPackageId()
        {
            return xx.TypeId<Leave>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Events.Leave\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
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
    /// 通知: 玩家破产
    /// </summary>
    public partial class NoMoney : CatchFish.Events.Event
    {

        public override ushort GetPackageId()
        {
            return xx.TypeId<NoMoney>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Events.NoMoney\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
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
    /// 通知: 退钱( 常见于子弹并发打中某鱼产生 miss 或鱼id未找到 或子弹生命周期结束 )
    /// </summary>
    public partial class Refund : CatchFish.Events.Event
    {
        /// <summary>
        /// 币值
        /// </summary>
        public long coin;
        /// <summary>
        /// 是否为私人消息( 当服务器收到发射请求并追帧计算后发现子弹已到期，就不会再广播该消息从而导致必须针对该玩家单独通知退款 )
        /// </summary>
        public bool isPersonal = false;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Refund>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.coin);
            bb.Write(this.isPersonal);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.Read(ref this.coin);
            bb.Read(ref this.isPersonal);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Events.Refund\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"coin\":" + coin.ToString());
            s.Append(", \"isPersonal\":" + isPersonal.ToString());
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
    /// 通知: 鱼被打死
    /// </summary>
    public partial class FishDead : CatchFish.Events.Event
    {
        /// <summary>
        /// 鱼id
        /// </summary>
        public int fishId;
        /// <summary>
        /// 子弹id
        /// </summary>
        public int bulletId;
        /// <summary>
        /// 金币所得( fish.coin * bullet.coin 或 server 计算牵连鱼之后的综合结果 )
        /// </summary>
        public long coin;
        /// <summary>
        /// 牵连死的鱼
        /// </summary>
        public xx.List<CatchFish.Events.FishDead> fishDeads;

        public override ushort GetPackageId()
        {
            return xx.TypeId<FishDead>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.fishId);
            bb.Write(this.bulletId);
            bb.Write(this.coin);
            bb.Write(this.fishDeads);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.Read(ref this.fishId);
            bb.Read(ref this.bulletId);
            bb.Read(ref this.coin);
            bb.readLengthLimit = 0;
            bb.Read(ref this.fishDeads);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Events.FishDead\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"fishId\":" + fishId.ToString());
            s.Append(", \"bulletId\":" + bulletId.ToString());
            s.Append(", \"coin\":" + coin.ToString());
            s.Append(", \"fishDeads\":" + (fishDeads == null ? "nil" : fishDeads.ToString()));
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
    /// 通知: 下发已生效 Weapon, 需要判断 flyFrameNumber, 放入 player.weapon 队列
    /// </summary>
    public partial class PushWeapon : CatchFish.Events.Event
    {
        /// <summary>
        /// 已于 server 端构造好的, 无牵挂的, 能干净下发的实例
        /// </summary>
        public CatchFish.Weapon weapon;

        public override ushort GetPackageId()
        {
            return xx.TypeId<PushWeapon>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.weapon);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.Read(ref this.weapon);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Events.PushWeapon\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"weapon\":" + (weapon == null ? "nil" : weapon.ToString()));
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
    /// 预约: 出鱼( 需判定 beginFrameNumber ), 放入 scene.borns 队列. 用不到 playerId
    /// </summary>
    public partial class PushFish : CatchFish.Events.Event
    {
        /// <summary>
        /// 已于 server 端构造好的, 无牵挂的, 能干净下发的实例
        /// </summary>
        public CatchFish.FishBorn born;

        public override ushort GetPackageId()
        {
            return xx.TypeId<PushFish>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.born);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.Read(ref this.born);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Events.PushFish\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"born\":" + (born == null ? "nil" : born.ToString()));
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
    /// 转发: 开启开火锁定
    /// </summary>
    public partial class OpenAutoLock : CatchFish.Events.Event
    {

        public override ushort GetPackageId()
        {
            return xx.TypeId<OpenAutoLock>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Events.OpenAutoLock\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
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
    /// 转发: 玩家锁定后瞄准某鱼
    /// </summary>
    public partial class Aim : CatchFish.Events.Event
    {
        /// <summary>
        /// 被瞄准的鱼id
        /// </summary>
        public int fishId;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Aim>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.fishId);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.Read(ref this.fishId);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Events.Aim\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"fishId\":" + fishId.ToString());
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
    /// 转发: 玩家开火解除锁定
    /// </summary>
    public partial class CloseAutoLock : CatchFish.Events.Event
    {

        public override ushort GetPackageId()
        {
            return xx.TypeId<CloseAutoLock>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Events.CloseAutoLock\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
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
    /// 转发: 玩家自动开火
    /// </summary>
    public partial class OpenAutoFire : CatchFish.Events.Event
    {

        public override ushort GetPackageId()
        {
            return xx.TypeId<OpenAutoFire>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Events.OpenAutoFire\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
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
    /// 转发: 玩家解除自动开火
    /// </summary>
    public partial class CloseAutoFire : CatchFish.Events.Event
    {

        public override ushort GetPackageId()
        {
            return xx.TypeId<CloseAutoFire>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Events.CloseAutoFire\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
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
    /// 转发: 发子弹( 单次 ). 非特殊子弹, 只可能是 cannons[0] 原始炮台发射
    /// </summary>
    public partial class Fire : CatchFish.Events.Event
    {
        /// <summary>
        /// 起始帧编号 ( 来自客户端 )
        /// </summary>
        public int frameNumber;
        /// <summary>
        /// 炮台id
        /// </summary>
        public int cannonId;
        /// <summary>
        /// 子弹id
        /// </summary>
        public int bulletId;
        /// <summary>
        /// 子弹的发射角度
        /// </summary>
        public float tarAngle;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Fire>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.frameNumber);
            bb.Write(this.cannonId);
            bb.Write(this.bulletId);
            bb.Write(this.tarAngle);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.Read(ref this.frameNumber);
            bb.Read(ref this.cannonId);
            bb.Read(ref this.bulletId);
            bb.Read(ref this.tarAngle);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Events.Fire\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"frameNumber\":" + frameNumber.ToString());
            s.Append(", \"cannonId\":" + cannonId.ToString());
            s.Append(", \"bulletId\":" + bulletId.ToString());
            s.Append(", \"tarAngle\":" + tarAngle.ToString());
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
    /// 转发: 切换炮台
    /// </summary>
    public partial class CannonSwitch : CatchFish.Events.Event
    {
        /// <summary>
        /// 炮台配置id
        /// </summary>
        public int cfgId;

        public override ushort GetPackageId()
        {
            return xx.TypeId<CannonSwitch>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.cfgId);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.Read(ref this.cfgId);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Events.CannonSwitch\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"cfgId\":" + cfgId.ToString());
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
    /// 转发: 切换炮台倍率
    /// </summary>
    public partial class CannonCoinChange : CatchFish.Events.Event
    {
        /// <summary>
        /// 炮台id
        /// </summary>
        public int cannonId;
        /// <summary>
        /// 币值 / 倍率
        /// </summary>
        public long coin;

        public override ushort GetPackageId()
        {
            return xx.TypeId<CannonCoinChange>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.cannonId);
            bb.Write(this.coin);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.Read(ref this.cannonId);
            bb.Read(ref this.coin);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Events.CannonCoinChange\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"cannonId\":" + cannonId.ToString());
            s.Append(", \"coin\":" + coin.ToString());
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
    /// 调试信息( 开发阶段校验用 )
    /// </summary>
    public partial class DebugInfo : CatchFish.Events.Event
    {
        /// <summary>
        /// 鱼id集合
        /// </summary>
        public xx.List<int> fishIds;

        public override ushort GetPackageId()
        {
            return xx.TypeId<DebugInfo>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.fishIds);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.readLengthLimit = 0;
            bb.Read(ref this.fishIds);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Events.DebugInfo\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"fishIds\":" + (fishIds == null ? "nil" : fishIds.ToString()));
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
namespace CatchFish.Stages
{
    /// <summary>
    /// 游戏关卡. 一切元素皆使用 Stage.ticks 来计算时间. 可弱引用 Stage 本身. 需要可以干净序列化
    /// </summary>
    public partial class Stage : xx.Object
    {
        /// <summary>
        /// 关卡 id( 通常等于下标值 )
        /// </summary>
        public int cfg_id;
        /// <summary>
        /// 结束时间点
        /// </summary>
        public int cfg_endTicks;
        /// <summary>
        /// 帧编号( 运行时每帧 +1 )
        /// </summary>
        public int ticks;
        /// <summary>
        /// 元素集合
        /// </summary>
        public xx.List<CatchFish.Stages.StageElement> elements;
        /// <summary>
        /// 监视器集合, 服务端专用
        /// </summary>
        public xx.List<CatchFish.Stages.StageElement> monitors;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Stage>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.cfg_id);
            bb.Write(this.cfg_endTicks);
            bb.Write(this.ticks);
            bb.Write(this.elements);
            bb.Write(this.monitors);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.cfg_id);
            bb.Read(ref this.cfg_endTicks);
            bb.Read(ref this.ticks);
            bb.readLengthLimit = 0;
            bb.Read(ref this.elements);
            bb.readLengthLimit = 0;
            bb.Read(ref this.monitors);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Stages.Stage\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"cfg_id\":" + cfg_id.ToString());
            s.Append(", \"cfg_endTicks\":" + cfg_endTicks.ToString());
            s.Append(", \"ticks\":" + ticks.ToString());
            s.Append(", \"elements\":" + (elements == null ? "nil" : elements.ToString()));
            s.Append(", \"monitors\":" + (monitors == null ? "nil" : monitors.ToString()));
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
    /// 关卡元素基类
    /// </summary>
    public partial class StageElement : xx.Object
    {
        /// <summary>
        /// 生效时间点
        /// </summary>
        public int cfg_beginTicks;
        /// <summary>
        /// 结束时间点
        /// </summary>
        public int cfg_endTicks;

        public override ushort GetPackageId()
        {
            return xx.TypeId<StageElement>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.cfg_beginTicks);
            bb.Write(this.cfg_endTicks);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.cfg_beginTicks);
            bb.Read(ref this.cfg_endTicks);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Stages.StageElement\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"cfg_beginTicks\":" + cfg_beginTicks.ToString());
            s.Append(", \"cfg_endTicks\":" + cfg_endTicks.ToString());
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
    /// 发射器: 随机小鱼
    /// </summary>
    public partial class Emitter_RandomFishs : CatchFish.Stages.StageElement
    {
        /// <summary>
        /// 配置: 两条鱼生成帧间隔
        /// </summary>
        public int cfg_bornTicksInterval;
        /// <summary>
        /// 配置: 币值
        /// </summary>
        public long cfg_coin;
        /// <summary>
        /// 配置: 体积随机起始范围
        /// </summary>
        public float cfg_scaleFrom;
        /// <summary>
        /// 配置: 体积随机结束范围
        /// </summary>
        public float cfg_scaleTo;
        /// <summary>
        /// 记录下次生成需要的帧编号( 在生成时令该值 = Stage.ticks + cfg_bornTicksInterval )
        /// </summary>
        public int bornAvaliableTicks;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Emitter_RandomFishs>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.cfg_bornTicksInterval);
            bb.Write(this.cfg_coin);
            bb.Write(this.cfg_scaleFrom);
            bb.Write(this.cfg_scaleTo);
            bb.Write(this.bornAvaliableTicks);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.Read(ref this.cfg_bornTicksInterval);
            bb.Read(ref this.cfg_coin);
            bb.Read(ref this.cfg_scaleFrom);
            bb.Read(ref this.cfg_scaleTo);
            bb.Read(ref this.bornAvaliableTicks);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Stages.Emitter_RandomFishs\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"cfg_bornTicksInterval\":" + cfg_bornTicksInterval.ToString());
            s.Append(", \"cfg_coin\":" + cfg_coin.ToString());
            s.Append(", \"cfg_scaleFrom\":" + cfg_scaleFrom.ToString());
            s.Append(", \"cfg_scaleTo\":" + cfg_scaleTo.ToString());
            s.Append(", \"bornAvaliableTicks\":" + bornAvaliableTicks.ToString());
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
    /// 监视器: 自动再生肥鱼, 服务端预约下发
    /// </summary>
    public partial class Monitor_KeepFatFish : CatchFish.Stages.Emitter_RandomFishs
    {
        /// <summary>
        /// 配置: 鱼总数限制( 可优化为鱼创建 & 析构时去 + - 同步分类统计表. 这个表似乎也可以用个下标来定位元素, 下标存放在 fish 类里面, 可以是个数组 )
        /// </summary>
        public int cfg_numFishsLimit;
        /// <summary>
        /// 配置: 预约延迟
        /// </summary>
        public int cfg_bornDelayFrameNumber;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Monitor_KeepFatFish>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.cfg_numFishsLimit);
            bb.Write(this.cfg_bornDelayFrameNumber);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.Read(ref this.cfg_numFishsLimit);
            bb.Read(ref this.cfg_bornDelayFrameNumber);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Stages.Monitor_KeepFatFish\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"cfg_numFishsLimit\":" + cfg_numFishsLimit.ToString());
            s.Append(", \"cfg_bornDelayFrameNumber\":" + cfg_bornDelayFrameNumber.ToString());
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
    /// 监视器: 自动再生大鱼, 服务端预约下发
    /// </summary>
    public partial class Monitor_KeepBigFish : CatchFish.Stages.StageElement
    {
        /// <summary>
        /// 配置: 两条鱼生成帧间隔
        /// </summary>
        public int cfg_bornTicksInterval;
        /// <summary>
        /// 配置: 鱼总数限制( 可优化为鱼创建 & 析构时去 + - 同步分类统计表. 这个表似乎也可以用个下标来定位元素, 下标存放在 fish 类里面, 可以是个数组 )
        /// </summary>
        public int cfg_numFishsLimit;
        /// <summary>
        /// 配置: 预约延迟
        /// </summary>
        public int cfg_bornDelayFrameNumber;
        /// <summary>
        /// 记录下次生成需要的帧编号( 在生成时令该值 = Stage.ticks + cfg_bornTicksInterval )
        /// </summary>
        public int bornAvaliableTicks;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Monitor_KeepBigFish>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.cfg_bornTicksInterval);
            bb.Write(this.cfg_numFishsLimit);
            bb.Write(this.cfg_bornDelayFrameNumber);
            bb.Write(this.bornAvaliableTicks);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.Read(ref this.cfg_bornTicksInterval);
            bb.Read(ref this.cfg_numFishsLimit);
            bb.Read(ref this.cfg_bornDelayFrameNumber);
            bb.Read(ref this.bornAvaliableTicks);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Stages.Monitor_KeepBigFish\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"cfg_bornTicksInterval\":" + cfg_bornTicksInterval.ToString());
            s.Append(", \"cfg_numFishsLimit\":" + cfg_numFishsLimit.ToString());
            s.Append(", \"cfg_bornDelayFrameNumber\":" + cfg_bornDelayFrameNumber.ToString());
            s.Append(", \"bornAvaliableTicks\":" + bornAvaliableTicks.ToString());
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
    /// 发射器: 从屏幕中间圆环批量出小鱼
    /// </summary>
    public partial class Emitter_RingFishs : CatchFish.Stages.StageElement
    {
        /// <summary>
        /// 配置: 每波鱼只数
        /// </summary>
        public int cfg_numFishsPerBatch;
        /// <summary>
        /// 配置: 两波鱼生成帧间隔
        /// </summary>
        public int cfg_bornTicksInterval;
        /// <summary>
        /// 配置: 每只鱼币值
        /// </summary>
        public long cfg_coin;
        /// <summary>
        /// 配置: 每只鱼体积
        /// </summary>
        public float cfg_scale;
        /// <summary>
        /// 配置: 每只鱼移动速度( 帧跨越像素距离 )
        /// </summary>
        public float cfg_speed;
        /// <summary>
        /// 记录下次生成需要的帧编号( 在生成时令该值 = Stage.ticks + cfg_bornTicksInterval )
        /// </summary>
        public int bornAvaliableTicks;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Emitter_RingFishs>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.cfg_numFishsPerBatch);
            bb.Write(this.cfg_bornTicksInterval);
            bb.Write(this.cfg_coin);
            bb.Write(this.cfg_scale);
            bb.Write(this.cfg_speed);
            bb.Write(this.bornAvaliableTicks);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.Read(ref this.cfg_numFishsPerBatch);
            bb.Read(ref this.cfg_bornTicksInterval);
            bb.Read(ref this.cfg_coin);
            bb.Read(ref this.cfg_scale);
            bb.Read(ref this.cfg_speed);
            bb.Read(ref this.bornAvaliableTicks);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Stages.Emitter_RingFishs\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"cfg_numFishsPerBatch\":" + cfg_numFishsPerBatch.ToString());
            s.Append(", \"cfg_bornTicksInterval\":" + cfg_bornTicksInterval.ToString());
            s.Append(", \"cfg_coin\":" + cfg_coin.ToString());
            s.Append(", \"cfg_scale\":" + cfg_scale.ToString());
            s.Append(", \"cfg_speed\":" + cfg_speed.ToString());
            s.Append(", \"bornAvaliableTicks\":" + bornAvaliableTicks.ToString());
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
    /// 发射器: 从屏幕中间 0 度开始旋转式出小鱼
    /// </summary>
    public partial class Emitter_CircleFishs : CatchFish.Stages.StageElement
    {
        /// <summary>
        /// 配置: 起始角度
        /// </summary>
        public float cfg_angleBegin;
        /// <summary>
        /// 配置: 每只鱼偏转角度
        /// </summary>
        public float cfg_angleIncrease;
        /// <summary>
        /// 配置: 两只鱼生成帧间隔
        /// </summary>
        public int cfg_bornTicksInterval;
        /// <summary>
        /// 配置: 每只鱼币值
        /// </summary>
        public long cfg_coin;
        /// <summary>
        /// 配置: 每只鱼体积
        /// </summary>
        public float cfg_scale;
        /// <summary>
        /// 配置: 每只鱼移动速度( 帧跨越像素距离 )
        /// </summary>
        public float cfg_speed;
        /// <summary>
        /// 记录下次生成需要的帧编号( 在生成时令该值 = Stage.ticks + cfg_bornTicksInterval )
        /// </summary>
        public int bornAvaliableTicks;
        /// <summary>
        /// 当前角度
        /// </summary>
        public float angle;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Emitter_CircleFishs>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.cfg_angleBegin);
            bb.Write(this.cfg_angleIncrease);
            bb.Write(this.cfg_bornTicksInterval);
            bb.Write(this.cfg_coin);
            bb.Write(this.cfg_scale);
            bb.Write(this.cfg_speed);
            bb.Write(this.bornAvaliableTicks);
            bb.Write(this.angle);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.Read(ref this.cfg_angleBegin);
            bb.Read(ref this.cfg_angleIncrease);
            bb.Read(ref this.cfg_bornTicksInterval);
            bb.Read(ref this.cfg_coin);
            bb.Read(ref this.cfg_scale);
            bb.Read(ref this.cfg_speed);
            bb.Read(ref this.bornAvaliableTicks);
            bb.Read(ref this.angle);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Stages.Emitter_CircleFishs\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"cfg_angleBegin\":" + cfg_angleBegin.ToString());
            s.Append(", \"cfg_angleIncrease\":" + cfg_angleIncrease.ToString());
            s.Append(", \"cfg_bornTicksInterval\":" + cfg_bornTicksInterval.ToString());
            s.Append(", \"cfg_coin\":" + cfg_coin.ToString());
            s.Append(", \"cfg_scale\":" + cfg_scale.ToString());
            s.Append(", \"cfg_speed\":" + cfg_speed.ToString());
            s.Append(", \"bornAvaliableTicks\":" + bornAvaliableTicks.ToString());
            s.Append(", \"angle\":" + angle.ToString());
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
namespace CatchFish.Configs
{
    /// <summary>
    /// 游戏配置主体
    /// </summary>
    public partial class Config : xx.Object
    {
        /// <summary>
        /// 所有固定路径( 工具创建 )
        /// </summary>
        public xx.List<CatchFish.Way> fixedWays;
        /// <summary>
        /// 所有鱼的配置信息
        /// </summary>
        public xx.List<CatchFish.Configs.Fish> fishs;
        /// <summary>
        /// 所有炮台的配置信息
        /// </summary>
        public xx.List<CatchFish.Configs.Cannon> cannons;
        /// <summary>
        /// 所有武器的配置信息
        /// </summary>
        public xx.List<CatchFish.Configs.Weapon> weapons;
        /// <summary>
        /// 循环关卡数据( Scene 初次创建时，从 stages[0] clone. 可以在内存中 cache 序列化后的 binary )
        /// </summary>
        public xx.List<CatchFish.Stages.Stage> stages;
        /// <summary>
        /// 基于设计尺寸为 1280 x 720, 屏中心为 0,0 点的 4 个玩家炮台的坐标( 0: 左下  1: 右下    2: 右上  3: 左上 )
        /// </summary>
        public xx.List<xx.Pos> sitPositons;
        /// <summary>
        /// 锁定点击范围 ( 增加容错, 不必点的太精确. 点击作用是 枚举该范围内出现的鱼, 找出并选取 touchRank 最大值那个 )
        /// </summary>
        public float aimTouchRadius;
        /// <summary>
        /// 普通鱼最大半径 ( 用于生成鱼线确保鱼出现时刚好位于屏幕外 )
        /// </summary>
        public float normalFishMaxRadius;
        /// <summary>
        /// 显示非当前玩家子弹时是否启用追帧快进令其同步( 会导致高延迟玩家发射的子弹看上去离炮口有点远 )
        /// </summary>
        public bool enableBulletFastForward = false;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Config>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.fixedWays);
            bb.Write(this.fishs);
            bb.Write(this.cannons);
            bb.Write(this.weapons);
            bb.Write(this.stages);
            bb.Write(this.sitPositons);
            bb.Write(this.aimTouchRadius);
            bb.Write(this.normalFishMaxRadius);
            bb.Write(this.enableBulletFastForward);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.readLengthLimit = 0;
            bb.Read(ref this.fixedWays);
            bb.readLengthLimit = 0;
            bb.Read(ref this.fishs);
            bb.readLengthLimit = 0;
            bb.Read(ref this.cannons);
            bb.readLengthLimit = 0;
            bb.Read(ref this.weapons);
            bb.readLengthLimit = 0;
            bb.Read(ref this.stages);
            bb.readLengthLimit = 0;
            bb.Read(ref this.sitPositons);
            bb.Read(ref this.aimTouchRadius);
            bb.Read(ref this.normalFishMaxRadius);
            bb.Read(ref this.enableBulletFastForward);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Configs.Config\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"fixedWays\":" + (fixedWays == null ? "nil" : fixedWays.ToString()));
            s.Append(", \"fishs\":" + (fishs == null ? "nil" : fishs.ToString()));
            s.Append(", \"cannons\":" + (cannons == null ? "nil" : cannons.ToString()));
            s.Append(", \"weapons\":" + (weapons == null ? "nil" : weapons.ToString()));
            s.Append(", \"stages\":" + (stages == null ? "nil" : stages.ToString()));
            s.Append(", \"sitPositons\":" + (sitPositons == null ? "nil" : sitPositons.ToString()));
            s.Append(", \"aimTouchRadius\":" + aimTouchRadius.ToString());
            s.Append(", \"normalFishMaxRadius\":" + normalFishMaxRadius.ToString());
            s.Append(", \"enableBulletFastForward\":" + enableBulletFastForward.ToString());
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
    /// 配置基类
    /// </summary>
    public partial class Item : xx.Object
    {
        /// <summary>
        /// 内部编号. 通常等同于所在容器下标
        /// </summary>
        public int id;
        /// <summary>
        /// 放大系数( 影响各种判定, 坐标计算 )
        /// </summary>
        public float scale;
        /// <summary>
        /// 初始z轴( 部分 boss 可能临时改变自己的 z )
        /// </summary>
        public int zOrder;
        /// <summary>
        /// 帧集合 ( 用于贴图动态加载 / 卸载管理. 派生类所有帧都应该在此放一份 )
        /// </summary>
        public xx.List<CatchFish.Configs.SpriteFrame> frames;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Item>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.id);
            bb.Write(this.scale);
            bb.Write(this.zOrder);
            bb.Write(this.frames);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.id);
            bb.Read(ref this.scale);
            bb.Read(ref this.zOrder);
            bb.readLengthLimit = 0;
            bb.Read(ref this.frames);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Configs.Item\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"id\":" + id.ToString());
            s.Append(", \"scale\":" + scale.ToString());
            s.Append(", \"zOrder\":" + zOrder.ToString());
            s.Append(", \"frames\":" + (frames == null ? "nil" : frames.ToString()));
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
    /// 鱼配置基类 ( 派生类中不再包含 sprite frame 相关, 以便于资源加载管理扫描 )
    /// </summary>
    public partial class Fish : CatchFish.Configs.Item
    {
        /// <summary>
        /// 金币 / 倍率随机范围 ( 最小值 )
        /// </summary>
        public long minCoin;
        /// <summary>
        /// 金币 / 倍率随机范围 ( 最大值 )
        /// </summary>
        public long maxCoin;
        /// <summary>
        /// 基于整个鱼的最大晃动范围的圆形碰撞检测半径( 2 判. <= 0 则直接进行 3 判: 物理检测 )
        /// </summary>
        public float maxDetectRadius;
        /// <summary>
        /// 必然命中的最小检测半径( 1 判. <= 0 则直接进行 2 判. 如果 bulletRadius + minDetectRadius > 子弹中心到鱼中心的距离 就认为命中 )
        /// </summary>
        public float minDetectRadius;
        /// <summary>
        /// 移动帧集合 ( 部分鱼可能具有多种移动状态, 硬编码确定下标范围 )
        /// </summary>
        public xx.List<CatchFish.Configs.FishSpriteFrame> moveFrames;
        /// <summary>
        /// 鱼死帧集合
        /// </summary>
        public xx.List<CatchFish.Configs.SpriteFrame> dieFrames;
        /// <summary>
        /// 点选优先级说明参数, 越大越优先
        /// </summary>
        public int touchRank;
        /// <summary>
        /// 影子显示时的放大系数. 平时与 scale 相等. 部分 boss 影子比身体小.
        /// </summary>
        public float shadowScale;
        /// <summary>
        /// 影子的偏移坐标
        /// </summary>
        public xx.Pos shadowOffset;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Fish>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.minCoin);
            bb.Write(this.maxCoin);
            bb.Write(this.maxDetectRadius);
            bb.Write(this.minDetectRadius);
            bb.Write(this.moveFrames);
            bb.Write(this.dieFrames);
            bb.Write(this.touchRank);
            bb.Write(this.shadowScale);
            ((xx.IObject)this.shadowOffset).ToBBuffer(bb);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.Read(ref this.minCoin);
            bb.Read(ref this.maxCoin);
            bb.Read(ref this.maxDetectRadius);
            bb.Read(ref this.minDetectRadius);
            bb.readLengthLimit = 0;
            bb.Read(ref this.moveFrames);
            bb.readLengthLimit = 0;
            bb.Read(ref this.dieFrames);
            bb.Read(ref this.touchRank);
            bb.Read(ref this.shadowScale);
            ((xx.IObject)this.shadowOffset).FromBBuffer(bb);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Configs.Fish\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"minCoin\":" + minCoin.ToString());
            s.Append(", \"maxCoin\":" + maxCoin.ToString());
            s.Append(", \"maxDetectRadius\":" + maxDetectRadius.ToString());
            s.Append(", \"minDetectRadius\":" + minDetectRadius.ToString());
            s.Append(", \"moveFrames\":" + (moveFrames == null ? "nil" : moveFrames.ToString()));
            s.Append(", \"dieFrames\":" + (dieFrames == null ? "nil" : dieFrames.ToString()));
            s.Append(", \"touchRank\":" + touchRank.ToString());
            s.Append(", \"shadowScale\":" + shadowScale.ToString());
            s.Append(", \"shadowOffset\":" + shadowOffset.ToString());
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
    /// 炮台 & 子弹配置基类
    /// </summary>
    public partial class Cannon : CatchFish.Configs.Item
    {
        /// <summary>
        /// 初始角度
        /// </summary>
        public float angle;
        /// <summary>
        /// 炮管长度
        /// </summary>
        public float muzzleLen;
        /// <summary>
        /// 拥有的数量( -1: 无限 )
        /// </summary>
        public int quantity;
        /// <summary>
        /// 同屏颗数限制 ( 到达上限就不允许继续发射 )
        /// </summary>
        public int numLimit;
        /// <summary>
        /// 发射间隔帧数
        /// </summary>
        public int fireCD;
        /// <summary>
        /// 子弹检测半径
        /// </summary>
        public int radius;
        /// <summary>
        /// 子弹最大 / 显示半径
        /// </summary>
        public int maxRadius;
        /// <summary>
        /// 子弹每帧前进距离
        /// </summary>
        public float distance;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Cannon>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.angle);
            bb.Write(this.muzzleLen);
            bb.Write(this.quantity);
            bb.Write(this.numLimit);
            bb.Write(this.fireCD);
            bb.Write(this.radius);
            bb.Write(this.maxRadius);
            bb.Write(this.distance);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.Read(ref this.angle);
            bb.Read(ref this.muzzleLen);
            bb.Read(ref this.quantity);
            bb.Read(ref this.numLimit);
            bb.Read(ref this.fireCD);
            bb.Read(ref this.radius);
            bb.Read(ref this.maxRadius);
            bb.Read(ref this.distance);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Configs.Cannon\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"angle\":" + angle.ToString());
            s.Append(", \"muzzleLen\":" + muzzleLen.ToString());
            s.Append(", \"quantity\":" + quantity.ToString());
            s.Append(", \"numLimit\":" + numLimit.ToString());
            s.Append(", \"fireCD\":" + fireCD.ToString());
            s.Append(", \"radius\":" + radius.ToString());
            s.Append(", \"maxRadius\":" + maxRadius.ToString());
            s.Append(", \"distance\":" + distance.ToString());
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
    /// 打爆部分特殊鱼出现的特殊武器配置基类
    /// </summary>
    public partial class Weapon : CatchFish.Configs.Item
    {
        /// <summary>
        /// 每帧移动距离
        /// </summary>
        public float distance;
        /// <summary>
        /// 展示时长 ( 帧数 )
        /// </summary>
        public float showNumFrames;
        /// <summary>
        /// 飞到玩家坐标之后变化出来的炮台 cfg 之基类
        /// </summary>
        public CatchFish.Configs.Cannon cannon;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Weapon>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.distance);
            bb.Write(this.showNumFrames);
            bb.Write(this.cannon);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.Read(ref this.distance);
            bb.Read(ref this.showNumFrames);
            bb.Read(ref this.cannon);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Configs.Weapon\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"distance\":" + distance.ToString());
            s.Append(", \"showNumFrames\":" + showNumFrames.ToString());
            s.Append(", \"cannon\":" + (cannon == null ? "nil" : cannon.ToString()));
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
    /// 精灵帧
    /// </summary>
    public partial class SpriteFrame : xx.Object
    {
        /// <summary>
        /// plist资源名
        /// </summary>
        public string plistName;
        /// <summary>
        /// 帧名
        /// </summary>
        public string frameName;

        public override ushort GetPackageId()
        {
            return xx.TypeId<SpriteFrame>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.plistName);
            bb.Write(this.frameName);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.readLengthLimit = 0;
            bb.Read(ref this.plistName);
            bb.readLengthLimit = 0;
            bb.Read(ref this.frameName);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Configs.SpriteFrame\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            if (plistName != null) s.Append(", \"plistName\":\"" + plistName.ToString() + "\"");
            else s.Append(", \"plistName\":nil");
            if (frameName != null) s.Append(", \"frameName\":\"" + frameName.ToString() + "\"");
            else s.Append(", \"frameName\":nil");
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
    /// 物理建模 for 鱼与子弹碰撞检测
    /// </summary>
    public partial class Physics : xx.Object
    {
        /// <summary>
        /// 基于当前帧图的多边形碰撞顶点包围区( 由多个凸多边形组合而成, 用于物理建模碰撞判定 )
        /// </summary>
        public xx.List<xx.List<xx.Pos>> polygons;

        public override ushort GetPackageId()
        {
            return xx.TypeId<Physics>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.polygons);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.readLengthLimit = 0;
            bb.Read(ref this.polygons);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Configs.Physics\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"polygons\":" + (polygons == null ? "nil" : polygons.ToString()));
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
    /// 带物理检测区和锁定线等附加数据的鱼移动帧动画
    /// </summary>
    public partial class FishSpriteFrame : xx.Object
    {
        /// <summary>
        /// 指向精灵帧
        /// </summary>
        public CatchFish.Configs.SpriteFrame frame;
        /// <summary>
        /// 指向物理建模
        /// </summary>
        public CatchFish.Configs.Physics physics;
        /// <summary>
        /// 首选锁定点( 如果该点还在屏幕上, 则 lock 准星一直在其上 )
        /// </summary>
        public xx.Pos lockPoint;
        /// <summary>
        /// 锁定点集合( 串成一条线的锁定点. 当首选锁定点不在屏上时, 使用该线与所在屏的边线的交点作为锁定点 )
        /// </summary>
        public xx.List<xx.Pos> lockPoints;
        /// <summary>
        /// 本帧动画切到下一帧动画后应该移动的距离( 受 Fish.speedScale 影响 )
        /// </summary>
        public float moveDistance;

        public override ushort GetPackageId()
        {
            return xx.TypeId<FishSpriteFrame>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            bb.Write(this.frame);
            bb.Write(this.physics);
            ((xx.IObject)this.lockPoint).ToBBuffer(bb);
            bb.Write(this.lockPoints);
            bb.Write(this.moveDistance);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            bb.Read(ref this.frame);
            bb.Read(ref this.physics);
            ((xx.IObject)this.lockPoint).FromBBuffer(bb);
            bb.readLengthLimit = 0;
            bb.Read(ref this.lockPoints);
            bb.Read(ref this.moveDistance);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Configs.FishSpriteFrame\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            s.Append(", \"frame\":" + (frame == null ? "nil" : frame.ToString()));
            s.Append(", \"physics\":" + (physics == null ? "nil" : physics.ToString()));
            s.Append(", \"lockPoint\":" + lockPoint.ToString());
            s.Append(", \"lockPoints\":" + (lockPoints == null ? "nil" : lockPoints.ToString()));
            s.Append(", \"moveDistance\":" + moveDistance.ToString());
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
    /// 小鱼环绕的大鱼的特殊配置
    /// </summary>
    public partial class BigFish : CatchFish.Configs.Fish
    {
        /// <summary>
        /// 每帧移动距离
        /// </summary>
        public float moveFrameDistance;
        /// <summary>
        /// 小鱼只数
        /// </summary>
        public int numChilds;
        /// <summary>
        /// 小鱼前进角速度
        /// </summary>
        public float childsAngleInc;

        public override ushort GetPackageId()
        {
            return xx.TypeId<BigFish>.value;
        }

        public override void ToBBuffer(xx.BBuffer bb)
        {
            base.ToBBuffer(bb);
            bb.Write(this.moveFrameDistance);
            bb.Write(this.numChilds);
            bb.Write(this.childsAngleInc);
        }

        public override void FromBBuffer(xx.BBuffer bb)
        {
            base.FromBBuffer(bb);
            bb.Read(ref this.moveFrameDistance);
            bb.Read(ref this.numChilds);
            bb.Read(ref this.childsAngleInc);
        }
        public override void ToString(System.Text.StringBuilder s)
        {
            if (__toStringing)
            {
        	    s.Append("[ \"***** recursived *****\" ]");
        	    return;
            }
            else __toStringing = true;

            s.Append("{ \"pkgTypeName\":\"CatchFish.Configs.BigFish\", \"pkgTypeId\":" + GetPackageId());
            ToStringCore(s);
            s.Append(" }");

            __toStringing = false;
        }
        public override void ToStringCore(System.Text.StringBuilder s)
        {
            base.ToStringCore(s);
            s.Append(", \"moveFrameDistance\":" + moveFrameDistance.ToString());
            s.Append(", \"numChilds\":" + numChilds.ToString());
            s.Append(", \"childsAngleInc\":" + childsAngleInc.ToString());
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
    public static class AllTypes
    {
        public static void Register()
        {
            xx.Object.RegisterInternals();
            xx.Object.Register<Generic.Success>(3);
            xx.Object.Register<Generic.Error>(4);
            xx.Object.Register<Generic.Ping>(5);
            xx.Object.Register<Generic.Pong>(6);
            xx.Object.Register<CatchFish_Client.EnterSuccess>(7);
            xx.Object.Register<CatchFish.Scene>(8);
            xx.Object.Register<xx.List<CatchFish.Player>>(9);
            xx.Object.Register<CatchFish.Player>(10);
            xx.Object.Register<CatchFish_Client.FrameEvents>(11);
            xx.Object.Register<xx.List<CatchFish.Events.Event>>(12);
            xx.Object.Register<CatchFish.Events.Event>(13);
            xx.Object.Register<Client_CatchFish.Enter>(14);
            xx.Object.Register<Client_CatchFish.Bet>(87);
            xx.Object.Register<Client_CatchFish.Fire>(15);
            xx.Object.Register<Client_CatchFish.Hit>(16);
            xx.Object.Register<CatchFish_Calc.HitCheck>(88);
            xx.Object.Register<xx.List<Calc.CatchFish.Hit>>(89);
            xx.Object.Register<Calc_CatchFish.HitCheckResult>(90);
            xx.Object.Register<xx.List<Calc.CatchFish.Fish>>(91);
            xx.Object.Register<xx.List<Calc.CatchFish.Bullet>>(92);
            xx.Object.Register<Calc.Push>(93);
            xx.Object.Register<Calc.Pop>(94);
            xx.Object.Register<xx.Random>(17);
            xx.Object.Register<xx.List<CatchFish.Fish>>(18);
            xx.Object.Register<CatchFish.Fish>(19);
            xx.Object.Register<xx.List<CatchFish.Item>>(20);
            xx.Object.Register<CatchFish.Item>(21);
            xx.Object.Register<xx.List<CatchFish.FishBorn>>(22);
            xx.Object.Register<CatchFish.FishBorn>(23);
            xx.Object.Register<CatchFish.Stages.Stage>(24);
            xx.Object.Register<xx.List<CatchFish.Sits>>(25);
            xx.Object.Register<xx.List<xx.Weak<CatchFish.Player>>>(26);
            xx.Object.Register<xx.List<CatchFish.Cannon>>(27);
            xx.Object.Register<CatchFish.Cannon>(28);
            xx.Object.Register<xx.List<CatchFish.Weapon>>(29);
            xx.Object.Register<CatchFish.Weapon>(30);
            xx.Object.Register<xx.List<CatchFish.Bullet>>(31);
            xx.Object.Register<CatchFish.Bullet>(32);
            xx.Object.Register<CatchFish.MoveItem>(33);
            xx.Object.Register<CatchFish.Way>(34);
            xx.Object.Register<xx.List<CatchFish.WayPoint>>(36);
            xx.Object.Register<CatchFish.WayFish>(80);
            xx.Object.Register<CatchFish.RoundFish>(82);
            xx.Object.Register<CatchFish.BigFish>(83);
            xx.Object.Register<xx.List<CatchFish.RoundFish>>(84);
            xx.Object.Register<CatchFish.Events.Enter>(37);
            xx.Object.Register<CatchFish.Events.Leave>(38);
            xx.Object.Register<CatchFish.Events.NoMoney>(39);
            xx.Object.Register<CatchFish.Events.Refund>(40);
            xx.Object.Register<CatchFish.Events.FishDead>(41);
            xx.Object.Register<xx.List<CatchFish.Events.FishDead>>(42);
            xx.Object.Register<CatchFish.Events.PushWeapon>(43);
            xx.Object.Register<CatchFish.Events.PushFish>(44);
            xx.Object.Register<CatchFish.Events.OpenAutoLock>(45);
            xx.Object.Register<CatchFish.Events.Aim>(46);
            xx.Object.Register<CatchFish.Events.CloseAutoLock>(47);
            xx.Object.Register<CatchFish.Events.OpenAutoFire>(48);
            xx.Object.Register<CatchFish.Events.CloseAutoFire>(49);
            xx.Object.Register<CatchFish.Events.Fire>(50);
            xx.Object.Register<CatchFish.Events.CannonSwitch>(51);
            xx.Object.Register<CatchFish.Events.CannonCoinChange>(52);
            xx.Object.Register<CatchFish.Events.DebugInfo>(53);
            xx.Object.Register<xx.List<int>>(54);
            xx.Object.Register<xx.List<CatchFish.Stages.StageElement>>(74);
            xx.Object.Register<CatchFish.Stages.StageElement>(75);
            xx.Object.Register<CatchFish.Stages.Emitter_RandomFishs>(76);
            xx.Object.Register<CatchFish.Stages.Monitor_KeepFatFish>(77);
            xx.Object.Register<CatchFish.Stages.Monitor_KeepBigFish>(86);
            xx.Object.Register<CatchFish.Stages.Emitter_RingFishs>(78);
            xx.Object.Register<CatchFish.Stages.Emitter_CircleFishs>(81);
            xx.Object.Register<CatchFish.Configs.Config>(57);
            xx.Object.Register<xx.List<CatchFish.Way>>(58);
            xx.Object.Register<xx.List<CatchFish.Configs.Fish>>(59);
            xx.Object.Register<CatchFish.Configs.Fish>(60);
            xx.Object.Register<xx.List<CatchFish.Configs.Cannon>>(61);
            xx.Object.Register<CatchFish.Configs.Cannon>(62);
            xx.Object.Register<xx.List<CatchFish.Configs.Weapon>>(63);
            xx.Object.Register<CatchFish.Configs.Weapon>(64);
            xx.Object.Register<xx.List<CatchFish.Stages.Stage>>(65);
            xx.Object.Register<xx.List<xx.Pos>>(66);
            xx.Object.Register<CatchFish.Configs.Item>(67);
            xx.Object.Register<xx.List<CatchFish.Configs.SpriteFrame>>(68);
            xx.Object.Register<CatchFish.Configs.SpriteFrame>(69);
            xx.Object.Register<xx.List<CatchFish.Configs.FishSpriteFrame>>(70);
            xx.Object.Register<CatchFish.Configs.FishSpriteFrame>(71);
            xx.Object.Register<CatchFish.Configs.Physics>(72);
            xx.Object.Register<xx.List<xx.List<xx.Pos>>>(73);
            xx.Object.Register<CatchFish.Configs.BigFish>(85);
        }
    }
}
