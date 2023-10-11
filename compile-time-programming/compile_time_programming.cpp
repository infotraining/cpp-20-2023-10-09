#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <array>
#include <algorithm>
#include <ranges>
#include <numeric>

using namespace std::literals;

int runtime_func(int x)
{
    return x * x;
}

constexpr int constexpr_func(int x)
{
    return x * x;
}

consteval int consteval_func(int x)
{
    return x * x;
}

TEST_CASE("types of functions")
{
    SECTION("runtime")
    {
        int x = 42;

        auto result = runtime_func(x);
    }

    SECTION("constexpr")
    {
        constexpr auto result = constexpr_func(42);

        int x = 42;
        auto runtime_result = constexpr_func(x);
    }

    SECTION("consteval")
    {
        constexpr auto result = consteval_func(42);

        int runtime_variable = consteval_func(42);
    }
}

void compile_time_error() // runtime function
{ }

consteval int next_two_digit_value(int value)
{
    if (value < 9 || value >= 99)
    {
        compile_time_error();
    }

    return ++value;
}

TEST_CASE("consteval")
{
    CHECK(next_two_digit_value(50) == 51);
    //CHECK(next_two_digit_value(100) == 101); // ERROR

    auto square = [](int x) consteval { return x * x; };

    constexpr std::array arr = { square(1), square(2), square(1024) };
}

constexpr int len(const char* s)
{
    if (std::is_constant_evaluated())
    {
        // compile-time friendly code
        int idx = 0;
        while (s[idx] != '\0')
            ++idx;
        return idx;
    }
    else
    {
       return std::strlen(s); // function called at runtime
    }
}

TEST_CASE("is_constant_evaluated")
{
    constexpr auto size1 = len("abc");

    int size2 = len("abcd");
}

consteval int get_id()
{
    return 665;
}

constinit int i = get_id();

struct MyType
{
    static inline constinit long id = get_id();
};

TEST_CASE("constinit")
{
    ++i;

    CHECK(MyType::id == 665);
}

template <size_t N>
constexpr auto create_powers()
{
    std::array<uint32_t, N> powers{};

    std::iota(powers.begin(), powers.end(), 1); // std algorithms are constexpr

    std::ranges::transform(powers, powers.begin(), [](int x) { return x * x; }); // ranges algorithms are constexpr

    return powers;
}

TEST_CASE("constexpr extensions")
{
    constexpr auto powers_lookup_table = create_powers<100>();

    REQUIRE(true);
}

template <std::ranges::input_range... TRng_>
constexpr auto avg_for_unique(const TRng_&... rng)
{
    using TElement = std::common_type_t<std::ranges::range_value_t<TRng_>...>;

    std::vector<TElement> vec;                            // empty vector
    vec.reserve((rng.size() + ...));                      // reserve a buffer - fold expression C++17
    (vec.insert(vec.end(), rng.begin(), rng.end()), ...); // fold expression C++17

    // sort items
    std::ranges::sort(vec); // std::sort(vec.begin(), vec.end());

    // create span of unique_items
    auto new_end = std::unique(vec.begin(), vec.end());
    std::span unique_items{vec.begin(), new_end};

    // calculate sum of unique items
    auto sum = std::accumulate(unique_items.begin(), unique_items.end(), TElement{});

    return sum / static_cast<double>(unique_items.size());
}

TEST_CASE("avg for unique")
{
    constexpr std::array lst1 = {1, 2, 3, 4, 5};
    constexpr std::array lst2 = {5, 6, 7, 8, 9};

    constexpr auto avg = avg_for_unique(lst1, lst2);

    std::cout << "AVG: " << avg << "\n";
}