#include <stdio.h>
auto main() -> int
{
    int s2 = 0;
    int s3 = 0;
    for( int s1 = 1; s1 <= 7; s1 += 1 ) {
        s2 += s1;
        s3 += s2;
    }
    printf( "%d\n", s3 );
}
