#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").

namespace cpp_machinery {
    inline namespace type_builders {
        template< class Type >  using const_    = const Type;
        template< class Type >  using ref_      = Type&;
        template< class Type >  using in_       = const Type&;
    }  // inline namespace type_builders
}  // namespace cpp_machinery
