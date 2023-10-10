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

namespace ver_1
{
    // unconstrained template
    template <typename T>
    // requires (!Traits::is_pointer_v<T>)
    T max_value(T a, T b)
    {
        return (a < b) ? b : a;
    }

    // constrained template
    template <typename T>
        requires Traits::is_pointer_v<T>
    auto max_value(T a, T b)
    {
        assert(a != nullptr);
        assert(b != nullptr);
        return (*a < *b) ? *b : *a;
    }
} // namespace ver_1

template <typename T>
concept Pointer = Traits::is_pointer_v<T>;

namespace ver_2
{
    static_assert(Pointer<int*>);
    static_assert(!Pointer<int>);

    // unconstrained template
    template <typename T>
    T max_value(T a, T b)
    {
        return (a < b) ? b : a;
    }

    // constrained template
    template <typename T>
        requires Pointer<T>
    auto max_value(T a, T b)
    {
        assert(a != nullptr);
        assert(b != nullptr);
        return (*a < *b) ? *b : *a;
    }
} // namespace ver_2

namespace ver_3
{
    template <typename T>
    T max_value(T a, T b)
    {
        return (a < b) ? b : a;
    }

    // constrained template
    template <Pointer T>
    auto max_value(T a, T b)
    {
        assert(a != nullptr);
        assert(b != nullptr);
        return (*a < *b) ? *b : *a;
    }
} // namespace ver_3

namespace ver_4
{
    auto max_value(auto a, auto b)
    {
        return (a < b) ? b : a;
    }

    auto max_value(std::integral auto a, std::integral auto b)
    {
        return std::cmp_less(a, b) ? b : a;
    }

    namespace IsInterpreted
    {
        template <typename T1, typename T2>
        auto max_value(T1 a, T2 b)
        {
            return (a < b) ? b : a;
        }
    } // namespace IsInterpreted

    auto max_value(Pointer auto a, Pointer auto b)
    {
        assert(a != nullptr);
        assert(b != nullptr);
        return max_value(*a, *b);
    }

    namespace IsInterpreted
    {
        template <Pointer T1, Pointer T2>
        auto max_value(T1 a, T2 b)
        {
            assert(a != nullptr);
            assert(b != nullptr);
            return max_value(*a, *b);
        }
    } // namespace IsInterpreted
} // namespace ver_4

TEST_CASE("constraints")
{
    using namespace ver_4;

    int x = -10;
    int y = 20;

    CHECK(max_value(x, y) == 20);

    std::string str1 = "text";
    std::string str2 = "Text";

    CHECK(max_value(str1, str2) == "text");

    CHECK(max_value(&x, &y) == 20);

    unsigned int ux = 40;

    CHECK(max_value(x, ux) == 40);
}

TEST_CASE("concepts")
{
    REQUIRE(true);
}