#pragma once

#include <memory>
#include <type_traits>

#ifndef __cpp_lib_constexpr_dynamic_alloc
namespace std {
  template <typename T, typename... Args>
  inline T *construct_at(T *p, Args &&...args) {
    return ::new (const_cast<void *>(static_cast<const volatile void *>(p)))
      T(std::forward<Args>(args)...);
  }
}
#endif

#define DEFAULT_CAPACITY 16

template <typename T> struct vector_t {
private:
  T *_data;
  std::size_t _size;
  std::size_t _capacity;
  std::allocator<T> _allocator;
  
public:
  vector_t() noexcept : _data(nullptr), _size(0), _capacity(0) {}

  vector_t(std::size_t size) : vector_t() {
    resize(size);
  }

  vector_t(vector_t const & other) : vector_t() {
    if (_size < other.size()) {
      reserve(other.size());
      _size = _capacity;
    }

    for (std::size_t i = 0; i < other.size(); ++i) {
      std::construct_at(this->begin() + i, *(other._data + i));
    }
  }

  vector_t(vector_t && other) {
    _data = other._data;
    _capacity = other._capacity;
    _size = other._size;
    other._data = nullptr;
    other._size = 0;
    other._capacity = 0;
  }

  vector_t &operator=(vector_t const & other) {
    if (_size < other.size()) {
      reserve(other.size());
      _size = _capacity;
    }

    for (std::size_t i = 0; i < other.size(); ++i) {
      std::construct_at(this->begin() + i, *(other._data + i));
    }

    return *this;
  }

  vector_t &operator=(vector_t && other) {
    _data = other._data;
    _capacity = other._capacity;
    _size = other._size;
    other._data = nullptr;
    other._size = 0;
    other._capacity = 0;
    return *this;
  }

  T *begin() { return _data; }
  T *end() { return _data + _size; }

  T const *begin() const { return _data; }
  T const *end() const { return _data + _size; }

  std::size_t size() const { return _size; }

  T &operator[](std::size_t i) { return _data[i]; }
  T const &operator[](std::size_t i) const { return _data[i]; }

  template<typename... Args> void emplace_back(Args&&... args) {
    std::size_t length = sizeof...(Args);
    if (_capacity == 0) reserve(DEFAULT_CAPACITY);
    if (_size < _capacity) std::construct_at(this->end(), std::forward<Args>(args)...);
    if (_size == _capacity && _capacity > 0) {
      reserve(_capacity * 2);
      std::construct_at(this->end(), std::forward<Args>(args)...);
    }
    if (length == 0) _size++;
    else _size += length;
  }

   void reserve(std::size_t new_capacity) {
    T* buffer = _allocator.allocate(new_capacity);
    if (_size > 0) {

      for (std::size_t i = 0; i < _size; ++i) {
	std::construct_at(buffer + i, std::move(*(this->begin() + i))); 
      }
      
      destroy(this->begin(), this->end());
      _allocator.deallocate(_data, _size);
    }
    _capacity = new_capacity;
    _data = buffer;
  }

  void resize(std::size_t new_size) {
    if (_size == new_size) return;

    if (_size == 0) {
      _data = _allocator.allocate(new_size);      

      for (std::size_t i = 0; i < new_size; ++i) {
	std::construct_at(this->end());
      }
      
      _capacity = new_size;
      _size = _capacity;
    }

    if (_size > 0) {
      if (new_size > _size) {
	T* buffer = _allocator.allocate(new_size);

	for (std::size_t i = 0; i < _size; ++i) {
	  std::construct_at(buffer + i, std::move(*(this->begin() + i)));
	}
	
	destroy(this->begin(), this->end()); 
	_allocator.deallocate(_data, _size);
	
	for (std::size_t i = _size; i < new_size; ++i) {
	  std::construct_at(buffer + i);
	}
	
	_size = new_size;
	_capacity = _size;
	_data = buffer;
      }
      
      if (new_size < _size) {
	destroy(this->begin() + new_size, this->end());
	_capacity = new_size;
	_size = _capacity;
      }      
    }
  }

  template< class ForwardIt> constexpr void destroy(ForwardIt first, ForwardIt last) {
    for (; first !=last; ++first) {
      std::destroy_at(std::addressof(*first));
    }
  }

  ~vector_t() {
    if (_size > 0) {
      destroy(this->begin(), this->end());       
      _allocator.deallocate(_data, _capacity);
    }
  }
};
