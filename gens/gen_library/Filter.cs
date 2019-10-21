﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;

namespace TemplateLibrary
{
    // 白名单
    public class Filter<T> where T : System.Attribute
    {
        HashSet<Type> depends = new HashSet<Type>();
        System.Collections.Generic.List<Type> ts = new System.Collections.Generic.List<Type>();

        public Filter(Assembly asm)
        {
            // 扫出命名空间白名单
            var nss = new HashSet<string>();
            var types = asm._GetTypes();
            var ifaces = types._GetInterfaces<T>();
            foreach (var iface in ifaces)
            {
                var lfs = iface.GetCustomAttributes<T>();
                foreach (var lf in lfs)
                {
                    nss.Add(lf.ToString());
                }
            }
            if (nss.Count == 0) return;

            // 记录　命名空间白名单　下所有类
            foreach (var t in types)
            {
                var ns = t.Namespace;
                if (ns != null && ns.Contains('.')) ns = ns.Split('.')[0];
                if (nss.Contains(ns))
                {
                    depends.Add(t);
                    this.ts.Add(t);
                }
            }

            // 使用游标依次扫描 ts, 将新出现的 基类, 成员类型 添加到 ts 最后. 直到扫光
            for (int i = 0; i < ts.Count; ++i)
            {
                var t = ts[i];
                if (t._IsList()) Push(t.GenericTypeArguments[0]);
                else
                {
                    if (t._HasBaseType()) Push(t.BaseType);
                    foreach (var f in t._GetFieldsConsts()) Push(f.FieldType);
                }
            }
        }

        bool Push(Type t)
        {
            if (!depends.Add(t)) return false;
            ts.Add(t);
            return true;
        }

        // 判断一个类型是否位于白名单之中( 为简化起见，　系统原生类型也算位于白名单之中 )
        public bool Contains(Type t)
        {
            if (depends.Count == 0) return true;
            if (t.Namespace == nameof(System)) return true;
            return depends.Contains(t);
        }
    }
}
