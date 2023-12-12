#include <stddef.h>     // size_t
#include <stdio.h>      // fprintf, stderr
#include <stdlib.h>     // malloc

// Keep track of memory allocations because no leaks is crucial for large scale usage:
int     n_bytes_allocated   = 0;
int     n_bytes_deallocated = 0;
int     n_unsized_deallocs  = 0;
int     n_allocs            = 0;
int     n_deallocs          = 0;

auto operator new( const size_t size )
    -> void*
{
    n_bytes_allocated += int( size );
    ++n_allocs;
    return ::malloc( size );
}

void operator delete( void* ) { ++n_unsized_deallocs; ++n_deallocs; }

void operator delete( void* p, const size_t size )
{
    n_bytes_deallocated += int( size );  ++n_deallocs;
    ::free( p );
}

namespace {
    struct Envelope
    {
        ~Envelope()
        {
            fprintf( stderr,
                "\n%d bytes allocated in %d calls.\n"
                "%d bytes known deallocated in %d calls.\n"
                "%d unsized deallocs.\n"
                "Number of allocations and deallocations %smatch.\n"
                "%s.\n",
                ::n_bytes_allocated, n_allocs,
                ::n_bytes_deallocated, n_deallocs,
                ::n_unsized_deallocs,
                (::n_allocs == ::n_deallocs? "" : "DON'T "),
                (
                    (n_allocs == n_deallocs) and
                    (::n_bytes_allocated == ::n_bytes_deallocated and ::n_unsized_deallocs == 0
                    or ::n_bytes_allocated > n_bytes_deallocated and ::n_unsized_deallocs > 0)
                    ? "OK" : "oops.")
                );
        }
        
        Envelope() {}
    };
    
    const auto envelope = Envelope();
}  // namespace <anon>
