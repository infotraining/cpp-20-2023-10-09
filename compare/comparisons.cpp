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

// bool operator==(const Point& p1, const Point& p2)
// {
//     return std::tie(p1.x, p1.y) == std::tie(p2.x, p2.y);
// }

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

    std::strong_ordering operator<=>(const Number& other) const = default;

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

struct FloatNumber
{
    double value;

    bool operator==(const FloatNumber&) const = default;

    auto operator<=>(const FloatNumber& other) const = default;
};

namespace TotalOrder
{
    struct FloatNumber
    {
        double value;

        bool operator==(const FloatNumber&) const = default;

        std::strong_ordering operator<=>(const FloatNumber& other) const
        {
            return std::strong_order(value, other.value);
        }
    };

} // namespace TotalOrder

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
            CHECK(5 <=> 4 == std::strong_ordering::greater);
            CHECK(3 <=> 4 == std::strong_ordering::less);
        }

        SECTION("partial_ordering")
        {
            CHECK(4.13 <=> 3.14 == std::partial_ordering::greater);
            CHECK(4.13 <=> 4.13 == std::partial_ordering::equivalent);
            CHECK(3.44 <=> std::numeric_limits<double>::quiet_NaN() == std::partial_ordering::unordered);

            CHECK(FloatNumber{3.13} < FloatNumber{3.14}); //  (FloatNumber{3.13} <=> FloatNumber{3.14}) < 0
            CHECK(FloatNumber{3.13} <=> FloatNumber{3.14} == std::partial_ordering::less);
        }
    }
}

struct IntNan
{
    std::optional<int> value;

    bool operator==(const IntNan& rhs) const
    {
        if (!value || !rhs.value)
        {
            return false;
        }
        return *value == *rhs.value;
    }

    std::partial_ordering operator<=>(const IntNan& rhs) const
    {
        if (!value || !rhs.value)
        {
            return std::partial_ordering::unordered;
        }

        return *value <=> *rhs.value; // std::strong_ordering is implicitly converted to std::partial_ordering
    }
};

TEST_CASE("IntNan - comparisons")
{
    auto result = IntNan{2} <=> IntNan{4};
    CHECK(result == std::partial_ordering::less);

    result = IntNan{2} <=> IntNan{};
    CHECK(result == std::partial_ordering::unordered);

    CHECK(IntNan{4} == IntNan{4});
    CHECK(IntNan{4} != IntNan{5});

    result = IntNan{4} <=> IntNan{4};
    CHECK(result == std::partial_ordering::equivalent);
}

TEST_CASE("std::strong_order & NaN")
{
    std::vector<double> numbers_with_nan = { 3.14, 5.0, std::numeric_limits<double>::quiet_NaN() , 1.11 };

    std::sort(numbers_with_nan.begin(), numbers_with_nan.end(), [](auto a, auto b) { return std::strong_order(a, b) < 0; });
}

struct Base
{
    std::string value;

    bool operator==(const Base& other) const { return value == other.value; }
    bool operator<(const Base& other) const { return value < other.value; }
};

struct Derived : Base
{
    std::vector<int> data;
 
    std::strong_ordering operator<=>(const Derived& other) const = default;
};

TEST_CASE("default <=> - how it works")
{
    Derived d1{{"text"}, {1, 2, 3}};
    Derived d2{{"text"}, {1, 2, 4}};

    CHECK(d1 < d2);
}

struct Human
{
    std::string name;
    int how_old;
    double height; 

    bool operator==(const Human& rhs) const 
    {
        return std::tie(name, how_old) == std::tie(rhs.name, rhs.how_old);
    }

    std::strong_ordering operator<=>(const Human& rhs) const
    {
        // if (auto cmp_result = name <=> rhs.name; cmp_result = 0)
        // {
        //     return how_old <=> rhs.how_old;
        // }
        // else
        // {
        //     return cmp_result;
        // }

        return std::tie(name, how_old) <=> std::tie(rhs.name, rhs.how_old);
    }

    auto operator<=>(const Person& p) const
    {
        return std::tie(name, how_old) <=> std::tie(p.name, p.age);
    }
};

TEST_CASE("Human - comparing")
{
    Human john1{"John", 33, 178.8};
    Human john2{"John", 33, 168.8};

    CHECK(john1 == john2);
    CHECK(john1 <= john2);
    CHECK(john1 > Human{"John", 22});

    CHECK(john1 < Person{"John", 34});
}

////////////////////////////////////////////

struct Data
{
    int* buffer_;
    std::size_t size_;
public:
    Data(std::initializer_list<int> lst) : buffer_{new int[lst.size()]}, size_{lst.size()}
    {
        std::ranges::copy(lst, buffer_);
    }

    Data(const Data&) = delete;
    Data& operator=(const Data&) = delete;

    bool operator==(const Data& other) const 
    {
        return size_ == other.size_ && std::equal(buffer_, buffer_ + size_, other.buffer_);
    }

    auto operator<=>(const Data& other) const
    {
        return std::lexicographical_compare_three_way(buffer_, buffer_ + size_, other.buffer_, other.buffer_ + other.size_);
    }

    ~Data()
    {
        delete[] buffer_;
    }
};

TEST_CASE("Data - comparisons")
{
    Data ds1{1, 2, 3};
    Data ds2{1, 2, 3};
    Data ds3{1, 2, 4};

    CHECK(ds1 == ds2);
    CHECK(ds1 != ds3);

    CHECK(ds1 < ds3);
}