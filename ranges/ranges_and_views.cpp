#include <catch2/catch_test_macros.hpp>
#include <helpers.hpp>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>
#include <list>

using namespace std::literals;

void use_range(std::ranges::range auto const&  rng)
{
    std::ranges::iterator_t<decltype(rng)> it{};
    
    std::vector<std::ranges::range_value_t> backup;
    std::ranges::copy(rng, std::back_inserter(backup));

    std::cout  << "void use_range(const auto&  rng)\n";
}

void use_range(std::ranges::bidirectional_range auto const&  rng)
{
    std::cout  << "void use_range(const std::ranges::bidirectional_range auto&  rng)\n";
}

void use_range(std::ranges::contiguous_range auto const&  rng)
{
    std::cout  << "void use_range(const std::ranges::contiguous_range auto&  rng)\n";
}

TEST_CASE("ranges - concepts")
{
    constexpr auto data = helpers::create_numeric_dataset<30>(42);

    static_assert(std::ranges::range<decltype(data)>);

    helpers::print(data, "data");

    use_range(data);

    REQUIRE(true);
}

template <auto Value_>
struct EndValue
{
    bool operator==(auto pos) const
    {
        return *pos == Value_;
    }
};

TEST_CASE("ranges - algorithms")
{
    SECTION("basics")
    {
        auto data = helpers::create_numeric_dataset<30>(42);

        std::sort(std::ranges::begin(data), std::ranges::end(data));

        helpers::print(data, "data");

        std::ranges::sort(data, std::greater{});

        helpers::print(data, "data");

        std::vector<int> backup(std::ranges::size(data));

        std::ranges::copy(data, backup.begin());
    }

    SECTION("projections")
    {
        std::vector words = {"twenty-two"s, "a"s, "abc"s, "b"s, "one"s, "aa"s};

        std::ranges::sort(words, std::less{}, /*projection*/ [](const std::string& str) { return str.size(); });

        helpers::print(words, "words sorted by size");
    }

    SECTION("iterators & sentinels")
    {
        std::vector data = {5, 423, 665, 1, 235, 42, 6, 345, 33, 665};

        std::ranges::sort(data.begin(), EndValue<42>{});
        std::ranges::sort(data.begin(), data.begin() + 5);

        helpers::print(data, "data sorted with sentinel");

        EndValue<'\0'> null_term;

        auto& txt = "acbgdef\0ajdhfgajsdhfgkasdjhfg"; // const char(&txt)[40]
        auto txt_array = std::to_array(txt); // std::array<const char, 40>

        std::ranges::sort(std::ranges::begin(txt_array), null_term, std::greater{});
        helpers::print(std::ranges::subrange(std::ranges::begin(txt_array), null_term), "txt_array fragment after sort");

        auto pos = std::ranges::find(data.begin(), std::unreachable_sentinel, 665);
        CHECK(*pos == 665);

        for (auto it = std::counted_iterator{std::cbegin(data), 5}; it != std::default_sentinel; ++it)
            std::cout << *it << ", ";
    }
}

TEST_CASE("views")
{
    std::list lst = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9, 10};

    SECTION("all")
    {
        auto all_items = std::views::all(lst);

        helpers::print(all_items, "all items");
    }

    SECTION("counted")
    {
        auto first_half = std::views::counted(lst.begin(), lst.size() / 2);

        for (auto& item : first_half)
            item *= 2;

        helpers::print(lst, "vec");
    }

    SECTION("iota")
    {
        auto my_range = std::views::iota(1, 20);

        helpers::print(my_range, "my_range");
    }

    SECTION("piping")
    {
        auto data = std::views::iota(1)
                        | std::views::take(20)
                        | std::views::filter([](int x) { return x % 2 == 0; })
                        | std::views::transform([](int x) { return x * x; })
                        | std::views::reverse;

        helpers::print(data, "data");
    }
}

namespace BeforeCpp20
{
    namespace ConstRef
    {
        template <typename T>
        void print(const T& coll)
        {
            for(const auto& item : coll)
                std::cout << item << " ";
            std::cout << "\n";
        }
    }    

    inline namespace UnversalReference
    {
        template <typename T>
        void print(T&& coll)
        {
            for(const auto& item : coll)
                std::cout << item << " ";
            std::cout << "\n";
        }
    }
}

namespace SinceCpp20
{
    template <std::ranges::view T>
    void print(T coll_view)
    {
        for(const auto& item : coll_view)
            std::cout << item << " ";
        std::cout << "\n";
    }
}

TEST_CASE("const& fiasco")
{
    std::list lst = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9, 10};
    
    BeforeCpp20::print(lst | std::views::filter([](int n) { return n % 2 == 0; }));

    auto head_lst = lst | std::views::take(5);

    SinceCpp20::print(head_lst);
}

std::vector<int> get_data()
{
    // std::vector vec{1, 2, 3, 42, 22};
    // return vec; // lvale - optional NRVO

    return {1, 2, 3, 42, 22}; // prvalue - must be RVO
}

TEST_CASE("borrowed iterators")
{
    SECTION("dangling iterator")
    {
        auto pos = std::ranges::find(get_data(), 42);

        //CHECK(*pos == 42); // Compilation ERROR - dangling iterator
    }

    SECTION("valid iterator")
    {
        auto&& data = get_data();
        auto pos = std::ranges::find(data, 42);

        REQUIRE(*pos == 42);
    }
}