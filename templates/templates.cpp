#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <source_location>
#include <bit>

using namespace std::literals;

TEST_CASE("templates & lambda expressions")
{
    auto printer1 = [](const std::ranges::range auto& coll) {
        std::cout << coll.size() << "\n";
    };

    auto printer2 = []<typename T>(const std::vector<T>& coll) {
        std::cout << coll.size() << " - " << coll.capacity() << "\n";
    };

    auto emplace_to_vec = []<typename T, typename... TArgs>(std::vector<T>& vec, TArgs&&... args) {
        vec.emplace_back(std::forward<TArgs>(args)...);
    };
}

TEST_CASE("default construction for lambda")
{
    auto cmp_by_value = [](auto a, auto b) {
        return *a < *b;
    };

    decltype(cmp_by_value) another_cmp_by_value; // Since C++20

    std::set<std::shared_ptr<int>, decltype(cmp_by_value)> my_set;

    my_set.insert(std::make_shared<int>(42));
    my_set.insert(std::make_shared<int>(2));
    my_set.insert(std::make_shared<int>(1));

    for (const auto& ptr : my_set)
        std::cout << *ptr << " ";
    std::cout << "\n";
}

template <typename T1, typename T2>
struct Values
{
    T1 fst;
    T2 snd;
};

TEST_CASE("aggregates")
{
    struct Person
    {
        int id;
        std::string name{"unknown"};
        double salary;
    };

    Person p1{1, "John", 10'000.0};
    Person p2{.id=2, .salary=20'000.0};
    // Person p3{.salary=20'000.0, .id=3}; // ERROR

    Person p3(4, "Adam", 10'000.0);

    auto ptr_p4 = std::make_shared<Person>(5, "Eva", 23'000);

    Values v1{1, "text"}; // CTAD works - default deduction guides

    auto sl = std::source_location::current();
 
    std::cout << "file: " << sl.file_name() << "\n";
    std::cout << "function: " << sl.function_name() << "\n";
    std::cout << "line/col: " << sl.line() << "\n";
}

template <size_t N>
concept PowerOf2 = std::has_single_bit(N);

template <typename T, size_t N>
    requires PowerOf2<N>
void use(std::array<T, N>& arr)
{
}

template <auto Factor>
auto scale(auto x)
{
    return x * Factor;
}

TEST_CASE("NTTP - double & float")
{
    CHECK(scale<2>(3) == 6);
    CHECK(scale<2.0>(3.14) == 6.28); // since C++20
}

struct Tax
{
    double value;
 
    constexpr Tax(double v)
        : value{v}
    {
        assert(v >= 0 && v < 1);
    }
};

template <Tax Vat>
double calc_gross_price(double net_price)
{
    return net_price + net_price * Vat.value;
}

TEST_CASE("NTTP - structs")
{
    constexpr Tax vat_pl{0.23};
    constexpr Tax vat_ger{0.19};

    CHECK(calc_gross_price<vat_pl>(100.0) == 123.0);
    CHECK(calc_gross_price<vat_ger>(100.0) == 119.0);
}

template <size_t N>
struct Str
{
    char value[N];

    constexpr Str(const char (&str)[N])
    {
        std::copy(str, str + N, value);
    }

    friend std::ostream& operator<<(std::ostream& out, const Str& str)
    {
        out << str.value;

        return out;
    }
};

template <Str Prefix>
class Logger
{
public:
    void log(const std::string& msg)
    {
        std::cout << Prefix << msg << "\n";
    }
};

TEST_CASE("strings as NTTP")
{
    Logger<">: "> logger1;
    logger1.log("start");
    logger1.log("end");
}

template <std::invocable auto GetVat>
double calc_gross_price(double net_price)
{
    return net_price + net_price * GetVat();
}

TEST_CASE("lambda as NTTP")
{
    CHECK(calc_gross_price<[]{ return 0.23; }>(100.0) == 123.0);

    constexpr static auto vat_ger = [] { return 0.19; };
    CHECK(calc_gross_price<vat_ger>(100.0) == 119.0);
}