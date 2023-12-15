#include <stdio.h>      // printf
#include <concepts>
#include <coroutine>
#include <optional>
#include <utility>

namespace app {
    using   std::coroutine_handle, std::suspend_always,     // <coroutine>
            std::nullopt, std::optional,                    // <optional>
            std::forward;                                   // <utility>

    struct Promise;

    struct Result
    {
        using promise_type = Promise;       // Required alias.
        coroutine_handle<Promise>   handle;
    };

    struct Input {};                        // A tag type.

    struct Promise
    {
        using Self          = Promise;
        using Await_result  = int;

        optional<Await_result>  m_value_awaited;    // Empty communicates "finish!" (our choice).

        struct Awaiter
        {
            const Promise*  m_p_promise;

            auto await_ready() const -> bool { return false; }          // I.e. suspend.
            void await_suspend( coroutine_handle<Promise> ) const {}    // "void": xfer back up to caller.
            
            auto await_resume() const
                -> optional<Await_result>                       // Return value for the `co_await`.
            { return m_p_promise->m_value_awaited; }
        };

        // All 5 functions are required, except can use `return_value` instead of `return_void`.
        auto get_return_object()        // Can't be `const` b/c `from_promise` parameter.
            -> Result
        { return Result{ coroutine_handle<Self>::from_promise( *this ) }; }

        auto initial_suspend() const noexcept   -> suspend_always   { return {}; }
        auto final_suspend() const noexcept     -> suspend_always   { return {}; }
        void return_void() const noexcept {}
        void unhandled_exception() {}   // Not expecting any exception, but could store it.
        
        auto await_transform( Input ) const noexcept -> Awaiter { return {this}; }
    };

    auto presenter() -> Result
    {
        int n_presented = 0;
        for( ;; ) {
            const optional<int> input = co_await Input();
            if( not input.has_value() ) {
                break;
            }
            printf( "%s%d", (n_presented > 0? ", " : ""), input.value() );
            ++n_presented;
        }
        if( n_presented > 0 ) { printf( ".\n" ); };
    }
    
    void run()
    {
        puts( "Instantiating the coroutine." );
        const auto h = presenter().handle;
        Promise& promise = h.promise();
        // At this point none of the code in the coroutine body has executed.
        
        puts( "Transferring control to the coroutine." );
        h.resume();     // Starts it waiting for the first value.

        puts( "Sending values to the coroutine." );
        printf( "%4s", "" );
        for( int i = 1; i <= 7; ++i ) {
            promise.m_value_awaited = i*i;
            h.resume();
        }
        promise.m_value_awaited = nullopt;
        h.resume();
        
        puts( "Finished." );
        h.destroy();
    }
}  // namespace app

auto main() -> int { app::run(); }
