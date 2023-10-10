#include <array>
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
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

        if (ptr != nullptr)
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

template <typename TRange>
concept PrintableRange = requires(std::ranges::range_value_t<TRange> item) {
    std::cout << item;
};

template <PrintableRange TContainer>
void print(const TContainer& c, std::string_view prefix = "items")
{
    std::cout << prefix << ": [ ";
    for (const auto& item : c)
        std::cout << item << " ";
    std::cout << "]\n";
}

namespace AlternativeTake
{
    template <typename TContainer>
        requires                                                                         // requires clause
            requires(std::ranges::range_value_t<TContainer> item) { std::cout << item; } // requires expression
    void print(const TContainer& c, std::string_view prefix = "items")
    {
        std::cout << prefix << ": [ ";
        for (const auto& item : c)
            std::cout << item << " ";
        std::cout << "]\n";
    }
} // namespace AlternativeTake

TEST_CASE("printing")
{
    std::vector<int> vec{1, 2, 3};
    print(vec);

    int data[10] = {1, 2, 3};
    print(data, "data");
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

// template <typename T>
// concept Pointer = Traits::is_pointer_v<T>;

template <typename T>
concept Pointer = requires(T ptr) {
    *ptr;
    ptr == nullptr;
};

static_assert(Pointer<std::shared_ptr<int>>);

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

    std::integral auto max_value(std::integral auto a, std::integral auto b)
    {
        return std::cmp_less(a, b) ? b : a;
    }

    namespace AlternativeTake
    {
        std::integral auto max_value(std::integral auto a, std::integral auto b)
            requires std::same_as<decltype(a), decltype(b)> // trailing requires clause
        {
            return std::cmp_less(a, b) ? b : a;
        }
    } // namespace AlternativeTake

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

    auto ptr1 = std::make_shared<int>(42);
    auto ptr2 = std::make_shared<int>(665);

    CHECK(max_value(ptr1, ptr2) == 665);
}

template <typename T>
struct Wrapper
{
    T value;

    void print() const
    {
        std::cout << "value: " << value << "\n";
    }

    void print() const
        requires std::ranges::range<T>
    {
        std::cout << "values: [ ";
        for (const auto& item : value)
            std::cout << item << " ";
        std::cout << "]\n";
    }
};

size_t get_id()
{
    static size_t id = 0;
    return ++id;
}

TEST_CASE("concepts")
{
    Wrapper wrapped_int{42};
    wrapped_int.print();

    Wrapper wrapped_vec{std::vector{1, 2, 3}};
    wrapped_vec.print();

    std::convertible_to<uint64_t> auto id = get_id();
}

// requires expression

template <typename T>
concept Addable = requires(T a) {
    a + a; // simple requirement
};

template <typename T>
concept BigType = requires(T obj) {
    requires sizeof(obj) > 32; // nested requirement
};

template <typename TContainer>
concept ContainerWithIterators = requires(TContainer container) {
    typename TContainer::iterator; // type requirement
    typename TContainer::const_iterator;
    {
        container.begin()
    } -> std::input_or_output_iterator; // std::input_or_output_iterator<decltype((container.begin())); - compound requirement
    {
        container.end()
    } -> std::input_or_output_iterator;
};

template <typename T>
concept AdditiveRange = requires(T&& c) {
    std::ranges::begin(c); // simple requirement
    std::ranges::end(c);
    typename std::ranges::range_value_t<T>;                        // type requirement
    requires requires(std::ranges::range_value_t<T> x) { x + x; }; // nested requirement
};

template <AdditiveRange Rng>
auto sum(const Rng& data)
{
    return std::accumulate(std::begin(data), std::end(data),
        std::ranges::range_value_t<Rng>{});
}

TEST_CASE("requires expression")
{
    static_assert(Addable<std::string>);
    static_assert(!Addable<std::vector<int>>);

    static_assert(!BigType<char>);
    static_assert(BigType<std::array<char, 1024>>);

    static_assert(ContainerWithIterators<std::vector<int>>);

    std::vector<int> vec = {1, 2, 3};

    CHECK(sum(vec) == 6);
}

template <typename TContainer, typename TValue>
void add_to_container(TContainer& container, TValue&& value)
{
    if constexpr (requires { container.push_back(std::forward<TValue>(value)); })
        container.push_back(std::forward<TValue>(value));
    else
        container.insert(std::forward<TValue>(value));
}

TEST_CASE("add to container")
{
    std::vector<int> vec;
    add_to_container(vec, 42);
    CHECK(vec == std::vector{42});

    std::set<int> set_ints;
    add_to_container(set_ints, 42);
}

//////////////////////////////////////////////
// concept subsumation

struct BoundingBox
{
    int w, h;
};

struct Color
{
    uint8_t r, g, b;
};

template <typename T>
concept Shape = requires(T obj)
{
    { obj.box() } -> std::same_as<BoundingBox>;
    obj.draw();
};

template <typename T>
concept ShapeWithColor = Shape<T> && requires(T shp, Color c) {
    shp.set_color(c);
    { shp.get_color() } -> std::same_as<Color>;
};

template <Shape T>
void render(const T& shp)
{
    std::cout << "template <Shape T> void render(const T& shp)\n";
    shp.draw();
}

template <ShapeWithColor T>
void render(T&& shp)
{
    std::cout << "template <ShapeWithColor T> void render(const T& shp)\n";
    shp.set_color({0, 0, 0});
    shp.draw();
}

struct Rect
{
    int w, h;
    Color color;

    void draw() const
    {
        std::cout << "Rect::draw()\n";
    }

    BoundingBox box() const
    {
        return BoundingBox{w, h};
    }

    void set_color(Color c)
    {
        std::cout << "Setting color\n";
        color = c;
    }

    Color get_color() const
    {
        return color;
    }
};

TEST_CASE("subsuming concepts")
{
    render(Rect{100, 200});
}

//////////////////////////////////////////////////////////

template <typename T>
concept SignedIntegral = std::integral<T> && std::is_signed_v<T>; // more constrained than integral<T>

template <typename T>
concept Arithmetic = std::integral<T> || std::floating_point<T>;  // less constrained than integral<T>

template <std::integral T>
void foo(T value)
{
    std::cout << "template <typename T> requires std::is_integral_v<T> void foo(T value)\n";
}

template <SignedIntegral T>
void foo(T value)
{
    std::cout << "template <typename T> requires std::is_integral_v<T> void foo(T value)\n";
}

TEST_CASE("subsuming requires concepts")
{
    foo(42);
}