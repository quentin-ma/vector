#pragma once

#include <memory>
#include <type_traits>
#include <tuple>

#ifndef __cpp_lib_constexpr_dynamic_alloc
namespace std {
template <typename T, typename... Args>
inline T *construct_at(T *p, Args &&...args) {
  return ::new (const_cast<void *>(static_cast<const volatile void *>(p)))
      T(std::forward<Args>(args)...);
}
} // namespace std
#endif

template <typename T> struct vector_t {
private:
  /// Pointer to the memory buffer.
  /// It should be either valid or equal nullptr.
  T *_data;

  /// Size of the vector.
  /// Holds the number of alive values.
  std::size_t _size;

  /// Capacity of the memory buffer.
  std::size_t _capacity;

  /// Memory allocator.
  std::allocator<T> _allocator;
  
public:
  /// Default constructor that initializes an empty vector with no capacity
  vector_t() noexcept : _data(nullptr), _size(0), _capacity(0) {}

  /// The following constructor should initialize a vector of given size. The
  /// capacity should be the same as the size, and all the elements must be
  /// default constructed[1].

  vector_t(std::size_t size)
      // Calling the default constructor first to ensure the vector
      // is well initialized. Member functions such as resize or reserve
      // should never be used on uninitialized objects.
      : vector_t() {
    resize(size);
  }

  vector_t(vector_t const & other) : vector_t() {
    if (_data) delete _data;
    reserve(other.size());
    for (std::size_t i = 0; i < other.size(); ++i) {
      std::construct_at(_data + i, *(other._data + i));
    }
  }

  vector_t(vector_t && other) noexcept : _data(other._data) {
    other._data = nullptr;
  }

  vector_t &operator=(vector_t const & other) {
    reserve(other.size());
    for (std::size_t i = 0; i < other.size(); ++i) {
      std::construct_at(_data + i, *(other._data + i));
    }
    return *this;
  }

  vector_t &operator=(vector_t && other) {
    if (_data) delete _data;
    _data = other._data;
    other._data = nullptr;
    return *this;
  }

  /// Returns a pointer as an iterator to the beginning of the vector.
  T *begin() { return _data; }
  /// Returns a pointer as an iterator to the end of the vector.
  T *end() { return _data + _size; }

  /// Returns a constant pointer as an iterator to the beginning of the vector.
  T const *begin() const { return _data; }
  /// Returns a constant pointer as an iterator to the end of the vector.
  T const *end() const { return _data + _size; }

  /// Returns the size of the vector.
  std::size_t size() const { return _size; }

  /// Non-const element access for getting and modifying elements.
  T &operator[](std::size_t i) { return _data[i]; }
  /// Read-only element access.
  T const &operator[](std::size_t i) const { return _data[i]; }

  /// emplace_back constructs a new element at the end of the vector.
  template<typename... Args> void emplace_back(Args &&...args)
  {
    auto t = std::make_tuple(std::forward<Args>(args)...);
    constexpr auto args_size = sizeof...(args);

    if (_capacity == 0) {
      reserve(1 << 4);
      std::construct_at(_data + _size);
    }
    
    if (args_size == 0) {
      if (_size < _capacity) std::construct_at(_data + _size);
      if (_size == _capacity && _capacity > 0) {
	reserve(_capacity << 1);
	std::construct_at(_data + _size);
      }
    } else {
      [&]<std::size_t ... p>(std::index_sequence<p...>)
	{
	  if (_size < _capacity) ((std::construct_at(_data + _size, std::get<p>(t))), ...);
	  if (_size == _capacity && _capacity > 0) {
	    reserve(_capacity << 1);
	    ((std::construct_at(_data + _size, std::get<p>(t))), ...);
	  }     
	} (std::make_index_sequence<args_size>{});    
    }
    _size++;
  }

  /// Reserve changes the capacity of the vector. 
   void reserve(std::size_t new_capacity) {
    T* buffer = _allocator.allocate(new_capacity);
    if (_data && _size > 0) {

      for (std::size_t i = 0; i < _size; ++i) {
	std::construct_at(buffer + i, std::move(*(_data + i))); // move construct
      }

      destroy(_data, _data + _size);

      _allocator.deallocate(_data, _size);
    }
    _capacity = new_capacity;
    _data = buffer;
  }

  /// Resize should set the size of the vector, destroying or
  /// default constructing values as necessary[1].
  /// It should also reserve memory as needed.

  void resize(std::size_t new_size) {
    if (_size == new_size) return; // do nothing

    if (_size == 0) { // if vector contain no value
      _data = _allocator.allocate(new_size);      
      for (std::size_t i = 0; i < new_size; ++i) {
	std::construct_at(_data + i); // default constructing
      }
      _capacity = new_size;
      _size = _capacity;
    }

    if (_size > 0) { // if vector contain at least 1 value
      if (new_size > _size) {
	T* buffer = _allocator.allocate(new_size);

	for (std::size_t i = 0; i < _size; ++i) {
	  std::construct_at(buffer + i, std::move(*(_data + i))); // move construct
	}

	destroy(_data, _data + _size); // destruction

	_allocator.deallocate(_data, _size);

	for (std::size_t i = _size; i < new_size; ++i) {
	  std::construct_at(buffer + i); // default constructing
	}
	_size = new_size;
	_capacity = _size;
	_data = buffer; // copying
      }

      if (new_size < _size) { // 
	destroy(_data, _data + _size); // destruction
	_capacity = new_size;
	_size = new_size;
      }
      
    }
  }

  // implementation of destroy (eq. to std::destroy_n)
  template< class ForwardIt> constexpr void destroy(ForwardIt first, ForwardIt last) {
    for (; first !=last; ++first)
      std::destroy_at(std::addressof(*first));
  }

  /// The destructor should destroy[1] all the values that are alive and
  /// deallocate the memory buffer, if there is one.
  ~vector_t() {
    destroy(_data, _data + _size);
    if (_data) {
      _allocator.deallocate(_data, _capacity);
    }
  }
};
