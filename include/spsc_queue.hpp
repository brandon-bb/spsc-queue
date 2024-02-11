#include <atomic>
#include <concepts>
#include <memory>
#include <type_traits>

namespace concurrent 
{
template <typename T, typename... Args>
concept constructible_type = std::is_constructible<T, Args...>::value;

template <typename T, typename Allocator = std::allocator<T>>
class spsc_queue final
{
  private:
    T* slots_;
    Allocator allocator_;

    const std::size_t capacity_;
    std::atomic<std::size_t> reader_ { 0 };
    std::atomic<std::size_t> writer_ { 0 };
    //std::atomic<std::size_t> buffer_size;

  public:
    spsc_queue (std::size_t capacity, const Allocator& allocator = Allocator())
      : capacity_ (capacity), allocator_(allocator)
    {
      slots_ = allocator_.allocate (capacity);
    }

    //can implement move and copy assignment but delete for now
    CircularBuffer& operator=(const CircularBuffer&) = delete;
    CircularBuffer& operator=(CircularBuffer&&) = delete;


    //is constructible
    template <typename... Args>
    requires constructible_type<T, Args...>
    T& emplace (Args&&... args)
    {
      const auto& index = writer_.load(std::memory_order_relaxed);
      writer_.store ((index + 1) % capacity, std::memory_order_relaxed);

      return *new (slots_ + index)
        T(std::forward<Args>(args)...);
    }

    
    T& write (const T& item)
    {
      const auto& index = writer_.load (std::memory_order_relaxed);

      if (index == capacity) {}

      slots_[index] = item;
      writer_.store ((index + 1) % capacity, std::memory_order_relaxed);

      return slots_[index];
    }


    const T& write (const T&& item)
    {
      static_assert (std::is_trivially_assignable<decltype(item)>::value,
        "T must be assignable via operator=");

      const auto& index = writer_.load (std::memory_order_relaxed);
      
      slots_[index] = T(std::forward<T>(value));
    }


    T& read ()
    {
      const auto& index = reader_.load (std::memory_order_relaxed);
      reader_.store ((index + 1) % capacity, std::memory_order_relaxed);

      return slots_[index];
    }

};
}