class Consumer
{
    int     m_sum   = 0;

public:
    void process( const int v ) { m_sum += v; }
    auto result() const -> int { return m_sum; }
};

#include <stdio.h>
auto main() -> int
{
    Consumer consumer;
    int sum = 0;
    for( int i = 1; i <= 7; ++i ) {
        sum += i;
        consumer.process( sum );
    }
    printf( "%d\n", consumer.result() );
}
