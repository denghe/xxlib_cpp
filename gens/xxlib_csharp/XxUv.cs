// todo: 翻抄 c++ 最新版 uv 库的 rpc 部分结构?

using System;
using System.Runtime.InteropServices;
#if NET_2_0 || NET_2_0_SUBSET
#else
using System.Collections.Concurrent;
#endif
using System.Text;

namespace xx
{
    public enum UvRunMode
    {
        Default = 0,
        Once,
        NoWait
    }

    public enum UvTcpStates
    {
        Disconnected,
        Connecting,
        Connected,
        Disconnecting,
    };

    public class UvLoop : IDisposable
    {
        public bool disposed;
        public List<UvTcpListener> tcpListeners = new List<UvTcpListener>();
        public List<UvTcpClient> tcpClients = new List<UvTcpClient>();
        public List<UvTimer> timers = new List<UvTimer>();
        public List<UvAsync> asyncs = new List<UvAsync>();
        public UvTimeoutManager timeoutManager;
        public UvRpcManager rpcManager;

        public IntPtr ptr;
        public GCHandle handle;
        public IntPtr handlePtr;

        public UvLoop()
        {
            try
            {
                this.Handle(ref handle, ref handlePtr);
                ptr = UvInterop.xxuv_alloc_uv_loop_t(handlePtr);
                if (ptr == IntPtr.Zero)
                {
                    this.Unhandle(ref handle, ref handlePtr);
                    throw new NullReferenceException();
                }

                int r = UvInterop.xxuv_loop_init(ptr);
                if (r != 0)
                {
                    this.Free(ref ptr);
                    this.Unhandle(ref handle, ref handlePtr);
                    r.Throw();
                }
            }
            catch (Exception ex)
            {
                disposed = true;
                throw ex;
            }
        }

        public void Run(UvRunMode mode = UvRunMode.Default)
        {
            if (disposed) throw new ObjectDisposedException("XxUvLoop");
            int r = UvInterop.xxuv_run(ptr, mode);
            if (r != 0 && r != 1) r.Throw();    // 1: uv_stop( loop )
        }

        public void Stop()
        {
            if (disposed) throw new ObjectDisposedException("XxUvLoop");
            UvInterop.xxuv_stop(ptr);
        }

        public bool alive
        {
            get
            {
                if (disposed) throw new ObjectDisposedException("XxUvLoop");
                return UvInterop.xxuv_loop_alive(ptr) != 0;
            }
        }

        public void InitTimeoutManager(ulong intervalMS = 1000, int wheelLen = 6, int defaultInterval = 5)
        {
            if (timeoutManager != null) throw new InvalidOperationException();
            timeoutManager = new UvTimeoutManager(this, intervalMS, wheelLen, defaultInterval);
        }

        public void InitRpcManager(ulong intervalMS = 1000, int defaultInterval = 5)
        {
            if (rpcManager != null) throw new InvalidOperationException();
            rpcManager = new UvRpcManager(this, intervalMS, defaultInterval);
        }

        #region Dispose

        public void Dispose()
        {
            if (disposed) return;
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposed)
            {
                // if (disposing) // Free other state (managed objects).

                tcpClients.ForEachReverse(o => o.Dispose());
                tcpListeners.ForEachReverse(o => o.Dispose());
                timers.ForEachReverse(o => o.Dispose());
                asyncs.ForEachReverse(o => o.Dispose());

                if (UvInterop.xxuv_loop_close(ptr) != 0)                    // busy
                {
                    UvInterop.xxuv_run(ptr, UvRunMode.Default).TryThrow();  // success
                    UvInterop.xxuv_loop_close(ptr).TryThrow();              // success
                }
                this.Free(ref ptr);
                this.Unhandle(ref handle, ref handlePtr);
                disposed = true;
            }
            //base.Dispose(disposing);
        }

        ~UvLoop()
        {
            Dispose(false);
        }

