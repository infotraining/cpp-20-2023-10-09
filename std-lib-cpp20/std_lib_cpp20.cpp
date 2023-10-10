#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <span>
#include <string>
#include <vector>
#include <format>
#include <numbers>

using namespace std::literals;

void print(std::span<const int> data)
{
    for (const auto& item : data)
        std::cout << item << " ";
    std::cout << "\n";
}

void zero(std::span<int> data, int zero_value = 0)
{
    for(auto& item : data)
        item = zero_value;
}

TEST_CASE("std::span")
{
    int native_array[10] = {1, 2, 3, 4, 5};
    std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    SECTION("fixed extent")
    {
        std::span<int, 10> span_view(native_array, 10);
        print(span_view);

        span_view = std::span<int, 10>(vec.begin(), 10);
        print(span_view);
    }

    SECTION("dynamic extent")
    {
        std::span<int> span_view(vec);
        print(span_view);
        print(vec);

        print(std::span{vec.begin(), 5});
        print(span_view.last(3));

        zero(vec);
        print(vec);
    }
}

void print_as_bytes(const float f, const std::span<const std::byte> bytes)
{
	std::cout << std::format("{:+6}", f) << " - { ";

	for(const std::byte b : bytes)
	{
		std::cout << std::format("{:02X} ", std::to_integer<int>(b));
	}

	std::cout << "}\n";
}

TEST_CASE("span of bytes")
{
    float data[] = { std::numbers::pi_v<float> };

    auto const_bytes = std::as_bytes(std::span{data});
    print_as_bytes(data[0], const_bytes);

    auto writable_bytes = std::as_writable_bytes(std::span{data});
    writable_bytes[3] |= std::byte{0b1000'0000};
    print_as_bytes(data[0], writable_bytes);
}

std::span<int> get_head(std::span<int> items, size_t head_size = 1)
{
    return items.first(head_size);
}

TEST_CASE("dangling pointers & span")
{
    std::vector vec{1, 2, 3};
    auto head = get_head(vec, 2);

    print(head);

    vec.push_back(4); // UB - iterators in span are invalidated
}