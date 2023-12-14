#include <stdio.h>      // printf
#include <concepts>
#include <coroutine>
#include <optional>
#include <utility>

namespace app {
    using   std::convertible_to,                            // <concepts>
            std::coroutine_handle, std::suspend_always,     // <coroutine>
            std::nullopt, std::optional,                    // <optional>
            std::forward;                                   // <utility>

    struct Promise;

    struct Result
    {
        using promise_type = Promise;       // Required alias.
        coroutine_handle<Promise>   handle;
    };
    
    struct Promise
    {
        using Self          = Promise;
        using Yield_result  = int;

        optional<Yield_result>  m_value_yielded;    // Empty communicates "finished" (our choice).

        // All 5 functions are required, except can use `return_value` instead of `return_void`.
        // `return_void` is here not a dummy: it has to communicate that the coroutine is finished.

        auto get_return_object()        // Can't be `const` b/c `from_promise` parameter.
            -> Result
        { return Result{ coroutine_handle<Self>::from_promise( *this ) }; }

        auto initial_suspend() const noexcept   -> suspend_always   { return {}; }
        auto final_suspend() noexcept           -> suspend_always   { return {}; }
        void return_void() { m_value_yielded = nullopt; }
        void unhandled_exception() {}   // Not expecting any exception, but could store it.

        // This function (or overloads) is called by a `co_yield`, i.e. is required for that:
        template< convertible_to<Yield_result> From >
        auto yield_value( From&& from )
            -> suspend_always
        {
            m_value_yielded = forward<From>( from );
            return {};
        }
    };

    auto numbers() -> Result
    {
        for( int i = 1; i <= 7; ++i ) { co_yield i*i; }
    }
    
    void run()
    {
        puts( "Instantiating the coroutine." );
        const auto h = numbers().handle;
        Promise& promise = h.promise();
        // At this point none of the code in the coroutine body has executed.
        
        puts( "Transferring control to the coroutine." );
        h.resume();

        puts( "Displaying the values that it produces." );
        printf( "%4s", "" );
        while( promise.m_value_yielded.has_value() ) {
            printf( "%d ", promise.m_value_yielded.value() );
            h.resume();
        }
        printf( "\n" );
        
        puts( "Finished." );
        h.destroy();
    }
}  // namespace app

auto main() -> int { app::run(); }