        #endregion
    }

    public class UvTcpListener : IDisposable
    {
        /******************************************************************************/
        // 用户事件绑定区
        public Func<UvTcpPeer> OnCreatePeer;
        public Action<UvTcpPeer> OnAccept;
        public Action OnDispose;
        /******************************************************************************/

        public bool disposed;
        public UvLoop loop;
        public List<UvTcpPeer> peers = new List<UvTcpPeer>();
        public int index_at_container;

        public IntPtr ptr;
        public GCHandle handle;
        public IntPtr handlePtr;
        public IntPtr addrPtr;

        public UvTcpListener(UvLoop loop)
        {
            try
            {
                this.Handle(ref handle, ref handlePtr);
                this.loop = loop;

                ptr = UvInterop.xxuv_alloc_uv_tcp_t(handlePtr);
                if (ptr == IntPtr.Zero)
                {
                    this.Unhandle(ref handle, ref handlePtr);
                    throw new OutOfMemoryException();
                }

                int r = UvInterop.xxuv_tcp_init(loop.ptr, ptr);
                if (r != 0)
                {
                    this.Free(ref ptr);
                    this.Unhandle(ref handle, ref handlePtr);
                    r.Throw();
                }

                addrPtr = UvInterop.xxuv_alloc_sockaddr_in(IntPtr.Zero);
                if (addrPtr == IntPtr.Zero)
                {
                    UvInterop.xxuv_close_(ptr);
                    ptr = IntPtr.Zero;
                    this.Unhandle(ref handle, ref handlePtr);
                    throw new OutOfMemoryException();
                }

                index_at_container = loop.tcpListeners.dataLen;
                loop.tcpListeners.Add(this);
            }
            catch (Exception ex)
            {
                disposed = true;
                throw ex;
            }
        }


        static UvInterop.uv_connection_cb OnAcceptCB = OnAcceptCBImpl;
#if NET_2_0 || NET_2_0_SUBSET
        [AOT.MonoPInvokeCallback(typeof(UvInterop.uv_connection_cb))]
#endif
        static void OnAcceptCBImpl(IntPtr stream, int status)
        {
            if (status != 0) return;
            var listener = stream.To<UvTcpListener>();
            UvTcpPeer peer = null;
            try
            {
                if (listener.OnCreatePeer != null) peer = listener.OnCreatePeer();
                else peer = new UvTcpPeer(listener);
            }
            catch
            {
                return;
            }
            if (listener.OnAccept != null) listener.OnAccept(peer);
        }
        public void Listen(int backlog = 128)
        {
            if (disposed) throw new ObjectDisposedException("XxUvTcpListener");
            UvInterop.xxuv_listen(ptr, backlog, OnAcceptCB).TryThrow();
        }


        public void Bind(string ip, int port)
        {
            if (disposed) throw new ObjectDisposedException("XxUvTcpListener");
            if (ip.Contains(":"))
            {
                UvInterop.xxuv_ip6_addr(ip, port, addrPtr).TryThrow();
            }
            else
            {
                UvInterop.xxuv_ip4_addr(ip, port, addrPtr).TryThrow();
            }
            UvInterop.xxuv_tcp_bind_(ptr, addrPtr).TryThrow();
        }

        #region Dispose

        public void Dispose()
        {
            if (disposed) return;
            if (OnDispose != null) OnDispose();
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposed)
            {
                // if (disposing) // Free other state (managed objects).

                peers.ForEachReverse(o => o.Dispose());

                UvInterop.xxuv_close_(ptr);
                ptr = IntPtr.Zero;
                this.Free(ref addrPtr);
                this.Unhandle(ref handle, ref handlePtr);

                loop.tcpListeners[loop.tcpListeners.dataLen - 1].index_at_container = index_at_container;
                loop.tcpListeners.SwapRemoveAt(index_at_container);
                loop = null;
                disposed = true;
            }
            //base.Dispose(disposing);
        }

        ~UvTcpListener()
        {
            Dispose(false);
        }

        #endregion
    }

    public abstract class UvTimeouterBase
    {
        // TimerManager 于 Add 时填充下列成员
        public UvTimeoutManager timeouterManager;        // 指向时间管理( 初始为空 )
        public UvTimeouterBase timeouterPrev;       // 指向同一 ticks 下的上一 timer
        public UvTimeouterBase timeouterNext;       // 指向同一 ticks 下的下一 timer
        public int timeouterIndex = -1;             // 位于管理器 timeouterss 数组的下标
        public Action OnTimeout;                    // 时间到达后要执行的函数

        public void TimeouterClear()
        {
            timeouterPrev = null;
            timeouterNext = null;
            timeouterIndex = -1;
        }

        public void TimeoutReset(int interval = 0)
        {
            if (timeouterManager == null) throw new InvalidOperationException();
            timeouterManager.AddOrUpdate(this, interval);
        }
        public void TimeouterStop()
        {
            if (timeouterManager == null) throw new InvalidOperationException();
            if (timeouting) timeouterManager.Remove(this);
        }

        public virtual void BindTimeoutManager(UvTimeoutManager tm)
        {
            if (timeouterManager != null) throw new InvalidOperationException();
            timeouterManager = tm;
        }

        public void UnbindTimeoutManager()
        {
            if (timeouting) timeouterManager.Remove(this);
            timeouterManager = null;
        }

        public bool timeouting { get { return timeouterManager != null && (timeouterIndex != -1 || timeouterPrev != null); } }
    }

    public abstract class UvTcpUdpBase : UvTimeouterBase, IDisposable
    {
        /******************************************************************************/
        // 用户事件绑定

        public Action<BBuffer> OnReceivePackage;

        // int: 流水号
        public Action<int, BBuffer> OnReceiveRequest;

        // 4个 int 代表 包起始offset, 含包头的总包长, 地址起始偏移, 地址长度( 方便替换地址并 memcpy )
        // BBuffer 的 offset 停在数据区起始位置
        public Action<BBuffer, int, int, int, int> OnReceiveRoutingPackage;

        public Action OnDispose;

        /******************************************************************************/

        public bool disposed;
        public UvLoop loop;
        public int index_at_container;

        public IntPtr ptr;
        public GCHandle handle;
        public IntPtr handlePtr;
        public IntPtr addrPtr;

        protected BBuffer bbRecv = new BBuffer();               // 复用的接收缓冲区
        protected BBuffer bbSend = new BBuffer();               // 复用

        // 用来放 serial 以便断线时及时发起 Request 超时回调
        protected System.Collections.Generic.HashSet<int> rpcSerials;

        public override void BindTimeoutManager(UvTimeoutManager tm = null)
        {
            if (timeouterManager != null) throw new InvalidOperationException();
            timeouterManager = tm == null ? loop.timeoutManager : tm;
        }


        public abstract bool Disconnected();
        protected abstract void DisconnectImpl();

        // 取没发出去的数据队列长度( 便于逻辑上及时 Dispose, 避免目标无接收能力而堆积数据吃内存 )
        public abstract int GetSendQueueSize();

        // 原始数据发送
        public abstract void SendBytes(byte[] data, int offset = 0, int len = 0);


        // 原始数据发送之 BBuffer 版( 发送 BBuffer 中的数据 )
        public void SendBytes(BBuffer bb)
        {
            SendBytes(bb.buf, 0, bb.dataLen);
        }

        // 包头 = 4字节数据长
        // 数据 = serial + data
        // 基础收数据处理, 投递到事件函数
        public virtual void ReceiveImpl(IntPtr bufPtr, int len)
        {
            bbRecv.WriteBuf(bufPtr, len);                   // 追加收到的数据到接收缓冲区

            var buf = bbRecv.buf;                           // 方便使用
            int offset = 0;                                 // 提速

            while (offset + 4 <= bbRecv.dataLen)           // ensure header len( 4 bytes )
            {
                var dataLen = buf[offset + 1] + (buf[offset + 2] << 8) + (buf[offset + 3] << 16) + (buf[offset + 4] << 24);
                if (dataLen <= 0 /* || len > maxLimit */)
                {
                    DisconnectImpl();
                    return;
                }
                if (offset + 4 + dataLen > bbRecv.dataLen) break;   // 确保数据长

                offset += 4;
                {
                    bbRecv.offset = offset;
                    int serial = 0;
                    if (!bbRecv.TryRead(ref serial))
                    {
                        DisconnectImpl();
                        return;
                    }

                    if (serial == 0)    // recv push
                    {
                        if (OnReceivePackage != null) OnReceivePackage(bbRecv);
                    }
                    else if (serial < 0)    // recv request
                    {
                        if (OnReceiveRequest != null) OnReceiveRequest(-serial, bbRecv);
                    }
                    else    // recv response
                    {
                        loop.rpcManager.Callback(serial, bbRecv);
                    }
                    
                    if (disposed || bbRecv.dataLen == 0) return;    // alive check at callback
                    if (Disconnected())
                    {
                        bbRecv.Clear();
                        return;
                    }
                }
                offset += dataLen;
            }
            if (offset < bbRecv.dataLen)                    // 还有剩余的数据: 移到最前面
            {
                Buffer.BlockCopy(buf, offset, buf, 0, bbRecv.dataLen - offset);
            }
            bbRecv.dataLen -= offset;
        }


        // 发包( serial == 0: 推送, < 0: 请求, > 0: 回应 )
        public void Send(xx.IObject pkg, int serial = 0)
        {
            if (disposed) throw new ObjectDisposedException("XxUvTcpBase");
            bbSend.Clear();
            bbSend.Reserve(1024);
            bbSend.dataLen = 4;
            bbSend.Write(serial);
            bbSend.WriteRoot(pkg);
            var p = bbSend.buf;
            var dataLen = bbSend.dataLen - 4;
            p[0] = (byte)dataLen;
            p[1] = (byte)(dataLen >> 8);
            p[2] = (byte)(dataLen >> 16);
            p[3] = (byte)(dataLen >> 24);
            SendBytes(p, 0, dataLen + 4);
        }

        // 发推送
        public void SendPush(xx.IObject pkg)
        {
            Send(pkg);
        }

        // 发请求
        public void SendRequest(xx.IObject pkg, Action<IObject> cb, int interval = 0)
        {
            if (disposed) throw new ObjectDisposedException("XxUvTcpBase");
            if (loop.rpcManager == null) throw new NullReferenceException("forget InitRpcManager ?");

            var serial = loop.rpcManager.Register((int ser, BBuffer bb) =>
            {
                if (rpcSerials != null) rpcSerials.Remove(ser);
                xx.IObject inPkg = null;   // 如果 超时或 read 异常, inPkg 会保持空值
                if (bb != null)
                {
                    inPkg = bb.TryReadRoot<xx.IObject>();
                }
                cb(inPkg); // call 原始 lambda
            }, interval);
            Send(pkg, -serial);
            if (rpcSerials == null)
            {
                rpcSerials = new System.Collections.Generic.HashSet<int>();
            }
            rpcSerials.Add(serial);
        }

        // 发应答
        public void SendResponse(int serial, xx.IObject pkg)
        {
            if (disposed) throw new ObjectDisposedException("XxUvTcpBase");
            Send(pkg, serial);
        }

        // 超时回调所有被跟踪 rpc 流水号并清空( 内部函数. 会自动在 OnDispose, OnDisconnect 事件前调用以触发超时回调 )
        public void RpcTraceCallback()
        {
            if (rpcSerials != null)
            {
                // 将就用 .net 的 HashSet, 免得导致 版本变化出 except
                var serials = new int[rpcSerials.Count];
                rpcSerials.CopyTo(serials);
                foreach (var serial in serials)
                {
                    loop.rpcManager.Callback(serial, null);
                    break;
                }
                serials = null;
            }
        }

        public void DelayRelease(int interval = 0)
        {
            OnReceivePackage = null;
            OnReceiveRequest = null;
            //OnReceiveRouting = null;
            OnDispose = null;
            if (timeouterManager == null)
            {
                BindTimeoutManager();
            }
            OnTimeout = Dispose;
            TimeoutReset(interval);
        }


        public abstract void Dispose();
    }

    public abstract class UvTcpBase : UvTcpUdpBase
    {
        public override bool Disconnected()
        {
            return ptr == IntPtr.Zero;
        }

        protected static UvInterop.uv_read_cb OnReadCB = OnReadCBImpl;
#if NET_2_0 || NET_2_0_SUBSET
        [AOT.MonoPInvokeCallback(typeof(UvInterop.uv_connection_cb))]
#endif
        static void OnReadCBImpl(IntPtr stream, IntPtr nread, IntPtr buf_t)
        {
            var tcp = stream.To<UvTcpBase>();
            var loopPtr = tcp.loop.ptr;        // 防事件 Dispose 先取出来
            var bufPtr = UvInterop.xxuv_get_buf(buf_t);
            int len = (int)nread;
            if (len > 0)
            {
                tcp.ReceiveImpl(bufPtr, len);
            }
            UvInterop.xxuv_free(bufPtr);
            if (len < 0)
            {
                tcp.DisconnectImpl();
            }
        }

        public override void SendBytes(byte[] data, int offset = 0, int len = 0)
        {
            if (disposed) throw new ObjectDisposedException("XxUvTcpBase");
            if (data == null || data.Length == 0) throw new NullReferenceException();
            if (offset + len > data.Length) throw new IndexOutOfRangeException();
            if (data.Length == offset) throw new NullReferenceException();
            if (len == 0) len = data.Length - offset;
            var h = GCHandle.Alloc(data, GCHandleType.Pinned);
            UvInterop.xxuv_write_(ptr, h.AddrOfPinnedObject(), (uint)offset, (uint)len).TryThrow();
            h.Free();
        }

        // 取待发送队列长度
        // 如果遇到发送量较大的场景, 应该抽空做这个检查, 及时 Dispose, 避免目标无接收能力而堆积数据吃内存.
        public override int GetSendQueueSize()
        {
            if (disposed) throw new ObjectDisposedException("XxUvTcpBase");
            return (int)UvInterop.xxuv_stream_get_write_queue_size(ptr);
        }
    }

    public class UvTcpPeer : UvTcpBase, IDisposable
    {
        public bool alive { get { return !disposed; } }

        public UvTcpListener listener;
        public UvTcpPeer(UvTcpListener listener)
        {
            try
            {
                this.Handle(ref handle, ref handlePtr);
                this.listener = listener;
                this.loop = listener.loop;

                ptr = UvInterop.xxuv_alloc_uv_tcp_t(handlePtr);
                if (ptr == IntPtr.Zero)
                {
                    this.Unhandle(ref handle, ref handlePtr);
                    throw new OutOfMemoryException();
                }

                int r = UvInterop.xxuv_tcp_init(loop.ptr, ptr);
                if (r != 0)
                {
                    this.Free(ref ptr);
                    this.Unhandle(ref handle, ref handlePtr);
                    r.Throw();
                }

                r = UvInterop.xxuv_accept(listener.ptr, ptr);
                if (r != 0)
                {
                    UvInterop.xxuv_close_(ptr);
                    ptr = IntPtr.Zero;
                    this.Unhandle(ref handle, ref handlePtr);
                    r.Throw();
                }

                r = UvInterop.xxuv_read_start_(ptr, OnReadCB);
                if (r != 0)
                {
                    UvInterop.xxuv_close_(ptr);
                    ptr = IntPtr.Zero;
                    this.Unhandle(ref handle, ref handlePtr);
                    r.Throw();
                }

                addrPtr = UvInterop.xxuv_alloc_sockaddr_in(IntPtr.Zero);
                if (addrPtr == IntPtr.Zero)
                {
                    UvInterop.xxuv_close_(ptr);
                    ptr = IntPtr.Zero;
                    this.Unhandle(ref handle, ref handlePtr);
                    throw new OutOfMemoryException();
                }

                index_at_container = listener.peers.dataLen;
                listener.peers.Add(this);
            }
            catch (Exception ex)
            {
                disposed = true;
                throw ex;
            }
        }

        protected override void DisconnectImpl()
        {
            Dispose();
        }

        byte[] ipBuf = new byte[64];
        string ip_ = null;
        public string ip
        {
            get
            {
                if (disposed) throw new ObjectDisposedException("XxUvTcpPeer");
                if (ip_ != null) return ip_;
                int len = 0;
                var h = GCHandle.Alloc(ipBuf, GCHandleType.Pinned);
                try
                {
                    UvInterop.xxuv_fill_client_ip(ptr, h.AddrOfPinnedObject(), ipBuf.Length, ref len).TryThrow();
                    ip_ = Encoding.ASCII.GetString(ipBuf, 0, len);
                    return ip_;
                }
                finally
                {
                    h.Free();
                }
            }
        }

        #region Dispose

        bool disposing = false; // 防递归
        public override void Dispose()
        {
            if (disposing || disposed) return;
            disposing = true;
            RpcTraceCallback();
            if (OnDispose != null) OnDispose();
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposed)
            {
                // if (disposing) // Free other state (managed objects).

                UvInterop.xxuv_close_(ptr);
                ptr = IntPtr.Zero;
                this.Free(ref addrPtr);
                this.Unhandle(ref handle, ref handlePtr);

                UnbindTimeoutManager();
                OnTimeout = null;

                bbSend = null;
                bbRecv = null;
                listener.peers[listener.peers.dataLen - 1].index_at_container = index_at_container;
                listener.peers.SwapRemoveAt(index_at_container);
                listener = null;
                loop = null;
                disposed = true;
            }
            //base.Dispose(disposing);
        }

        ~UvTcpPeer()
        {
            Dispose(false);
        }

        #endregion
    }

    public class UvTcpClient : UvTcpBase, IDisposable
    {
        /******************************************************************************/
        // 用户事件绑定
        public Action<int> OnConnect;
        public Action OnDisconnect;
        /******************************************************************************/

        public UvTcpStates state;
        public bool alive { get { return !disposed && state == UvTcpStates.Connected; } }

        public UvTcpClient(UvLoop loop)
        {
            try
            {
                this.Handle(ref handle, ref handlePtr);
                this.loop = loop;

                addrPtr = UvInterop.xxuv_alloc_sockaddr_in(IntPtr.Zero);
                if (addrPtr == IntPtr.Zero)
                {
                    this.Unhandle(ref handle, ref handlePtr);
                    throw new OutOfMemoryException();
                }

                index_at_container = loop.tcpClients.dataLen;
                loop.tcpClients.Add(this);
            }
            catch (Exception ex)
            {
                disposed = true;
                throw ex;
            }
        }

        public void SetAddress(string ip, int port)
        {
            if (disposed) throw new ObjectDisposedException("XxUvTcpClient");
            if (ip.Contains(":"))
            {
                UvInterop.xxuv_ip6_addr(ip, port, addrPtr).TryThrow();
            }
            else
            {
                UvInterop.xxuv_ip4_addr(ip, port, addrPtr).TryThrow();
            }
        }

        static UvInterop.uv_connect_cb OnConnectCB = OnConnectCBImpl;
#if NET_2_0 || NET_2_0_SUBSET
        [AOT.MonoPInvokeCallback(typeof(UvInterop.uv_connection_cb))]
#endif
        static void OnConnectCBImpl(IntPtr req, int status)
        {
            var client = (UvTcpClient)((GCHandle)UvInterop.xxuv_get_ud_from_uv_connect_t(req)).Target;
            UvInterop.xxuv_free(req);
            if (client == null) return;
            if (status < 0)
            {
                client.Disconnect();
            }
            else
            {
                client.state = UvTcpStates.Connected;
                UvInterop.xxuv_read_start_(client.ptr, OnReadCB);
            }
            if (client.OnConnect != null) client.OnConnect(status);
        }

        public void Connect()
        {
            if (disposed) throw new ObjectDisposedException("XxUvTcpClient");
            if (state != UvTcpStates.Disconnected) throw new InvalidOperationException();

            ptr = UvInterop.xxuv_alloc_uv_tcp_t(handlePtr);
            if (ptr == IntPtr.Zero) throw new OutOfMemoryException();

            int r = UvInterop.xxuv_tcp_init(loop.ptr, ptr);
            if (r != 0)
            {
                this.Free(ref ptr);
                r.Throw();
            }

            r = UvInterop.xxuv_tcp_connect_(ptr, addrPtr, OnConnectCB);
            if (r != 0)
            {
                UvInterop.xxuv_close_(ptr);
                ptr = IntPtr.Zero;
                r.Throw();
            }

            state = UvTcpStates.Connecting;
        }

        public Exception TryConnect(string ipv4, int port)
        {
            try
            {
                Disconnect();
                SetAddress(ipv4, port);
                Connect();
            }
            catch (Exception e)
            {
                return e;
            }
            return null;
        }

        public void Disconnect()
        {
            if (disposed) throw new ObjectDisposedException("XxUvTcpClient");
            if (state == UvTcpStates.Disconnected) return;
            state = UvTcpStates.Disconnected;
            RpcTraceCallback();
            if (OnDisconnect != null) OnDisconnect();
            UvInterop.xxuv_close_(ptr);
            ptr = IntPtr.Zero;
            bbSend.Clear();
            bbRecv.Clear();
        }

        protected override void DisconnectImpl()
        {
            Disconnect();
        }

        #region Dispose

        bool disposing = false; // 防递归
        public override void Dispose()
        {
            if (disposing || disposed) return;
            disposing = true;
            RpcTraceCallback();
            if (OnDispose != null) OnDispose();
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposed)
            {
                // if (disposing) // Free other state (managed objects).

                state = UvTcpStates.Disconnected;
                if (ptr != IntPtr.Zero)
                {
                    UvInterop.xxuv_close_(ptr);
                    ptr = IntPtr.Zero;
                }
                this.Free(ref addrPtr);
                this.Unhandle(ref handle, ref handlePtr);

                UnbindTimeoutManager();
                OnTimeout = null;

                bbSend = null;
                bbRecv = null;
                loop.tcpClients[loop.tcpClients.dataLen - 1].index_at_container = index_at_container;
                loop.tcpClients.SwapRemoveAt(index_at_container);
                loop = null;
                disposed = true;
            }
            //base.Dispose(disposing);
        }

        ~UvTcpClient()
        {
            Dispose(false);
        }

        #endregion
    }

    public class UvTimer : IDisposable
    {
        /******************************************************************************/
        // 用户事件绑定
        public Action OnFire;
        public Action OnDispose;
        /******************************************************************************/

        public bool disposed;
        public UvLoop loop;
        public int index_at_container;

        public IntPtr ptr;
        public GCHandle handle;
        public IntPtr handlePtr;

        public UvTimer(UvLoop loop, ulong timeoutMS, ulong repeatIntervalMS, Action OnFire = null)
        {
            try
            {
                this.OnFire = OnFire;
                this.Handle(ref handle, ref handlePtr);
                this.loop = loop;

                ptr = UvInterop.xxuv_alloc_uv_timer_t(handlePtr);
                if (ptr == IntPtr.Zero)
                {
                    this.Unhandle(ref handle, ref handlePtr);
                    throw new OutOfMemoryException();
                }

                int r = UvInterop.xxuv_timer_init(loop.ptr, ptr);
                if (r != 0)
                {
                    this.Free(ref ptr);
                    this.Unhandle(ref handle, ref handlePtr);
                    r.Throw();
                }

                r = UvInterop.xxuv_timer_start(ptr, OnTimerCB, timeoutMS, repeatIntervalMS);
                if (r != 0)
                {
                    UvInterop.xxuv_close_(ptr);
                    ptr = IntPtr.Zero;
                    this.Unhandle(ref handle, ref handlePtr);
                    r.Throw();
                }

                index_at_container = loop.timers.dataLen;
                loop.timers.Add(this);
            }
            catch (Exception ex)
            {
                disposed = true;
                throw ex;
            }
        }

        static UvInterop.uv_timer_cb OnTimerCB = OnTimerCBImpl;
#if NET_2_0 || NET_2_0_SUBSET
        [AOT.MonoPInvokeCallback(typeof(UvInterop.uv_connection_cb))]
#endif
        static void OnTimerCBImpl(IntPtr handle)
        {
            var timer = handle.To<UvTimer>();
            timer.OnFire();
        }

        public void SetRepeat(ulong repeatIntervalMS)
        {
            if (disposed) throw new ObjectDisposedException("XxUvTimer");
            UvInterop.xxuv_timer_set_repeat(ptr, repeatIntervalMS);
        }

        public void Again()
        {
            if (disposed) throw new ObjectDisposedException("XxUvTimer");
            UvInterop.xxuv_timer_again(ptr).TryThrow();
        }

        public void Stop()
        {
            if (disposed) throw new ObjectDisposedException("XxUvTimer");
            UvInterop.xxuv_timer_stop(ptr).TryThrow();
        }

        #region Dispose

        public void Dispose()
        {
            if (disposed) return;
            if (OnDispose != null) OnDispose();
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposed)
            {
                // if (disposing) // Free other state (managed objects).

                UvInterop.xxuv_close_(ptr);
                ptr = IntPtr.Zero;
                this.Unhandle(ref handle, ref handlePtr);

                loop.timers[loop.timers.dataLen - 1].index_at_container = index_at_container;
                loop.timers.SwapRemoveAt(index_at_container);
                loop = null;
                disposed = true;
            }
            //base.Dispose(disposing);
        }

        ~UvTimer()
        {
            Dispose(false);
        }

        #endregion
    }

    public class UvTimeoutManager
    {
        UvTimer timer;
        List<UvTimeouterBase> timeouterss = new List<UvTimeouterBase>();
        int cursor = 0;                         // 环形游标
        int defaultInterval;

        // intervalMS: 帧间隔毫秒数;    wheelLen: 轮子尺寸( 需求的最大计时帧数 + 1 );    defaultInterval: 默认计时帧数
        public UvTimeoutManager(UvLoop loop, ulong intervalMS, int wheelLen, int defaultInterval)
        {
            timer = new UvTimer(loop, 0, intervalMS, Process);
            timeouterss.Resize(wheelLen);
            this.defaultInterval = defaultInterval;
        }

        public void Process()
        {
            var t = timeouterss[cursor];            // 遍历当前 ticks 链表
            while (t != null)
            {
                t.OnTimeout();                // 执行
                var nt = t.timeouterNext;
                t.TimeouterClear();
                t = nt;
            };
            timeouterss[cursor] = null;
            cursor++;                           // 环移游标
            if (cursor == timeouterss.dataLen) cursor = 0;
        }

        // 不触发 OnTimerFire
        public void Clear()
        {
            for (int i = 0; i < timeouterss.dataLen; ++i)
            {
                var t = timeouterss[i];
                while (t != null)               // 遍历链表
                {
                    var nt = t.timeouterNext;
                    t.TimeouterClear();             // 清理关联
                    t = nt;
                };
                timeouterss[i] = null;              // 清空链表头
            }
            cursor = 0;
        }

        // 于指定 interval 所在 timers 链表处放入一个 timer
        public void Add(UvTimeouterBase t, int interval = 0)
        {
            if (t.timeouting) throw new InvalidOperationException();
            var timeouterssLen = timeouterss.dataLen;
            if (t == null || (interval < 0 && interval >= timeouterss.dataLen)) throw new ArgumentException();
            if (interval == 0) interval = defaultInterval;

            // 环形定位到 timers 下标
            interval += cursor;
            if (interval >= timeouterssLen) interval -= timeouterssLen;

            // 填充 链表信息
            t.timeouterPrev = null;
            t.timeouterIndex = interval;
            t.timeouterNext = timeouterss[interval];
            if (t.timeouterNext != null)            // 有就链起来
            {
                t.timeouterNext.timeouterPrev = t;
            }
            timeouterss[interval] = t;              // 成为链表头
        }

        // 移除
        public void Remove(UvTimeouterBase t)
        {
            if (!t.timeouting) throw new InvalidOperationException();
            if (t.timeouterNext != null) t.timeouterNext.timeouterPrev = t.timeouterPrev;
            if (t.timeouterPrev != null) t.timeouterPrev.timeouterNext = t.timeouterNext;
            else timeouterss[t.timeouterIndex] = t.timeouterNext;
            t.TimeouterClear();
        }

        // 如果存在就移除并放置到新的时间点
        public void AddOrUpdate(UvTimeouterBase t, int interval = 0)
        {
            if (t.timeouting) Remove(t);
            Add(t, interval);
        }

    }

    public class UvAsync : IDisposable
    {
        /******************************************************************************/
        // 用户事件绑定
        public Action OnFire;
        public Action OnDispose;
        /******************************************************************************/

        public bool disposed;
        public UvLoop loop;
        public int index_at_container;
        public ConcurrentQueue<Action> actions = new ConcurrentQueue<Action>();

        public IntPtr ptr;
        public GCHandle handle;
        public IntPtr handlePtr;
        public UvAsync(UvLoop loop)
        {
            try
            {
                this.Handle(ref handle, ref handlePtr);
                this.loop = loop;
                this.OnFire = this.OnFireImpl;

                ptr = UvInterop.xxuv_alloc_uv_async_t(handlePtr);
                if (ptr == IntPtr.Zero)
                {
                    this.Unhandle(ref handle, ref handlePtr);
                    throw new OutOfMemoryException();
                }

                int r = UvInterop.xxuv_async_init(loop.ptr, ptr, AsyncCB);
                if (r != 0)
                {
                    this.Free(ref ptr);
                    this.Unhandle(ref handle, ref handlePtr);
                    r.Throw();
                }

                index_at_container = loop.asyncs.dataLen;
                loop.asyncs.Add(this);
            }
            catch (Exception ex)
            {
                disposed = true;
                throw ex;
            }
        }

        static UvInterop.uv_async_cb AsyncCB = OnAsyncCBImpl;
#if NET_2_0 || NET_2_0_SUBSET
        [AOT.MonoPInvokeCallback(typeof(UvInterop.uv_connection_cb))]
#endif
        static void OnAsyncCBImpl(IntPtr handle)
        {
            var self = handle.To<UvAsync>();
            self.OnFire();
        }

        public void Dispatch(Action a)
        {
            if (disposed) throw new ObjectDisposedException("XxUvAsync");
            actions.Enqueue(a);
            UvInterop.xxuv_async_send(ptr).TryThrow();
        }

        public void OnFireImpl()
        {
            Action a;
            while (actions.TryDequeue(out a)) a();
        }

        #region Dispose

        public void Dispose()
        {
            if (disposed) return;
            if (OnDispose != null) OnDispose();
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposed)
            {
                // if (disposing) // Free other state (managed objects).
                actions = null;

                UvInterop.xxuv_close_(ptr);
                ptr = IntPtr.Zero;
                this.Unhandle(ref handle, ref handlePtr);

                loop.asyncs[loop.asyncs.dataLen - 1].index_at_container = index_at_container;
                loop.asyncs.SwapRemoveAt(index_at_container);
                loop = null;
                disposed = true;
            }
            //base.Dispose(disposing);
        }

        ~UvAsync()
        {
            Dispose(false);
        }

        #endregion
    }

    public class UvRpcManager
    {
        // 已知问题: 当 uv 太繁忙时, timer 可能无法及时回调, 导致超时和队列清理行为无法及时发生
        UvTimer timer;

        // 循环使用的自增流水号
        int serial;

        // 流水号 与 cb(bb) 回调 的映射. bb 为空表示超时调用
        Dict<int, Action<int, BBuffer>> mapping = new Dict<int, Action<int, BBuffer>>();

        // 用一个队列记录流水号的超时时间, 以便删掉超时的. first: timeout ticks
        Queue<Pair<int, int>> serials = new Queue<Pair<int, int>>();

        // 默认计时帧数
        int defaultInterval;

        // 帧步进值
        int ticks;

        // 返回队列深度
        public int count { get { return serials.Count; } }

        // intervalMS: 帧间隔毫秒数;    defaultInterval: 默认计时帧数
        public UvRpcManager(UvLoop loop, ulong intervalMS, int defaultInterval)
        {
            if (defaultInterval <= 0) throw new ArgumentException();
            timer = new UvTimer(loop, 0, intervalMS, Process);
            this.defaultInterval = defaultInterval;
        }

        // 不断将超时的从字典移除( 以 false 参数 call 之 ), 直到 Pop 到未超时的停止.
        public void Process()
        {
            ++ticks;
            if (serials.IsEmpty) return;
            while (!serials.IsEmpty && serials.Top().first <= ticks)
            {
                var idx = mapping.Find(serials.Top().second);
                if (idx != -1)
                {
                    var a = mapping.ValueAt(idx);
                    mapping.RemoveAt(idx);
                    a(serial, null);
                }
                serials.Pop();
            }
        }

        // 放入上下文, 返回流水号
        public int Register(Action<int, BBuffer> cb, int interval = 0)
        {
            if (interval == 0) interval = defaultInterval;
            unchecked { serial = (serial + 1) & 0x7FFFFFFF; }           // 非负循环自增
            var r = mapping.Add(serial, cb, true);
            serials.Push(new Pair<int, int>
            {
                first = ticks + interval,       // 算出超时 ticks
                second = serial
            });
            return serial;
        }

        // 根据流水号 反注册回调事件( 通常出现于提前断线或退出之后不想收到相关回调 )
        // 这种情况不产生回调
        public void Unregister(int serial)
        {
            mapping.Remove(serial);
        }

        // 根据 流水号 定位到 回调函数并调用( 由 UvTcpXxxx 来 call )
        public void Callback(int serial, BBuffer bb)
        {
            int idx = mapping.Find(serial);
            if (idx == -1) return;              // 已超时移除
            var a = mapping.ValueAt(idx);
            mapping.RemoveAt(idx);
            if (a != null) a(serial, bb);
        }
    }

    public static class UvInterop
    {
#if !UNITY_EDITOR && UNITY_IPHONE
        const string DLL_NAME = "__Internal";
#else
        const string DLL_NAME = "xxuvlib";
#endif

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr xxuv_alloc_uv_loop_t(IntPtr ud);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr xxuv_alloc_uv_tcp_t(IntPtr ud);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr xxuv_alloc_uv_udp_t(IntPtr ud);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr xxuv_alloc_sockaddr_in(IntPtr ud);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr xxuv_alloc_uv_timer_t(IntPtr ud);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr xxuv_alloc_uv_async_t(IntPtr ud);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr xxuv_alloc_uv_signal_t(IntPtr ud);



        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern void xxuv_free(IntPtr p);



        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr xxuv_get_ud(IntPtr p);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr xxuv_get_ud_from_uv_connect_t(IntPtr req);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr xxuv_get_buf(IntPtr buf_t);



        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr xxuv_strerror(int n);


        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr xxuv_err_name(int n);



        //[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        //public delegate void uv_close_cb(IntPtr handle);

        //[DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        //public static extern void xxuv_close(IntPtr handle, uv_close_cb close_cb);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern void xxuv_close_(IntPtr handle);   // auto free




        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int xxuv_loop_init(IntPtr loop);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int xxuv_run(IntPtr loop, UvRunMode mode);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern void xxuv_stop(IntPtr loop);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int xxuv_loop_close(IntPtr loop);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int xxuv_loop_alive(IntPtr loop);




        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int xxuv_ip4_addr(string ipv4, int port, IntPtr addr);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int xxuv_ip6_addr(string ipv6, int port, IntPtr addr);




        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int xxuv_tcp_init(IntPtr loop, IntPtr tcp);

        //[DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        //public static extern int xxuv_tcp_bind(IntPtr tcp, IntPtr addr, uint flags);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int xxuv_tcp_bind_(IntPtr listener, IntPtr addr);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void uv_connection_cb(IntPtr listener, int status);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int xxuv_listen(IntPtr listener, int backlog, uv_connection_cb cb);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int xxuv_accept(IntPtr listener, IntPtr peer);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void uv_read_cb(IntPtr stream, IntPtr nread, IntPtr buf_t);

        //public delegate void uv_alloc_cb (IntPtr handle, IntPtr suggested_size, IntPtr buf);
        //[DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        //public static extern int xxuv_read_start(IntPtr client, uv_alloc_cb alloc_cb, uv_read_cb read_cb);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int xxuv_read_start_(IntPtr stream, uv_read_cb read_cb);

        //public delegate void uv_write_cb(IntPtr req, int status);
        //[DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        //public static extern int xxuv_write(IntPtr req, IntPtr stream, uv_buf_t[] bufs, uint nbufs, uv_write_cb cb);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int xxuv_write_(IntPtr stream, IntPtr buf, uint offset, uint len);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int xxuv_fill_client_ip(IntPtr stream, IntPtr buf, int buf_len, ref int data_len);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void uv_connect_cb(IntPtr req, int status);

        //[DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        //public static extern int xxuv_tcp_connect(IntPtr req, IntPtr stream, IntPtr addr, uv_connect_cb cb);
        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int xxuv_tcp_connect_(IntPtr stream, IntPtr addr, uv_connect_cb cb);






        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int xxuv_timer_init(IntPtr loop, IntPtr timer_req);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void uv_timer_cb(IntPtr req);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int xxuv_timer_start(IntPtr timer_req, uv_timer_cb cb, ulong timeoutMS, ulong repeatIntervalMS);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern void xxuv_timer_set_repeat(IntPtr timer_req, ulong repeatIntervalMS);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int xxuv_timer_again(IntPtr timer_req);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int xxuv_timer_stop(IntPtr timer_req);




        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void uv_async_cb(IntPtr req);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int xxuv_async_init(IntPtr loop, IntPtr async_req, uv_async_cb cb);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int xxuv_async_send(IntPtr async_req);





        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int xxuv_is_readable(IntPtr stream);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int xxuv_is_writable(IntPtr stream);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr xxuv_stream_get_write_queue_size(IntPtr stream);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern int xxuv_try_write_(IntPtr stream, IntPtr buf, uint len);





        public static void Throw(this int n)
        {
            var p = UvInterop.xxuv_strerror(n);
            var s = Marshal.PtrToStringAnsi(p);
            throw new Exception("uv exception: " + s);
        }

        public static void TryThrow(this int n)
        {
            if (n != 0) Throw(n);
        }


        public static T To<T>(this IntPtr p)
        {
            return (T)((GCHandle)xxuv_get_ud(p)).Target;
        }

        public static void Handle(this IDisposable self, ref GCHandle handle, ref IntPtr handlePtr)
        {
            handle = GCHandle.Alloc(self);
            handlePtr = (IntPtr)handle;
        }

        // self 用不到. 只是为便于写扩展, 说明语义
        public static void Unhandle(this IDisposable self, ref GCHandle handle, ref IntPtr handlePtr)
        {
            if (handlePtr != IntPtr.Zero)
            {
                handle.Free();
                handlePtr = IntPtr.Zero;
            }
        }

        // self 用不到. 只是为便于写扩展, 说明语义
        public static void Free(this IDisposable self, ref IntPtr ptr)
        {
            if (ptr != IntPtr.Zero)
            {
                xxuv_free(ptr);
                ptr = IntPtr.Zero;
            }
        }

    }

#if NET_2_0 || NET_2_0_SUBSET
    // u3d 下 模拟 ConcurrentQueue
    public class ConcurrentQueue<T>
    {
        protected xx.Queue<T> queue;

        public void Enqueue(T a)
        {
            lock (queue) queue.Enqueue(a);
        }

        public bool TryDequeue(out T a)
        {
            lock (queue) return queue.TryDequeue(out a);
        }
    }
#endif
}
