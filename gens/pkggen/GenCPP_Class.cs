using System;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;

public static class GenCPP_Class
{
    public static bool Gen(Assembly asm, string outDir, string templateName, string md5, TemplateLibrary.Filter<TemplateLibrary.CppFilter> filter = null, bool generateBak = false)
    {
        var ts = asm._GetTypes();
        var typeIds = new TemplateLibrary.TypeIds(asm);
        var sb = new StringBuilder();
        var sb2 = new StringBuilder();

        // template namespace
        sb.Append(@"#pragma once
#include ""xx_bbuffer.h""
namespace " + templateName + @" {
	struct PkgGenMd5 {
		inline static const std::string value = """ + md5 + @""";
    };
	struct AllTypesRegister {
        AllTypesRegister();
    };
    inline AllTypesRegister allTypesRegisterInstance;   // for auto register at program startup
");

        var cs = ts._GetClasssStructs();


#if true
        for (int i = 0; i < cs.Count; ++i)
        {
            var c = cs[i];
            if (filter != null && !filter.Contains(c)) continue;

            // namespace e_ns {
            if (c.Namespace != null && (i == 0 || (i > 0 && cs[i - 1].Namespace != c.Namespace))) // namespace 去重
            {
                sb.Append(@"
namespace " + c.Namespace.Replace(".", "::") + @" {");
            }

            // desc
            // enum class xxxxxxxxx : underlyingType
            sb.Append(c._GetDesc()._GetComment_Cpp(4) + @"
    " + (c.IsValueType ? "struct" : "struct") + @" " + c.Name + @";
    using " + c.Name + @"_s = std::shared_ptr<" + c.Name + @">;
    using " + c.Name + @"_w = std::weak_ptr<" + c.Name + @">;
");

            // namespace }
            if (c.Namespace != null && ((i < cs.Count - 1 && cs[i + 1].Namespace != c.Namespace) || i == cs.Count - 1))
            {
                sb.Append(@"
}");
            }
        }
#endif


        var es = ts._GetEnums();
        for (int i = 0; i < es.Count; ++i)
        {
            var e = es[i];
            if (filter != null && !filter.Contains(e)) continue;

            // namespace e_ns {
            if (e.Namespace != null && (i == 0 || (i > 0 && es[i - 1].Namespace != e.Namespace))) // namespace 去重
            {
                sb.Append(@"
namespace " + e.Namespace.Replace(".", "::") + @" {");
            }

            // desc
            // enum class xxxxxxxxx : underlyingType
            sb.Append(e._GetDesc()._GetComment_Cpp(4) + @"
    enum class " + e.Name + @" : " + e._GetEnumUnderlyingTypeName_Cpp() + @" {");

            // desc
            // xxxxxx = val
            var fs = e._GetEnumFields();
            foreach (var f in fs)
            {
                sb.Append(f._GetDesc()._GetComment_Cpp(8) + @"
        " + f.Name + " = " + f._GetEnumValue(e) + ",");
            }

            // enum /
            sb.Append(@"
    };");

            // namespace }
            if (e.Namespace != null && ((i < es.Count - 1 && es[i + 1].Namespace != e.Namespace) || i == es.Count - 1))
            {
                sb.Append(@"
}");
            }
        }

        var ss = ts._GetStructs();
        if (!generateBak)
        {
            ss._SortByInheritRelation();
        }
        else
        {
            ss._SortByFullName();
        }
        for (int i = 0; i < ss.Count; ++i)
        {
            var c = ss[i];
            if (filter != null && !filter.Contains(c)) continue;
            var o = asm.CreateInstance(c.FullName);

            // namespace e_ns {
            if (c.Namespace != null && (i == 0 || (i > 0 && ss[i - 1].Namespace != c.Namespace))) // namespace 去重
            {
                sb.Append(@"
namespace " + c.Namespace.Replace(".", "::") + @" {");
            }

            // desc
            // struct xxxxxxxxx
            sb.Append(c._GetDesc()._GetComment_Cpp(4) + @"
    struct " + c.Name + @" {");

            // desc
            // T xxxxxx = val
            // consts( static ) / fields
            var fs = c._GetFieldsConsts();
            foreach (var f in fs)
            {
                var ft = f.FieldType;
                var ftn = ft._GetTypeDecl_Cpp(templateName);
                sb.Append(f._GetDesc()._GetComment_Cpp(8) + @"
        " + (f.IsStatic ? "constexpr " : "") + ftn + " " + f.Name);

                var v = f.GetValue(f.IsStatic ? null : o);
                var dv = v._GetDefaultValueDecl_Cpp(templateName);
                if (dv != "" && !ft._IsList() && !ft._IsUserClass() && !ft._IsString() && !ft._IsObject() && !ft._IsWeak())  // 当前还无法正确处理 String 数据类型的默认值
                {
                    sb.Append(" = " + dv + ";");
                }
                else
                {
                    sb.Append(";");
                }
            }

            if (c._Has<TemplateLibrary.AttachInclude>())
            {
                sb.Append(@"
#include<" + c._GetTypeDecl_Lua(templateName) + @".inc>");
            }

            // struct /
            sb.Append(@"
    };");

            // namespace }
            if (c.Namespace != null && ((i < ss.Count - 1 && ss[i + 1].Namespace != c.Namespace) || i == ss.Count - 1))
            {
                sb.Append(@"
}");
            }
        }



        cs = ts._GetClasss();
        if (!generateBak)
        {
            cs._SortByInheritRelation();
        }
        else
        {
            cs._SortByFullName();
        }


        // 预声明
        for (int i = 0; i < cs.Count; ++i)
        {
            var c = cs[i];
            if (filter != null && !filter.Contains(c)) continue;
            var o = asm.CreateInstance(c.FullName);

            // namespace c_ns {
            if (c.Namespace != null && (i == 0 || (i > 0 && cs[i - 1].Namespace != c.Namespace))) // namespace 去重
            {
                sb.Append(@"
namespace " + c.Namespace.Replace(".", "::") + @" {");
            }

            // 定位到基类
            var bt = c.BaseType;
            var btn = c._HasBaseType() ? bt._GetTypeDecl_Cpp(templateName) : "xx::Object";

            // desc
            // T xxxxxxxxx = defaultValue
            // constexpr T xxxxxxxxx = defaultValue

            sb.Append(c._GetDesc()._GetComment_Cpp(4) + @"
    struct " + c.Name + @" : " + btn + @" {");

            // consts( static ) / fields
            var fs = c._GetFieldsConsts();
            foreach (var f in fs)
            {
                var ft = f.FieldType;
                var ftn = ft._GetTypeDecl_Cpp(templateName, "_s");
                sb.Append(f._GetDesc()._GetComment_Cpp(8) + @"
        " + (f.IsStatic ? "constexpr " : "") + ftn + " " + f.Name);

                if (ft._IsExternal() && !ft._GetExternalSerializable() && !string.IsNullOrEmpty(ft._GetExternalCppDefaultValue()))
                {
                    sb.Append(" = " + ft._GetExternalCppDefaultValue() + ";");
                }
                else
                {
                    var v = f.GetValue(f.IsStatic ? null : o);
                    var dv = v._GetDefaultValueDecl_Cpp(templateName);
                    if (dv != "" && !ft._IsList() && !(ft._IsUserClass()) && !ft._IsString() && !ft._IsNullable() && !ft._IsObject() && !ft._IsWeak())  // 当前还无法正确处理 String 数据类型的默认值
                    {
                        sb.Append(" = " + dv + ";");
                    }
                    else
                    {
                        sb.Append(";");
                    }
                }
            }

            var ms = c._GetMethods();
            foreach (var m in ms)
            {
                var ps = m.GetParameters();
                var rt = m.ReturnType;
                var rtn = rt._GetTypeDecl_Cpp(templateName, "_s");

                sb.Append(m._GetDesc()._GetComment_Cpp(8) + @"
        virtual " + rtn + " " + m.Name + "(");
                foreach (var p in ps)
                {
                    string attr = " ";
                    if (p._Has<TemplateLibrary.ConstRef>()) attr = " const& ";
                    if (p._Has<TemplateLibrary.PointerConstRef>()) attr = "* const& ";

                    sb.Append(p._GetDesc()._GetComment_Cpp(12) + @"
            " + (p != ps[0] ? ", " : "") + p.ParameterType._GetTypeDecl_Cpp(templateName) + attr + p.Name);
                }
                sb.Append(") noexcept;");
            }

#if false
            sb.Append(@"

        typedef " + c.Name + @" ThisType;
        typedef " + btn + @" BaseType;
	    " + c.Name + @"() = default;
		" + c.Name + @"(" + c.Name + @" const&) = delete;
		" + c.Name + @"& operator=(" + c.Name + @" const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;");

            if (c._Has<TemplateLibrary.CustomInitCascade>())
            {
                sb.Append(@"
        int InitCascadeCore(void* const& o = nullptr) noexcept;");
            }
            else
            {
                sb.Append(@"
        int InitCascade(void* const& o = nullptr) noexcept override;");
            }
#else
            if (c._Has<TemplateLibrary.CustomInitCascade>())
            {

                sb.Append(@"

        XX_CODEGEN_CLASS_HEADER_CASCADE_CORE(" + c.Name + ", " + btn + @")");
            }
            else
            {
                sb.Append(@"

        XX_CODEGEN_CLASS_HEADER_CASCADE(" + c.Name + ", " + btn + @")");
            }
#endif



            if (c._Has<TemplateLibrary.AttachInclude>())
            {
                sb.Append(@"
#include<" + c._GetTypeDecl_Lua(templateName) + @".inc>");
            }
            sb.Append(@"
    };");   // class }

            // namespace }
            if (c.Namespace != null && ((i < cs.Count - 1 && cs[i + 1].Namespace != c.Namespace) || i == cs.Count - 1))
            {
                sb.Append(@"
}");
            }
        }
        sb.Append(@"
}");

        // 结构体 Object 接口适配
        sb.Append(@"
namespace xx {");
        cs = ts._GetStructs();
        foreach (var c in cs)
        {
            if (filter != null && !filter.Contains(c)) continue;
            var ctn = c._GetTypeDecl_Cpp(templateName);
            var fs = c._GetFields();

            sb.Append(@"
	template<>
	struct BFuncs<" + ctn + @", void> {
		static inline void WriteTo(BBuffer& bb, " + ctn + @" const& in) noexcept {
			bb.Write(");
            foreach (var f in fs)
            {
                if (!f._Has<TemplateLibrary.NotSerialize>())
                {
                    sb.Append((f == fs.First() ? "" : @", ") + "in." + f.Name);
                }
            }
            sb.Append(@");
		}
		static inline int ReadFrom(BBuffer& bb, " + ctn + @"& out) noexcept {
			return bb.Read(");
            foreach (var f in fs)
            {
                sb.Append((f == fs.First() ? "" : @", ") + "out." + f.Name);
            }
            sb.Append(@");
		}
	};
	template<>
	struct SFuncs<" + ctn + @", void> {
		static inline void WriteTo(std::string& s, " + ctn + @" const& in) noexcept {
			xx::Append(s, ""{ \""structTypeName\"":\""" + (string.IsNullOrEmpty(c.Namespace) ? c.Name : c.Namespace + "." + c.Name) + @"\""""");
            foreach (var f in fs)
            {
                sb.Append(@", "", \""" + f.Name + @"\"":"", in." + f.Name);
            }
            sb.Append(@", "" }"");
        }
    };");

        }


        // 遍历所有 type 及成员数据类型 生成  BBuffer.Register< T >( typeId ) 函数组. 0 不能占. String 占掉 1. BBuffer 占掉 2. ( 这两个不生成 )
        // 在基础命名空间中造一个静态类 AllTypes 静态方法 Register

        foreach (var kv in typeIds.types)
        {
            if (filter != null && !filter.Contains(kv.Key)) continue;
            var ct = kv.Key;
            if (ct._IsString() || ct._IsBBuffer() || ct._IsExternal() && !ct._GetExternalSerializable()) continue;
            var typeId = kv.Value;
            var ctn = ct._GetTypeDecl_Cpp(templateName);

            sb.Append(@"
    template<> struct TypeId<" + ctn + @"> { static const uint16_t value = " + typeId + @"; };");
        }

        sb.Append(@"
}
");
































        // defines ( cpp )

        sb2.Append(@"#include """ + templateName + "_class" + (filter != null ? "_filter" : "") + ".h" + @"""
namespace " + templateName + @" {");

        cs = ts._GetClasss();   //._SortByInheritRelation();
        // 实现
        for (int i = 0; i < cs.Count; ++i)
        {
            var c = cs[i];
            if (filter != null && !filter.Contains(c)) continue;

            // namespace c_ns {
            if (c.Namespace != null && (i == 0 || (i > 0 && cs[i - 1].Namespace != c.Namespace))) // namespace 去重
            {
                sb2.Append(@"
namespace " + c.Namespace.Replace(".", "::") + @" {");
            }

            var ms = c._GetMethods();
            foreach (var m in ms)
            {
                var ps = m.GetParameters();
                var rt = m.ReturnType;
                var rtn = rt._GetTypeDecl_Cpp(templateName, "_s");

                sb2.Append(@"
    " + rtn + " " + c.Name + "::" + m.Name + "(");
                foreach (var p in ps)
                {
                    string attr = " ";
                    if (p._Has<TemplateLibrary.ConstRef>()) attr = " const& ";
                    if (p._Has<TemplateLibrary.PointerConstRef>()) attr = "* const& ";

                    sb2.Append(p._GetDesc()._GetComment_Cpp(12) + @"
            " + (p != ps[0] ? ", " : "") + p.ParameterType._GetTypeDecl_Cpp(templateName) + attr + p.Name);
                }
                sb2.Append(") noexcept {" + (rtn != "void" ? (" return " + rtn + "(); ") : "") + "}");
            }


            // 定位到基类
            var bt = c.BaseType;
            var fs = c._GetFields();

            sb2.Append(@"
    uint16_t " + c.Name + @"::GetTypeId() const noexcept {
        return " + typeIds.types[c] + @";
    }
    void " + c.Name + @"::ToBBuffer(xx::BBuffer& bb) const noexcept {");

            if (c._HasBaseType())
            {
                sb2.Append(@"
        this->BaseType::ToBBuffer(bb);");
            }
            fs = c._GetFields();
            foreach (var f in fs)
            {
                var ft = f.FieldType;
                if (ft._IsExternal() && !ft._GetExternalSerializable()) continue;
                if (f._Has<TemplateLibrary.NotSerialize>())
                {
                    //            sb2.Append(@"
                    //bb.WriteDefaultValue<" + ft._GetTypeDecl_Cpp(templateName, "_s") + ">();");
                }
                else if (f._Has<TemplateLibrary.CustomSerialize>())
                {
                    //            sb2.Append(@"
                    //bb.CustomWrite(bb, (void*)this, _offsetof(ThisType, " + f.Name + "));");
                    throw new NotImplementedException();
                }
                else
                {
                    sb2.Append(@"
        bb.Write(this->" + f.Name + ");");
                }
            }

            sb2.Append(@"
    }
    int " + c.Name + @"::FromBBuffer(xx::BBuffer& bb) noexcept {");
            if (c._HasBaseType())
            {
                sb2.Append(@"
        if (int r = this->BaseType::FromBBuffer(bb)) return r;");
            }
            fs = c._GetFields();
            foreach (var f in fs)
            {
                if (f.FieldType._IsExternal() && !f.FieldType._GetExternalSerializable()) continue;
                if (f.FieldType._IsContainer())
                {
                    sb2.Append(@"
        bb.readLengthLimit = " + f._GetLimit() + ";");
                }

                sb2.Append(@"
        if (int r = bb.Read(this->" + f.Name + @")) return r;");
            }
            sb2.Append(@"
        return 0;
    }");
            if (c._Has<TemplateLibrary.CustomInitCascade>())
            {
                sb2.Append(@"
    int " + c.Name + @"::InitCascadeCore(void* const& o) noexcept {");
            }
            else
            {
                sb2.Append(@"
    int " + c.Name + @"::InitCascade(void* const& o) noexcept {");
            }
            if (c._HasBaseType())
            {
                sb2.Append(@"
        if (int r = this->BaseType::InitCascade(o)) return r;");
            }
            fs = c._GetFields();
            foreach (var f in fs)
            {
                var ft = f.FieldType;
                if (!ft._IsList() && !ft._IsUserClass() || ft._IsWeak() || ft._IsExternal() && !ft._GetExternalSerializable()) continue;
                sb2.Append(@"
        if (this->" + f.Name + @") {
            if (int r = this->" + f.Name + @"->InitCascade(o)) return r;
        }");
            }
            sb2.Append(@"
        return 0;
    }");
            sb2.Append(@"
    void " + c.Name + @"::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, ""[ \""***** recursived *****\"" ]"");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, ""{ \""pkgTypeName\"":\""" + (string.IsNullOrEmpty(c.Namespace) ? c.Name : c.Namespace + "." + c.Name) + @"\"", \""pkgTypeId\"":"", GetTypeId());
        ToStringCore(s);
        xx::Append(s, "" }"");
        
        this->SetToStringFlag(false);
    }
    void " + c.Name + @"::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);");
            foreach (var f in fs)
            {
                if (f.FieldType._IsExternal() && !f.FieldType._GetExternalSerializable()) continue;
                if (f.FieldType._IsString())
                {
                    sb2.Append(@"
        if (this->" + f.Name + @") xx::Append(s, "", \""" + f.Name + @"\"":\"""", this->" + f.Name + @", ""\"""");
        else xx::Append(s, "", \""" + f.Name + @"\"":nil"");");
                }
                else
                {
                    sb2.Append(@"
        xx::Append(s, "", \""" + f.Name + @"\"":"", this->" + f.Name + @");");
                }
            }
            sb2.Append(@"
    }");

            // namespace }
            if (c.Namespace != null && ((i < cs.Count - 1 && cs[i + 1].Namespace != c.Namespace) || i == cs.Count - 1))
            {
                sb2.Append(@"
}");
            }

        }

        sb2.Append(@"
}");


        sb2.Append(@"
namespace " + templateName + @" {
	AllTypesRegister::AllTypesRegister() {");
        foreach (var kv in typeIds.types)
        {
            var ct = kv.Key;
            if (filter != null && !filter.Contains(ct)) continue;
            if (ct._IsString() || ct._IsBBuffer() || ct._IsExternal() && !ct._GetExternalSerializable()) continue;
            var ctn = ct._GetTypeDecl_Cpp(templateName);
            var bt = ct.BaseType;
            var btn = ct._HasBaseType() ? bt._GetTypeDecl_Cpp(templateName) : "xx::Object";

            sb2.Append(@"
	    xx::BBuffer::Register<" + ctn + @">(" + kv.Value + @");");
        }
        sb2.Append(@"
	}
}
");

        // 以命名空间排序版生成备份文件 用以对比是否发生改变. 如果.bak 经过生成对比发现无差异，就不必再生成了
        if (generateBak)
        {
            var sb3 = new StringBuilder();
            sb3.Append(sb);
            sb3.Append(sb2);
            return sb3._WriteToFile(Path.Combine(outDir, templateName + "_class" + (filter != null ? "_filter" : "") + ".h.bak"));
        }
        else if (!Gen(asm, outDir, templateName, "", filter, true))
        {
            return false;
        }

        // 写文件。如果无变化就退出。后面的都不必做了
        // 追加子包含文件
        foreach (var c in cs)
        {
            if (c._Has<TemplateLibrary.AttachInclude>())
            {
                sb2.Append(@"#include <" + c._GetTypeDecl_Lua(templateName) + @".hpp>
");
            }
        }
        sb._WriteToFile(Path.Combine(outDir, templateName + "_class" + (filter != null ? "_filter" : "") + ".h"));
        sb2._WriteToFile(Path.Combine(outDir, templateName + "_class" + (filter != null ? "_filter" : "") + ".cpp"));

        sb.Clear();
        foreach (var c in cs)
        {
            if (c._Has<TemplateLibrary.AttachInclude>())
            {
                sb._WriteToFile(Path.Combine(outDir, c._GetTypeDecl_Lua(templateName) + ".inc"));
                sb._WriteToFile(Path.Combine(outDir, c._GetTypeDecl_Lua(templateName) + ".hpp"));
            }
        }

        return true;
    }
}
