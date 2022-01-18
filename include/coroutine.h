#ifndef UTILS_COROUTINE_H
#define UTILS_COROUTINE_H
#include <coroutine>
#include <exception>
#include <iterator>

template<typename T>
struct co_generator {
    static_assert(!std::is_void_v<T>, "co_generator: template parameter must not be void");
    struct promise_type;
    using co_handle = std::coroutine_handle<promise_type>;
private:
    co_handle handle;
public:
    struct promise_type {
        T current_value;
        promise_type() = default;

        co_generator get_return_object() { return {co_handle::from_promise(*this)}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() { std::terminate(); }
        template<std::convertible_to<T> U>
        std::suspend_always yield_value(U&& u) {
            current_value = std::forward<U>(u);
            return {};
        }
        void return_void() {}
    };

    struct sentinel {};
    const sentinel _sentinel{};

    struct iterator {
        co_generator* gen;
        iterator(co_generator* _gen) : gen(_gen) {}
        T operator*() {
            return gen->handle.promise().current_value;
        }

        T* operator->() {
            return &gen->handle.promise().current_value;
        }

        iterator& operator++() {
            gen->handle.resume();
            return *this;
        }

        bool operator==(const sentinel& s) { return !gen || gen->handle.done(); }
    };


    co_generator(co_handle h) : handle(h) {}
    co_generator(const co_generator&) = delete;
    co_generator(co_generator&& other)  noexcept : handle(other.handle) {other.handle = nullptr; }
    ~co_generator() { if (handle) handle.destroy(); }
    operator co_handle() const { return handle; }

    T operator()() {
        T t = std::move(handle.promise().current_value);
        handle.resume();
        return t;
    }

    iterator begin() { return {this}; }
    const sentinel& end() { return _sentinel; }
};

#endif // UTILS_COROUTINE_H
