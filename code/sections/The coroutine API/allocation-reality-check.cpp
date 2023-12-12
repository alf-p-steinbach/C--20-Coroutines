﻿#include <stddef.h>     // size_t
#include <stdio.h>      // puts, fprintf, stderr
#include <stdlib.h>     // malloc

// Keep track of memory allocations because no leaks is crucial for large scale usage:
int     n_bytes_allocated   = 0;
int     n_bytes_deallocated = 0;

auto operator new( const size_t size )
    -> void*
{ n_bytes_allocated += int( size ); return ::malloc( size ); }

void operator delete( void* p, const size_t size )
{
    n_bytes_deallocated += int( size );
    ::free( p );
}

namespace {
    struct Envelope
    {
        ~Envelope()
        {
            fprintf( stderr, "\n%d bytes allocated, %d bytes deallocated, %s.\n",
                ::n_bytes_allocated,
                ::n_bytes_deallocated,
                (::n_bytes_allocated == ::n_bytes_deallocated? "OK" : "oops.")
                );
        }
        
        Envelope() {}
    };
    
    const auto envelope = Envelope();
}  // namespace <anon>
