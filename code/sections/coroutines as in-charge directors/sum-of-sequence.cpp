#include <cpp_machinery/coroutine/Sequence_.hpp>
#include <stdio.h>
namespace coroutine = cpp_machinery::coroutine;

auto numbers( const int n ) -> coroutine::Sequence_<int>
{
    int sum = 0;
    for( int i = 1; i <= n; ++i ) {
        sum += i;
        co_yield sum;
    }
}

auto main() -> int
{
    int sum = 0;
    for( const int v: numbers( 7 ) ) { sum += v; }
    printf( "%d\n", sum );
}
