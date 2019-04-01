using System;
using System.Text;

namespace xx
{
    public struct Pos : IObject
    {
        public float x, y;

        public ushort GetPackageId() { throw new NotImplementedException(); }
        public void FromBBuffer(BBuffer bb)
        {
            bb.Read(ref x);
            bb.Read(ref y);
        }
        public void ToBBuffer(BBuffer bb)
        {
            bb.Write(x);
            bb.Write(y);
        }

        public override string ToString()
        {
            var sb = new System.Text.StringBuilder();
            ToString(sb);
            return sb.ToString();
        }
        public void ToString(StringBuilder s)
        {
            s.Append("{ \"x\":" + x + ", \"y\":" + y + " }");
        }

        public void ToStringCore(StringBuilder sb) { }

        bool toStringFlag;
        public void SetToStringFlag(bool doing)
        {
            toStringFlag = doing;
        }
        public bool GetToStringFlag()
        {
            return toStringFlag;
        }
    }
}
