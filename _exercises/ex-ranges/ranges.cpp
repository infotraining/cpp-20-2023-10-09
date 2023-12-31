#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <set>
#include <list>
#include <source_location>
#include <ranges>
#include <helpers.hpp>

template <typename T1, typename T2>
std::ostream& operator<<(std::ostream& out, const std::pair<T1, T2>& p)
{
    out << "{" << p.first << "," << p.second << "}";
    return out;
}

#include <catch2/catch_test_macros.hpp>

using namespace std::literals;

std::pair<std::string_view, std::string_view> split(std::string_view line, std::string_view separator = "/")
{
    if (auto pos = line.find(separator); pos == std::string_view::npos)
        return {line, ""sv};
    else
        return {line.substr(0, pos), line.substr(pos + 1)};
}

TEST_CASE("split")
{
    std::string s1 = "324/44";
    CHECK(split(s1) == std::pair{"324"sv, "44"sv});

    std::string s2 = "4343";
    CHECK(split(s2) == std::pair{"4343"sv, ""sv});

    std::string s3 = "345/";
    CHECK(split(s3) == std::pair{"345"sv, ""sv});

    std::string s4 = "/434";
    CHECK(split(s4) == std::pair{""sv, "434"sv});

    SECTION("spliting string")
    {
        const char* text = "abc def ghi";

        std::vector<std::string_view> tokens;

        auto splitted_ranges = std::string_view(text) | std::views::split(" "sv);

        for (auto&& rng : splitted_ranges)
        {
            tokens.emplace_back(&*rng.begin(), std::ranges::distance(rng));
            // tokens.emplace_back(rng); // since C++23
        }

        CHECK(tokens == std::vector{"abc"sv, "def"sv, "ghi"sv});
    }
}

TEST_CASE("Exercise - ranges")
{
    const std::vector<std::string_view> lines = {
        "# Comment 1",
        "# Comment 2",
        "# Comment 3",
        "1/one",
        "2/two",
        "\n",
        "3/three",
        "4/four",
        "5/five",
        "\n",
        "\n",
        "6/six"
    };

    helpers::print(lines, "lines");

    auto result = std::views::all(lines)
                    | std::views::drop_while([](const std::string_view& sv) { return sv.starts_with("#"); })
                    //| std::views::filter([](const std::string_view& sv){ return sv.rfind("\n", 0) != 0; })
                    //| std::views::filter([](auto& item) { return !item.contains('\n'); })
                    | std::views::filter([](const auto& sv) { return sv != "\n"; })
                    | std::views::transform([](const std::string_view& sv){ return split(sv); })
                    | std::views::elements<1>;

    helpers::print(result, "result");

    auto expected_result = {"one"s, "two"s, "three"s, "four"s, "five"s, "six"s};

    CHECK(std::ranges::equal(result, expected_result));
}