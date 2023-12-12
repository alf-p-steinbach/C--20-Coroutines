#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <cpp_machinery\basic\type_builders.hpp>    // in_, const_, ref_

#include <concepts>
#include <coroutine>
#include <exception>
#include <optional>
#include <stdexcept>
#include <utility>
#include <variant>

namespace cpp_machinery::coroutine {
    using   std::convertible_to,                                                    // <concepts>
            std::coroutine_handle, std::suspend_always,                             // <coroutine>
            std::current_exception, std::exception_ptr, std::rethrow_exception,     // <exception>
            std::optional,                                                          // <optional>
            std::runtime_error,                                                     // <stdexcept>
            std::forward, std::move,                                                // <utility>
            std::get, std::variant, std::monostate;                                 // <variant>

    template< class Yield_result >
    class Simple_progress_state_
    {
    public:
        struct State_index{ enum Enum{ startup, value, finished }; };

    private:
        using   Starting_state      = monostate;
        using   Value_state         = optional< Yield_result >;
        using   Finished_state      = exception_ptr;

        using   State = variant< Starting_state, Value_state, Finished_state >;
        
        State           m_state;

        auto opt_value_ref()    -> ref_<Value_state>    { return get<State_index::value>( m_state ); }
        auto x_ptr_ref()        -> ref_<exception_ptr>  { return get<State_index::finished>( m_state ); }

        auto opt_value_ref() const  -> ref_<const Value_state>      { return get<State_index::value>( m_state ); }
        auto x_ptr_ref() const      -> ref_<const exception_ptr>    { return get<State_index::finished>( m_state ); }

    public:
        auto state() const -> State_index::Enum { return typename State_index::Enum( m_state.index() ); }

        auto is_in_startup_state() const noexcept   -> bool { return (state() == State_index::startup); }
        auto is_in_value_state() const noexcept     -> bool { return (state() == State_index::value); }
        auto is_in_finished_state() const noexcept  -> bool { return (state() == State_index::finished); }

        // Startup-state → value-state:
        //
        template< convertible_to<Yield_result> From >
        void set_value( From&& from )
        {
            if( is_in_finished_state() ) {
                // TODO: Check for nested exception.
                throw runtime_error( "Can't go back from finished state." );
            }
            m_state.template emplace<State_index::value>( forward<From>( from ) );
        }

        // Value-state → finished-state:
        //
        void set_exception( const exception_ptr px ) { m_state.template emplace<State_index::finished>( px ); }
        void set_finished() { if( not is_in_finished_state() ) { set_exception( nullptr ); } }


        // Value-state interface:

        auto has_value() const noexcept
            -> bool
        { return (is_in_value_state() and !!opt_value_ref()); }

        void clear_any_value()
        {
            if( is_in_value_state() ) { opt_value_ref().reset(); }
        }

        auto value() -> ref_<Yield_result>
        {
            rethrow_if_exception();
            if( not has_value() ) {
                throw runtime_error( "No value value." );
            }
            return opt_value_ref().value();
        }


        // Finished-state interface:

        auto has_exception() const noexcept
            -> bool
        { return (is_in_finished_state() and !!x_ptr_ref()); }

        void rethrow_if_exception() const
        {
            // TODO: Check for nested exception.
            if( has_exception() ) { rethrow_exception( x_ptr_ref() ); }
        }
    };


    //-----------------------------------------------------------------------------------------
    // Implementation of the standard interface used by the C++ coroutine machinery:
    //
    template< class Coroutine_result, class Yield_result >
    class Simple_promise_:
        public Simple_progress_state_< Yield_result >
    {
        using Base      = Simple_progress_state_< Yield_result >;
        using Self      = Simple_promise_;
        using Handle    = coroutine_handle<Self>;

    public:
        using   Base::set_finished, Base::set_exception, Base::set_value;

        auto get_return_object()      // Can't be `const` b/c `from_promise`.
            -> Coroutine_result
        { return Coroutine_result( Handle::from_promise( *this ) ); }

        auto initial_suspend() const noexcept   -> suspend_always   { return {}; }
        auto final_suspend() noexcept           -> suspend_always   { set_finished(); return {}; }

        void unhandled_exception() { set_exception( current_exception() ); }

        template< convertible_to<Yield_result> From >
        auto yield_value( From&& from )
            -> suspend_always
        {
            set_value( forward<From>( from ) );
            return {};
        }

        void return_void() {}
    };
     

