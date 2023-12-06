class Numbers
{
    int     m_n;
    int     m_i;
    int     m_sum;

public:
    Numbers( const int n ): m_n( n ), m_i( 0 ), m_sum( 0 ) { advance(); }
    
    auto current() const        -> int      { return m_sum; }
    auto available() const      -> bool     { return (m_i <= m_n); }

    void advance()
    {
        ++m_i;
        m_sum = (available()? m_sum + m_i : -1);
    }
};

#include <stdio.h>
auto main() -> int
{
    int sum = 0;
    for( Numbers numbers( 7 ); numbers.available(); numbers.advance() ) {
        sum += numbers.current();
    }
    printf( "%d\n", sum );
}
