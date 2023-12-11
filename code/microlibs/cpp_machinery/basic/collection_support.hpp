#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <cpp_machinery/basic/a_.hpp>                   // a_
#include <cpp_machinery/basic/type_builders.hpp>        // in_, ref_

#include <stack>
#include <utility>

namespace cpp_machinery {
    inline namespace collection_support {
        using   std::stack,         // <stack>
                std::move;          // <utility>

        template< class Type >
        auto popped_top_of( ref_<stack<Type>> st )
            -> Type
        {
            const auto result = move( a_<Type>( st.top() ) );
            st.pop();
            return result;
        }
        
        template< class Type >
        auto is_empty( in_<Type> c ) -> bool { return c.empty(); }
    }  // inline namespace collection_support
}  // namespace cpp_machinery
