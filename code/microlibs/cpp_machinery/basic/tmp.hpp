#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").

namespace cpp_machinery {
    inline namespace tmp {
        template< class Type > struct Non_deduced_t_ { using T = Type; };
        template< class Type > using Non_deduced_ = typename Non_deduced_t_<Type>::T;
    }  // inline namespace tmp
}  // namespace cpp_machinery
