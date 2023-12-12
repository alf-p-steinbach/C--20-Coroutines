#include <stddef.h>     // size_t
#include <stdio.h>      // puts, fprintf, stderr
#include <stdlib.h>     // malloc
#include <coroutine>

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

namespace app {
    using   std::coroutine_handle, std::suspend_always;

    struct Promise;

    struct Result
    {
        using promise_type = Promise;       // Required alias.
        coroutine_handle<Promise>   handle; // What this is all about: getting a handle on things.
    };
    
    struct Promise
    {
        using Self = Promise;

        // All 5 functions are required, except can use `return_value` instead of `return_void`.

        auto get_return_object()        // Can't be `const` b/c `from_promise` parameter.
            -> Result
        { return Result{ coroutine_handle<Self>::from_promise( *this ) }; }

        auto initial_suspend() const noexcept   -> suspend_always   { return {}; }
        auto final_suspend() noexcept           -> suspend_always   { return {}; }
        void return_void() {}
        void unhandled_exception() {}   // Not expecting any exception, but could store it.
    };

    auto say_hello() -> Result
    {
        puts( "  Coroutine `say_hello` says hello, dear world!" );
        co_return;
    }
    
    void run()
    {
        puts( "Instantiating the coroutine." );
        const auto h = say_hello().handle;
        // At this point none of the code in the coroutine body has executed.
        
        puts( "Transferring control to the coroutine." );
        h.resume();
        
        puts( "Finished." );
        #ifndef FORCE_MEMORY_LEAK
            h.destroy();
        #endif
    }
}  // namespace app

auto main() -> int
{
    app::run();
    fprintf( stderr, "%d bytes allocated, %d bytes deallocated.\n",
        ::n_bytes_allocated, ::n_bytes_deallocated
        );
}
