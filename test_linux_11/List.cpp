#include "List.h"
#include <cassert>
#include <string.h>
#include <type_traits>
#include <cstddef>
#include <algorithm>
#include <stdint.h>

namespace xxx
{
    size_t Calc2n(size_t const& n) noexcept {
        assert(n);
#ifdef _MSC_VER
        unsigned long r = 0;
#if defined(_WIN64) || defined(_M_X64)
        _BitScanReverse64(&r, n);
# else
        _BitScanReverse(&r, n);
# endif
        return (size_t)r;
#else
#if defined(__LP64__) || __WORDSIZE == 64
        return int(63 - __builtin_clzl(n));
# else
        return int(31 - __builtin_clz(n));
# endif
#endif
    }

    // 返回一个刚好大于 n 的 2^x 对齐数
    size_t Round2n(size_t const& n) noexcept {
        auto rtv = size_t(1) << Calc2n(n);
        if (rtv == n) return n;
        else return rtv << 1;
    }

    List_bool::List_bool( int capacity )
    {
        assert( capacity > 0 );
        auto len = int( capacity / 8 );
        if( len < 64 ) len = 64;
        else len = (int)Round2n( len );
        size = 0;
        buf = new char[ len ];
        maxSize = len * 8;
    }

    List_bool::~List_bool()
    {
        if( buf )
        {
            Clear();
            delete[] buf;
        }
    }

    List_bool::List_bool( List_bool && o )
        : buf( o.buf )
        , size( o.size )
        , maxSize( o.maxSize )
    {
        o.buf = nullptr;
    }

    List_bool::List_bool( List_bool const & o )
        : List_bool( o.size )
    {
        memcpy( buf, o.buf, o.ByteSize() );
    }

    List_bool& List_bool::operator=( List_bool && o )
    {
        delete[] buf;
        buf = o.buf;
        size = o.size;
        maxSize = o.maxSize;
        o.buf = nullptr;
        return *this;
    }

    List_bool& List_bool::operator=( List_bool const & o )
    {
        if( this == &o ) return *this;
        size = o.size;
        if( maxSize < o.size )
        {
            auto len = int( o.size / 8 );
            if( len < 64 ) len = 64;
            else len = (int)Round2n( len );
            maxSize = len * 8;
            delete[] buf;
            buf = new char[ len ];
        }
        memcpy( buf, o.buf, o.ByteSize() );
        return *this;
    }

    void List_bool::Push( bool v )
    {
        if( size == maxSize ) Reserve( size + 1 );
        Set( size++, v );
    }

    void List_bool::Pop()
    {
        assert( size > 0 );
        --size;
    }

    bool List_bool::Top() const
    {
        assert( size > 0 );
        return operator[]( size - 1 );
    }

    void List_bool::Clear()
    {
        size = 0;
    }

    void List_bool::Reserve( int capacity )
    {
        assert( capacity > 0 );
        if( capacity <= maxSize ) return;
        auto len = (int)Round2n( ( capacity - 1 ) / 8 + 1 );
        maxSize = len * 8;
        auto newBuf = new char[ len ];
        memcpy( newBuf, buf, ByteSize() );
        delete[] buf;
        buf = newBuf;
    }

    char* List_bool::Data() const
    {
        return buf;
    }

    int List_bool::Size() const
    {
        return size;
    }

    int List_bool::ByteSize() const
    {
        if( size ) return ( size - 1 ) / 8 + 1;
        return 0;
    }

    bool List_bool::operator[]( int idx ) const
    {
        return IndexAt( idx );
    }

    bool List_bool::IndexAt( int idx ) const
    {
        return ( ( (size_t*)buf )[ idx / ( sizeof( size_t ) * 8 ) ] &
                 ( size_t( 1 ) << ( idx % ( sizeof( size_t ) * 8 ) ) ) ) > 0;
    }

    void List_bool::Set( int idx, bool v )
    {
        assert( idx >= 0 && idx < size );
        if( v ) SetTrue( idx );
        else SetFalse( idx );
    }

    void List_bool::SetTrue( int idx )
    {
        assert( idx >= 0 && idx < size );
        ( (size_t*)buf )[ idx / ( sizeof( size_t ) * 8 ) ] |=
            ( size_t( 1 ) << ( idx % ( sizeof( size_t ) * 8 ) ) );
    }

    void List_bool::SetFalse( int idx )
    {
        assert( idx >= 0 && idx < size );
        ( (size_t*)buf )[ idx / ( sizeof( size_t ) * 8 ) ] &=
            ~( size_t( 1 ) << ( idx % ( sizeof( size_t ) * 8 ) ) );
    }

    void List_bool::FillTrue()
    {
        memset( buf, 0xFFFFFFFFu, ( size - 1 ) / 8 + 1 );
    }

    void List_bool::FillFalse()
    {
        memset( buf, 0, ( size - 1 ) / 8 + 1 );
    }

    void List_bool::Fill( bool v, int idxFrom, int idxTo )
    {
        assert( size > 0 && idxFrom >= 0 && idxTo >= 0 && idxFrom < size && idxTo < size );
        if( idxFrom == idxTo )
        {
            Set( idxFrom, v );
            return;
        }
        if( idxFrom > idxTo )
        {
            std::swap( idxFrom, idxTo );
        }
        auto byteIdxFrom = idxFrom >> 3;
        auto byteIdxTo = idxTo >> 3;

        if( byteIdxFrom == byteIdxTo )
        {
            // 搞一个 中间一段是 v 的 uint8_t 出来
            if( v )
            {
                buf[ byteIdxFrom ] |= (uint8_t)0xFFu >> ( 7 - ( idxTo - idxFrom ) ) << ( idxFrom & 7 );
            }
            else
            {
                buf[ byteIdxFrom ] &= ~( (uint8_t)0xFFu >> ( 7 - ( idxTo - idxFrom ) ) << ( idxFrom & 7 ) );
            }
        }
        else
        {
            // 分别搞一头一尾, 再 memset 中间
            auto idxFrom7 = idxFrom & 7;
            auto idxTo7 = idxTo & 7;
            if( v )
            {
                buf[ byteIdxFrom ] |= (uint8_t)0xFFu << idxFrom7;
                buf[ byteIdxTo ] |= (uint8_t)0xFFu >> ( 7 - idxTo7 );
            }
            else
            {
                buf[ byteIdxFrom ] &= ~( (uint8_t)0xFFu << idxFrom7 );
                buf[ byteIdxTo ] &= ~( (uint8_t)0xFFu >> ( 7 - idxTo7 ) );
            }
            if( idxFrom7 ) ++byteIdxFrom;
            if( idxTo7 ) --byteIdxTo;
            if( byteIdxFrom <= byteIdxTo )
            {
                memset( buf + byteIdxFrom, v ? 0xFFFFFFFFu : 0, byteIdxTo - byteIdxFrom );
            }
        }
    }

    void List_bool::Resize( int capacity, bool init )
    {
        if( capacity == size ) return;
        if( capacity < size )
        {
            size = capacity;
            return;
        }
        Reserve( capacity );
        auto oldSize = size;
        size = capacity;
        if( init )
        {
            Fill( false, oldSize, capacity - 1 );
        }
    }

    void List_bool::CleanUp() const
    {
        if( auto mod = size % 8 )
        {
            buf[ ( size - 1 ) / 8 ] &= ~( 0xFF << mod );
        }
    }
}
