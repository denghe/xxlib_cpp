#pragma once

namespace xxx
{
    // 2进制排列为 从右到左，由低到高位，即小尾机可 memcpy
    class List_bool
    {
    public:
        List_bool( int capacity = 512 );
        ~List_bool();
        List_bool( List_bool&& o );
        List_bool( List_bool const& o );
        List_bool& operator=( List_bool&& o );
        List_bool& operator=( List_bool const& o );
        void Push( bool bit );
        void Pop();
        bool Top() const;
        void Clear();
        void Reserve( int capacity );
        char* Data() const;
        int Size() const;
        int ByteSize() const;
        bool operator[]( int idx ) const;
        bool IndexAt( int idx ) const;
        void Set( int idx, bool bit );
        void SetTrue( int idx );
        void SetFalse( int idx );
        void FillTrue();
        void FillFalse();
        void Fill( bool bit, int idxFrom = 0, int idxTo = 0 );
        void Resize( int capacity, bool init = true );
        void CleanUp() const;   // clear last byte's unused bits

    private:
        char*       buf;
        int         size;
        int         maxSize;
    };


}
