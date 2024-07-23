#pragma once

#include <mutex>
#include <utility>

/**
 * Wrapper for synchronised access to a resource.
 *
 * @tparam Type
 */
template<typename Type>
class Synchronised
{
public:
    class Accessor
    {
    public:
        constexpr Accessor(std::mutex& mutex, Type& value)
            : lock(std::unique_lock(mutex))
            , value(value)
        {}

        constexpr Type* operator -> () noexcept {
            return &(this->value);
        }

        constexpr const Type* operator -> () const noexcept {
            return &(this->value);
        }

        constexpr Type& operator * () noexcept {
            return this->value;
        }

        constexpr const Type& operator * () const noexcept {
            return this->value;
        }

    private:
        std::unique_lock<std::mutex> lock;
        Type& value;
    };

    Synchronised() = default;

    explicit Synchronised(Type value)
        : value(std::move(value))
    {}

    Accessor accessor() {
        return Accessor(this->mutex, this->value);
    }

    /**
     * Don't use this unless you already hold a raw (not managed by an Accessor) lock to the contained value.
     *
     * This should only be used in instances where you need to hold a raw lock, like in the `stop_waiting`
     * predicate function for a call to std::condition_variable::wait().
     *
     * In all other instances, you should use Synchronised::accessor().
     *
     * @return
     */
    Type& unsafeReference() {
        return this->value;
    }

    std::unique_lock<std::mutex> lock() {
        return std::unique_lock{this->mutex};
    }

private:
    Type value;
    std::mutex mutex;
};
