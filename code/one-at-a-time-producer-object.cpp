#include <stdio.h>

class Squares
{
    int     m_x;
    int     m_n;
    
public:
    Squares( const int n ): m_x( 1 ), m_n( n ) {}
    
    auto current() const        -> int      { return m_x*m_x; }
    auto is_finished() const    -> bool     { return (m_x > m_n); }

    void advance() { ++m_x; }
};

auto main() -> int
{
    int sum = 0;
    for( Squares sq( 7 ); not sq.is_finished(); sq.advance() ) {
        sum += sq.current();
    }
    printf( "%d\n", sum );
}
