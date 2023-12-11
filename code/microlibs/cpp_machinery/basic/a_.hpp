#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <cpp_machinery/basic/tmp.hpp>      // Non_deduced_

#include <utility>

namespace cpp_machinery {
    using   std::move;              // <utility>

    template< class Type >
    auto a_( Non_deduced_<Type&> o )        -> Type&        { return o; }
    
    template< class Type >
    auto a_( Non_deduced_<const Type&> o )  -> const Type&  { return o; }
    
    template< class Type >
    auto a_( Non_deduced_<Type&&> o )       -> Type&&       { return move( o ); }
}  // namespace cpp_machinery
