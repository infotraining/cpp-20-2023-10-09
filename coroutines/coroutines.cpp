#include <catch2/catch_test_macros.hpp>
#include <coroutine>
#include <iostream>
#include <string>
#include <vector>

using namespace std::literals;

// https://github.com/facebookexperimental/libunifex

class TaskResumer
{
public:
    struct promise_type;

    using CoroutineHandle = std::coroutine_handle<promise_type>;

    struct promise_type
    {
        TaskResumer get_return_object()
        {
            return TaskResumer{CoroutineHandle::from_promise(*this)};
        }

        std::suspend_always initial_suspend() const noexcept { return {}; }

        std::suspend_always final_suspend() noexcept { return {}; }

        void return_void()
        { }

        void unhandled_exception()
        {
            std::terminate();
        }
    };

public:
    TaskResumer(CoroutineHandle coro_hndl)
        : coro_hndl_{coro_hndl}
    { }

    TaskResumer(const TaskResumer&) = delete;
    TaskResumer& operator=(const TaskResumer&) = delete;
    TaskResumer(TaskResumer&&) = delete;
    TaskResumer& operator=(TaskResumer&&) = delete;

    ~TaskResumer() noexcept
    {
        if (coro_hndl_)
            coro_hndl_.destroy();
    }

    bool resume() const
    {
         if (!coro_hndl_ || coro_hndl_.done())
            return false;

        coro_hndl_.resume(); // resuming suspended coroutine

        return !coro_hndl_.done();
    }

private:
    CoroutineHandle coro_hndl_;
};

TaskResumer foo(int max)
{
    std::string str = "HELLO";
    std::cout << "..coro(START, " << max << ")\n";

    for (int value = 1; value <= max; ++value)
    {
        std::cout << "..coro(" << value << ", " << max << ")\n";
        ///////////////////////////////////////////////////////
        co_await std::suspend_always{}; // SUSPENSION POINT
    }

    std::cout << "..coro(END, " << max << ")\n";
}

TEST_CASE("first coroutine")
{
    TaskResumer task = foo(5);

    while(task.resume())
    {
        std::cout << "coro() suspended...\n";
    }
    
    std::cout << "coro() done!\n";
}

namespace FutureStd
{

    template <typename T>
    class [[nodiscard]] Generator
    {
    public:
        struct promise_type;

        using CoroutineHandle = std::coroutine_handle<promise_type>;

        struct promise_type
        {
            Generator get_return_object()
            {
                return Generator{CoroutineHandle::from_promise(*this)};
            }

            std::suspend_always initial_suspend() const { return {}; }

            std::suspend_always final_suspend() const noexcept { return {}; }

            void unhandled_exception() { std::terminate(); }

            std::suspend_always yield_value(auto&& yielded_value)
            {
                value = std::forward<decltype(yielded_value)>(yielded_value);
                return {};
            }

            void return_void() { }

            T value;
        };

        struct iterator
        {
            using value_type = T;
            using reference = T;
            using iterator_category = std::input_iterator_tag;

            CoroutineHandle coroutine_handle_ = nullptr;

            iterator() = default;

            iterator(auto coroutine_handle)
                : coroutine_handle_{coroutine_handle}
            { }

            T operator*() const
            {
                assert(coroutine_handle_ != nullptr);
                return coroutine_handle_.promise().value;
            }

            T* operator->() const
            {
                assert(coroutine_handle_ != nullptr);
                return &coroutine_handle_.promise().value;
            }

            iterator& operator++()
            {
                move_to_next();
                return *this;
            }

            iterator operator++(int)
            {
                iterator prev_pos = *this;
                move_to_next();
                return prev_pos;
            }

            bool operator==(const iterator& other) const = default;

        private:
            friend class Generator;

            void move_to_next()
            {
                if (coroutine_handle_ && !coroutine_handle_.done())
                {
                    coroutine_handle_.resume();

                    if (coroutine_handle_.done())
                    {
                        coroutine_handle_ = nullptr;
                    }
                }
            }
        };

        Generator(CoroutineHandle coroutine_hndl)
            : coroutine_hndl_{coroutine_hndl}
        { }

        Generator(const Generator&) = delete;
        Generator& operator=(const Generator&) = delete;

        ~Generator()
        {
            if (coroutine_hndl_)
                coroutine_hndl_.destroy();
        }

        std::optional<T> next_value()
        {
            assert(coroutine_hndl_);
            // if (!coroutine_hndl_ || coroutine_hndl_.done())
            //     return std::nullopt;

            coroutine_hndl_.resume();

            if (coroutine_hndl_.done())
                return std::nullopt;

            return coroutine_hndl_.promise().value;
        }

        iterator begin() const
        {
            if (!coroutine_hndl_ || coroutine_hndl_.done())
                return {};

            iterator it{coroutine_hndl_};
            it.next();
            return it;
        }

        iterator end() const
        {
            return {};
        }

        iterator begin()
        {
            if (!coroutine_hndl_ || coroutine_hndl_.done())
                return {};

            iterator it{coroutine_hndl_};
            it.move_to_next();
            return it;
        }

        iterator end()
        {
            return {};
        }

    private:
        CoroutineHandle coroutine_hndl_;
    };
} // namespace FutureStd

using FutureStd::Generator;

Generator<int> fibonacci(int n)
{
    auto a = 0, b = 1;

    while (a < n)
        co_yield std::exchange(a, std::exchange(b, a + b));
}

namespace views = std::ranges::views;

TEST_CASE("fibonacci with generator")
{
    for (const auto& item : fibonacci(100))
        std::cout << item << " ";
    std::cout << "\n";
}