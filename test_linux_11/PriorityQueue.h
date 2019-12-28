#ifndef __PRIORITYQUEUE_H__
#define __PRIORITYQUEUE_H__

#include <vector>

// only for AStar
template<typename T>
struct PriorityQueue
{
    int Push( T item )
    {
        int p = (int)data.size(), p2;
        item->pqIdx = p;
        data.push_back( item );
        do
        {
            if( p == 0 ) break;
            p2 = ( p - 1 ) / 2;
            if( data[ p ]->f < data[ p2 ]->f )
            {
                Swap( p, p2 );
                p = p2;
            }
            else break;
        } while( true );
        return p;
    }

    T Pop()
    {
        auto result = data[ 0 ];
        int p = 0, p1, pn;

        data[ 0 ] = data[ data.size() - 1 ];
        data[ 0 ]->pqIdx = 0;
        data.pop_back();

        do
        {
            pn = p;
            p1 = 2 * p + 1;
            if( (int)data.size() > p1 && data[ p ]->f > data[ p1 ]->f ) p = p1;
            if( p == pn ) break;
            Swap( p, pn );
        } while( true );

        result->pqIdx = -1;
        return result;
    }

    void Update( T item )
    {
        int count = (int)data.size();
        while( ( item->pqIdx - 1 >= 0 ) && ( data[ item->pqIdx - 1 ]->f > data[ item->pqIdx ]->f ) )
        {
            Swap( item->pqIdx - 1, item->pqIdx );
        }
        while( ( item->pqIdx + 1 < count ) && ( data[ item->pqIdx + 1 ]->f < data[ item->pqIdx ]->f ) )
        {
            Swap( item->pqIdx + 1, item->pqIdx );
        }
    }

    void Clear()
    {
        data.clear();
    }

private:
    void Swap( int srcIdx, int dstIdx )
    {
        std::swap( data[ srcIdx ]->pqIdx, data[ dstIdx ]->pqIdx );
        std::swap( data[ srcIdx ], data[ dstIdx ] );
    }

    std::vector<T> data;
};


#endif
