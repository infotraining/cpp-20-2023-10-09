#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std::literals;

namespace Traits
{
    // generic template
    template <typename T>
    struct is_void
    {
        static constexpr bool value = false;
    };

    // full template specialization
    template <>
    struct is_void<void>
    {
        static constexpr bool value = true;
    };

    // template variable
    template <typename T>
    constexpr static bool is_void_v = is_void<T>::value;

    ///////////////////////////////////////////////////

    template <typename T>
    struct is_pointer
    {
        static constexpr bool value = false;
    };

    template <typename T>
    struct is_pointer<T*>
    {
        static constexpr bool value = true;
    };

    // template variable
    template <typename T>
    constexpr static bool is_pointer_v = is_pointer<T>::value;

    ///////////////////////////////////////////////////////

    template <typename T>
    struct remove_const
    {
        using type = T;
    };

    template <typename T>
    struct remove_const<const T>
    {
        using type = T;
    };

    template <typename T>
    using remove_const_t = typename remove_const<T>::type;

} // namespace Traits

namespace BeforeCpp20
{
    template <typename T>
    void use(T ptr)
    {
        static_assert(Traits::is_pointer_v<T>, "T must be pointer type");

        if (ptr)
            std::cout << *ptr << "\n";
    }
} // namespace BeforeCpp20

TEST_CASE("using traits")
{
    using U = void;
    using V = int;

    static_assert(Traits::is_void<U>::value);
    static_assert(Traits::is_void_v<U>);
    static_assert(Traits::is_void<V>::value == false);

    using X = int;
    using Y = int*;

    static_assert(Traits::is_pointer<Y>::value);
    static_assert(!Traits::is_pointer<X>::value);

    using M = const int;

    Traits::remove_const_t<M> m;
}

template <typename TContainer>
void print(const TContainer& c, std::string_view prefix = "items")
{
    std::cout << prefix << ": [ ";
    for (const auto& item : c)
        std::cout << item << " ";
    std::cout << "]\n";
}

TEST_CASE("constraints")
{
    print(std::vector{1, 2, 3}, "vec");
    REQUIRE(true);
}

TEST_CASE("concepts")
{
    REQUIRE(true);
}