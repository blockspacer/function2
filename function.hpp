
//              Copyright Denis Blank 2015
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef function_hpp__
#define function_hpp__

#include <tuple>
#include <memory>
#include <type_traits>

namespace my
{

namespace detail
{

namespace unwrap_traits
{
    template<
        typename /*DecayedFunction*/,
        bool /*IsMember*/,
        bool /*IsConst*/,
        bool /*IsVolatile*/>
    struct unwrap_trait_base;

    /// Unwrap base
    template<
        typename ReturnType, typename... Args,
        bool IsMember,
        bool IsConst,
        bool IsVolatile>
    struct unwrap_trait_base<ReturnType(Args...), IsMember, IsConst, IsVolatile>
    {
        ///  The decayed type of the function without qualifiers.
        using decayed_type = ReturnType(Args...);

        /// The return type of the function.
        using return_type = ReturnType;

        /// The argument types of the function as pack in std::tuple.
        using argument_type = std::tuple<Args...>;

        /// Is true if the given function is a member function.
        static constexpr bool is_member = IsMember;

        /// Is true if the given function is const.
        static constexpr bool is_const = IsConst;

        /// Is true if the given function is volatile.
        static constexpr bool is_volatile = IsVolatile;
    };

    template<typename ClassType>
    struct class_trait_base
    {
        /// Class type of the given function.
        using class_type = ClassType;
    };

    /// Function unwrap trait
    template<typename /*Fn*/>
    struct unwrap_trait;

    /// Function
    template<typename ReturnType, typename... Args>
    struct unwrap_trait<ReturnType(Args...)>
        : unwrap_trait_base<ReturnType(Args...), false, false, false> { };

    /// Const function
    template<typename ReturnType, typename... Args>
    struct unwrap_trait<ReturnType(Args...) const>
        : unwrap_trait_base<ReturnType(Args...), false, true, false> { };

    /// Volatile function
    template<typename ReturnType, typename... Args>
    struct unwrap_trait<ReturnType(Args...) volatile>
        : unwrap_trait_base<ReturnType(Args...), false, false, true> { };

    /// Const volatile function
    template<typename ReturnType, typename... Args>
    struct unwrap_trait<ReturnType(Args...) const volatile>
        : unwrap_trait_base<ReturnType(Args...), false, true, true> { };

    /// Function pointer
    template<typename ReturnType, typename... Args>
    struct unwrap_trait<ReturnType(*)(Args...)>
        : unwrap_trait_base<ReturnType(Args...), false, false, false> { };

    /// Const function pointer
    template<typename ReturnType, typename... Args>
    struct unwrap_trait<ReturnType(*const)(Args...)>
        : unwrap_trait_base<ReturnType(Args...), false, true, false> { };

    /// Volatile function pointer
    template<typename ReturnType, typename... Args>
    struct unwrap_trait<ReturnType(*volatile)(Args...)>
        : unwrap_trait_base<ReturnType(Args...), false, false, true> { };

    /// Const volatile function pointer
    template<typename ReturnType, typename... Args>
    struct unwrap_trait<ReturnType(*const volatile)(Args...) >
        : unwrap_trait_base<ReturnType(Args...), false, true, true> { };

    /// Class method pointer
    template<typename ClassType, typename ReturnType, typename... Args>
    struct unwrap_trait<ReturnType(ClassType::*)(Args...)>
        : unwrap_trait_base<ReturnType(Args...), true, false, false>,
          class_trait_base<ClassType> { };

    /// Const class method pointer
    template<typename ClassType, typename ReturnType, typename... Args>
    struct unwrap_trait<ReturnType(ClassType::*)(Args...) const>
        : unwrap_trait_base<ReturnType(Args...), true, true, false>,
          class_trait_base<ClassType> { };

    /// Volatile class method pointer
    template<typename ClassType, typename ReturnType, typename... Args>
    struct unwrap_trait<ReturnType(ClassType::*)(Args...) volatile>
        : unwrap_trait_base<ReturnType(Args...), true, false, true>,
        class_trait_base<ClassType> { };

    /// Const volatile class method pointer
    template<typename ClassType, typename ReturnType, typename... Args>
    struct unwrap_trait<ReturnType(ClassType::*)(Args...) const volatile>
        : unwrap_trait_base<ReturnType(Args...), true, true, true>,
        class_trait_base<ClassType> { };

} // namespace unwrap_traits

template<typename /*Fn*/>
struct move_only_wrapper_impl;

template<typename ReturnType, typename... Args>
struct move_only_wrapper_impl<ReturnType(Args...)>
{
    virtual ~move_only_wrapper_impl() { }

