#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using namespace std::literals;

TEST_CASE("safe comparisons between integral numbers")
{
    int x = -7;
    unsigned int y = 42;

    // CHECK(x < y);
    CHECK(std::cmp_less(x, y));
    CHECK(std::cmp_greater_equal(y, x));

    CHECK(std::in_range<size_t>(-1) == false);
    CHECK(std::in_range<size_t>(665) == true);

    static_assert(std::in_range<size_t>(665));

    auto my_cmp_less = [](std::integral auto x, std::integral auto y) {
        return std::cmp_less(x, y);
    };

    std::vector<int> vec;
    CHECK(my_cmp_less(-7, vec.size()));

    // my_cmp_less("test"s, "text"s);
}

struct Point
{
    int x, y;

    Point(int x = 0, int y = 0)
        : x{x}
        , y{y}
    { }

    // bool operator==(const Point& other) const
    // {
    //     return std::tie(x, y) == std::tie(other.x, other.y);
    // }

    bool operator==(const Point&) const = default; // since C++20
};

struct NamedPoint : Point
{
    std::string name = "not-set";

    bool operator==(const NamedPoint&) const = default;
};

std::ostream& operator<<(std::ostream& out, const Point& pt)
{
    out << "Point(" << pt.x << ", " << pt.y << ")";
    return out;
}

TEST_CASE("operator ==")
{
    Point pt{10, 20};

    CHECK(pt == Point(10, 20));
    CHECK(pt != Point(40, 20)); // rewriting: !(pt == Point(40, 20)

    Point x_versor{1};    // Point(1, 0)
    CHECK(x_versor == 1); // x_versor == Point(1, 0) -> x_versor.operator==(Point(1, 0))
    CHECK(1 == x_versor); // rewriting: x_versor == 1 -> x_versor.operator==(Point(1, 0))

    NamedPoint npt1{{10, 20}, "npt1"};

    CHECK(npt1 == NamedPoint{{10, 20}, "npt1"});
    CHECK(npt1 != NamedPoint{{10, 20}, "npt2"});
}

struct Number
{
    int value;

    auto operator<=>(const Number& other) const = default;

    bool operator==(const Number&) const = default; // implicitly declared when auto operator<=>(const Number&) const = default;
};

TEST_CASE("order")
{
    std::vector<Number> vec = {Number(10), Number(1), Number(5)};

    std::ranges::sort(vec);

    CHECK(std::ranges::is_sorted(vec));

    CHECK(Number{10} > Number(2));
    CHECK(Number{10} >= Number(10));
    CHECK(Number{10} < Number(665));
    CHECK(Number{665} <= Number(665));

    CHECK(Number{} == Number(0));
    CHECK(Number{} != Number(1));
}

struct Person
{
    std::string name;
    uint8_t age;

    auto operator<=>(const Person&) const = default;
};

TEST_CASE("many members and <=>")
{
    CHECK(Person{"Jan", 33} > Person{"Jan", 23});
    CHECK(Person{"Jan", 33} >= Person{"Jan", 23});
    CHECK(Person{"Jan", 33} < Person{"Janek", 33});
}

TEST_CASE("operator <=>")
{
    SECTION("result can be compared with zero")
    {
        if (4 <=> 4 == 0)
        {
            std::cout << "4 equals 4\n";
        }

        if (3 <=> 4 < 0)
        {
            std::cout << "3 is less 4\n";
        }

        if (7 <=> 4 > 0)
        {
            std::cout << "7 is greater 4\n";
        }
    }

    SECTION("result is comparison_category.value")
    {
        SECTION("strong_ordering")
        {
            CHECK(4 <=> 4 == std::strong_ordering::equal);
        }

        SECTION("partial_ordering")
        {
            CHECK(4.13 <=> 3.14 == std::partial_ordering::greater);
            CHECK(4.13 <=> 4.13 == std::partial_ordering::equivalent);
        }
    }
}