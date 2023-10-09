#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <map>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

using namespace std::literals;

struct Vector
{
    double x, y;

    bool operator==(const Vector&) const = default;
};

Vector operator*(const Vector& v, double d)
{
    return Vector{v.x * d, v.y * d};
}

double operator*(const Vector& a, const Vector& b)
{
    return a.x;
}

template <typename T1, typename T2>
auto multiply(T1 a, T2 b)
{
    return a * b;
}

template <auto N>
auto create_buffer()
{
    if constexpr (N > 1024)
        return std::vector<std::byte>(N);
    else
        return std::array<std::byte, N>{};
}

TEST_CASE("multiply")
{
    CHECK(multiply(Vector{1, 1}, 2) == Vector{2, 2});
    CHECK(multiply(Vector{1, 1}, Vector{1, 0}) == 1.0);

    auto buffer1 = create_buffer<2048>();
    auto buffer2 = create_buffer<512>();
}

auto foo(int arg)
{
    return arg * 2.0;
}

auto my_cmp_less = [](const auto& a, const auto& b) {
    return a < b;
};

// since C++20
void print(const auto& obj, const auto& prefix)
{
    std::cout << "obj: " << obj << "\n";
}

namespace IsInterpreted
{
    template <typename T1, typename T2>
    void print(const T1& obj, const T2& prefix)
    {
        std::cout << prefix << ": " << obj << "\n";
    }
} // namespace IsInterpreted

template <typename F, typename... TArgs>
decltype(auto) call_wrapper(F f, TArgs&&... args)
{
    std::cout << "calling a function!!!" << std::endl;

    return f(std::forward<TArgs>(args)...); // perfect forwarding
}

namespace Cpp20
{
    decltype(auto) call_wrapper(auto f, auto&&... args)
    {
        std::cout << "Calling a function!!!\n";

        return f(std::forward<decltype(args)>(args)...);
    }
} // namespace Cpp20

TEST_CASE("functions with auto")
{
    print(42, "value"s);
    print("text"s, "value"s);
}

// mixed style
template <typename T>
void append(std::vector<T>& vec, auto&& item)
{
    vec.push_back(std::forward<decltype(item)>(item));
}

auto cmp_by_size_lambda = [](const auto& a, const auto& b) { return a.size() < b.size(); };

auto cmp_by_size_function(const auto& a, const auto& b) { return a.size() < b.size(); }

TEST_CASE("difference between generic lambda & function with auto params")
{
    std::vector<std::string> words = {"twenty-two", "zero", "one", "two", "three", "four"};

    std::ranges::sort(words, cmp_by_size_lambda);
    std::ranges::sort(words, cmp_by_size_function<std::string, std::string>);
}