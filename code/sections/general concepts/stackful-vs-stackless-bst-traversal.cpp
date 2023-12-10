#include <initializer_list>     // For curly braces list in range based `for`.
#include <functional>
#include <stack>
#include <utility>

namespace cppm {    // cpp machinery
    using   std::stack,         // <stack>
            std::move;          // <utility>

    template< class Type >  using const_    = const Type;
    template< class Type >  using in_       = const Type&;
    
    template< class Type > struct Non_deduced_t_ { using T = Type; };
    
    template< class Type >
    using Non_deduced_ = typename Non_deduced_t_<Type>::T;

    template< class Type >
    auto a_( Non_deduced_<Type&> o )        -> Type& { return o; }
    
    template< class Type >
    auto a_( Non_deduced_<const Type&> o )  -> const Type& { return o; }
    
    template< class Type >
    auto a_( Non_deduced_<Type&&> o )       -> Type&& { return move( o ); }
    
    template< class Type >
    auto popped_top_of( stack<Type>& st )
        -> Type
    {
        const auto result = a_<Type>( st.top() );
        st.pop();
        return result;
    }
    
    template< class Type >
    auto is_empty( in_<Type> c ) -> bool { return c.empty(); }
}  // namespace cppm

namespace bst {
    using   cppm::const_, cppm::in_, cppm::a_, cppm::popped_top_of, cppm::is_empty;
    using   std::function,          // <functional>
            std::stack;             // <stack>

    struct Node{ int value; Node* left; Node* right; };
    
    void insert( const int new_value, Node*& root )
    {
        const_<Node*> new_node = new Node{ new_value };
        if( not root ) {
            root = new_node;
            return;
        }
        Node* current = root;
        for( ;; ) {
            Node*& child = (new_value < current->value? current->left : current->right);
            if( not child ) {
                child = new_node;
                break;
            } else {
                current = child;
            }
        }
    }

    void recursive_for_each(
        const_<Node*>               root,
        in_<function<void(int)>>    consume
        )
    {
        if( root ) {
            recursive_for_each( root->left, consume );
            consume( root->value );
            recursive_for_each( root->right, consume );
        }
    }

    void iterative_for_each(
        const_<Node*>               root,
        in_<function<void(int)>>    consume
        )
    {
        if( not root ) { return; }
        enum class Heading{ down, up_from_left, up_from_right };
        auto    heading     = Heading::down;
        auto    current     = a_<Node*>( root );
        auto    parents     = stack<Node*>();
        for( ;; ) {
            switch( heading ) {
                case Heading::down: {
                    if( current->left ) {
                        parents.push( current );
                        current = current->left;
                    } else {
                        heading = Heading::up_from_left;
                    }
                    break;
                }
                case Heading::up_from_left: {
                    consume( current->value );
                    if( current->right ) {
                        parents.push( current );
                        current = current->right;
                        heading = Heading::down;
                    } else {
                        heading = Heading::up_from_right;
                    }
                    break;
                }
                case Heading::up_from_right: {
                    if( is_empty( parents ) ) {
                        return;
                    } else {
                        const auto parent = a_<Node*>( popped_top_of( parents ) );
                        heading = (current == parent->left?
                            Heading::up_from_left : Heading::up_from_right
                            );
                        current = parent;
                    }
                    break;
                }
            }
        }
    }
}  // namespace bst

#include <stdio.h>
auto main() -> int
{
    bst::Node* root = nullptr;
    for( const int v: {4, 2, 1, 3, 6, 5, 7} ) { bst::insert( v, root ); }

    bst::recursive_for_each( root, []( const int v ) { printf( "%d ", v ); } );
    printf( "\n" );

    bst::iterative_for_each( root, []( const int v ) { printf( "%d ", v ); } );
    printf( "\n" );
    
    printf( "Finished.\n" );
}
