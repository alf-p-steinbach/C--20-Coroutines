#include <vector>
#include <stdio.h>
using std::vector;

auto numbers( const int n ) -> vector<int>
{
    vector<int> result;
    int sum = 0;
    for( int i = 1; i <= n; ++i ) {
        sum += i;
        result.push_back( sum );
    }
    return result;
}

auto main() -> int
{
    int sum = 0;
    for( const int v: numbers( 7 ) ) { sum += v; }
    printf( "%d\n", sum );
}