    // Basic_sequence_.
    // Example usage in classic `for` loop, where `one_through` returns a `Basic_sequence_`:
    //
    //  for( auto numbers = one_through( 7 ); not numbers.is_finished(); numbers.advance() ) {
    //      printf( "%d ", numbers.moved_value() );
    //  }
    //  printf( "\n" );
    //
    template< class Coroutine_result, class Yield_result >
    class Basic_sequence_
    {
    public:
        using Promise   = Simple_promise_< Coroutine_result, Yield_result >;
        using Handle    = Promise::Handle;

        using promise_type = Promise;       // Required.

    private:
        Basic_sequence_( in_<Basic_sequence_> ) = delete;
        auto operator=( in_<Basic_sequence_> ) = delete;

        Handle      m_cor_handle;

        auto promise() const    -> ref_<Promise>    { return m_cor_handle.promise(); }  // Cache it?

        void if_starting_up_start_execution() const
        {
            if( promise().is_in_startup_state() ) { m_cor_handle.resume(); }
        }

        template< class Result, class Self >
        static auto value_impl_( ref_<Self> self ) -> ref_<Result>
        {
            self.if_starting_up_start_execution();
            return self.promise().value();
        }

     public:
        ~Basic_sequence_() { m_cor_handle.destroy(); }
        Basic_sequence_( const Handle h ) : m_cor_handle( h ) {}

        auto is_finished() const -> bool { return m_cor_handle.done(); }

        void advance()
        {
            if( is_finished() ) {
                throw runtime_error( "Finished, can't advance." );
            }
            m_cor_handle.resume();
        }

        auto value() -> ref_<Yield_result>
        {
            if_starting_up_start_execution();
            return promise().value();
        }
    };


    // Iterable_sequence_.
    // Offers `begin()` and `end()`, for e.g. range based loop. I.e. standard C++ iteration.
    //
    // Example usage in range based `for` loop, where `one_through` returns an `Iterable_sequence_`:
    //
    //  for( const int v: one_through( 7 ) ) {
    //      printf( "%d ", v );
    //  }
    //  printf( "\n" );
    //
    template< class Yield_result >
    class Iterable_sequence_:
        public Basic_sequence_< Iterable_sequence_< Yield_result >, Yield_result >
    {
        using Base = Basic_sequence_< Iterable_sequence_, Yield_result >;

        Iterable_sequence_( in_<Iterable_sequence_> ) = delete;
        auto operator=( in_<Iterable_sequence_> ) = delete;

    public:
        using typename Base::Handle;
        Iterable_sequence_( const Handle h ): Base( h ) {}

        class Iterator
        {
            Iterable_sequence_*     m_p_generator;
            
        public:
            Iterator( const_<Iterable_sequence_*> p_generator = nullptr ): m_p_generator( p_generator ) {}
            
            auto operator*() const  -> Yield_result&    { return m_p_generator->value(); }
            auto operator++()       -> Iterator&        { m_p_generator->advance(); return *this; }
            
            auto is_at_end() const  -> bool             { return (m_p_generator == nullptr or m_p_generator->is_finished()); }

            friend
            auto operator==( in_<Iterator> a, in_<Iterator> b )
                -> bool
            { return (a.m_p_generator == b.m_p_generator or a.is_at_end() and b.is_at_end()); }
            
            friend
            auto operator!=( in_<Iterator> a, in_<Iterator> b ) -> bool { return not(a == b); }
        };
        
        auto begin()    -> Iterator { return Iterator( this ); }
        auto end()      -> Iterator { return Iterator(); }
    };

    template< class Yield_result >
    using Sequence_ = Iterable_sequence_< Yield_result >;
}  // namespace cpp_machinery::coroutine