    virtual ReturnType operator() (Args&&...) = 0;

}; // struct move_only_wrapper_impl

template<typename /*Fn*/>
struct wrapper_impl;

template<typename ReturnType, typename... Args>
struct wrapper_impl<ReturnType(Args...)>
    : move_only_wrapper_impl<ReturnType(Args...)>
{
    virtual ~wrapper_impl() { }

    // virtual wrapper_impl* clone() = 0;

}; // struct wrapper_impl

template<typename /*Fn*/>
struct fake_wrapper_impl;

template<typename ReturnType, typename... Args>
struct fake_wrapper_impl<ReturnType(Args...)>
    : wrapper_impl<ReturnType(Args...)>
{
    ReturnType operator() (Args&&...) override
    {
        return ReturnType();
    }

}; // struct fake_wrapper_impl

template<typename /*Fn*/, bool /*Copyable*/, bool /*Constant*/, bool /*Volatile*/>
class function;

template <typename /*Base*/>
struct call_operator;

template <typename ReturnType, typename... Args, bool Copyable>
struct call_operator<function<ReturnType(Args...), Copyable, false, false>>
{
    using func = function<ReturnType(Args...), Copyable, false, false>;

    ReturnType operator()(Args... args)
    {
        return (*static_cast<func*>(this)->_impl)(std::forward<Args>(args)...);
    }
};

template <typename ReturnType, typename... Args, bool Copyable>
struct call_operator<function<ReturnType(Args...), Copyable, true, false>>
{
    using func = function<ReturnType(Args...), Copyable, true, false>;

    ReturnType operator()(Args... /*args*/) const
    {
        return ReturnType();
        // FIXME return (*static_cast<const func*>(this)->_impl)(std::forward<Args>(args)...);
    }
};

template <typename ReturnType, typename... Args, bool Copyable>
struct call_operator<function<ReturnType(Args...), Copyable, false, true>>
{
    using func = function<ReturnType(Args...), Copyable, false, true>;

    ReturnType operator()(Args... /*args*/) volatile
    {
        return ReturnType();
        // FIXME return (*static_cast<volatile func*>(this)->_impl)(std::forward<Args>(args)...);
    }
};

template <typename ReturnType, typename... Args, bool Copyable>
struct call_operator<function<ReturnType(Args...), Copyable, true, true>>
{
    using func = function<ReturnType(Args...), Copyable, true, true>;

    ReturnType operator()(Args... /*args*/) const volatile
    {
        return ReturnType();
        // FIXME return (*static_cast<const volatile func*>(this)->_impl)(std::forward<Args>(args)...);
    }
};

template<typename ReturnType, typename... Args, bool Copyable, bool Constant, bool Volatile>
class function<ReturnType(Args...), Copyable, Constant, Volatile>
    : public call_operator<function<ReturnType(Args...), Copyable, Constant, Volatile>>
{
    std::unique_ptr<wrapper_impl<ReturnType(Args...)>> _impl;

    friend struct call_operator<function>;

public:
    function()
        : _impl(std::make_unique<fake_wrapper_impl<ReturnType(Args...)>>()) { }

    template<typename T>
    function(T&&)
        : _impl(std::make_unique<fake_wrapper_impl<ReturnType(Args...)>>()) { }

    using call_operator<function>::operator();    

}; // class function

template<typename Signature, bool Copyable>
using function_base = function<
    typename unwrap_traits::unwrap_trait<Signature>::decayed_type,
    Copyable,
    unwrap_traits::unwrap_trait<Signature>::is_const,
    unwrap_traits::unwrap_trait<Signature>::is_volatile
>;

} // namespace detail

/// Copyable function wrapper
template<typename Signature>
using function = detail::function_base<Signature, true>;

/// Non copyable function wrapper
template<typename Signature>
using non_copyable_function = detail::function_base<Signature, false>;

/// Creates a function object from the given parameter
template<typename Fn>
auto make_function(Fn&& functional)
{
    return detail::function<
        detail::unwrap_traits::unwrap_trait<decltype(std::declval<Fn>(), &Fn::operator())>::decayed_type,
        std::is_copy_assignable<std::decay_t<Fn>>::value &&
        std::is_copy_constructible<std::decay_t<Fn>>::value,
        detail::unwrap_traits::unwrap_trait<decltype(std::declval<Fn>(), &Fn::operator())>::is_const,
        detail::unwrap_traits::unwrap_trait<decltype(std::declval<Fn>(), &Fn::operator())>::is_volatile
    >(std::forward<Fn>)(functional);
}

} // namespace my

#endif // function_hpp__
