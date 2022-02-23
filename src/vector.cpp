/// Series of tests for vector.

// When your implementation is done, all the tests should pass.

#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>

#include <catch2/catch_test_macros.hpp>

#include "vector.hpp"

/// Series of tests on values whose constructors and destructors are trivial
/// (ie. don't perform any action)
TEST_CASE("Basic functional tests: trivial values") {
  vector_t<int> vec;               // Some vector
  vector_t<int> const &cref = vec; // A constant reference to the vector

  // Default-init vector should be empty
  REQUIRE(cref.size() == 0);

  // Adding 4 values
  vec.emplace_back(8);
  vec.emplace_back(8);
  vec.emplace_back(8);
  vec.emplace_back(8);

  // Testing value access
  REQUIRE(cref[0] == 8);
  REQUIRE(cref[1] == 8);
  REQUIRE(cref[2] == 8);
  REQUIRE(cref[3] == 8);

  // Testing value modification & access through a constant reference
  vec[0] = 0;
  vec[1] = 1;
  vec[2] = 2;
  vec[3] = 3;

  REQUIRE(cref[0] == 0);
  REQUIRE(cref[1] == 1);
  REQUIRE(cref[2] == 2);
  REQUIRE(cref[3] == 3);

  // Testing size management

  REQUIRE(cref.size() == 4);
  vec.resize(3);
  REQUIRE(cref.size() == 3);

  // Testing iterators

  REQUIRE(*cref.begin() == 0);
  REQUIRE(*(cref.end() - 1) == 2);

  // Testing copy constructor

  {
    vector_t<int> vec_copy(vec);

    REQUIRE(vec_copy[0] == 0);
    REQUIRE(vec_copy[1] == 1);
    REQUIRE(vec_copy[2] == 2);
  }

  // Testing copy assign operator
  {
    vector_t<int> vec_copy;
    vec_copy = vec;

    REQUIRE(vec_copy[0] == 0);
    REQUIRE(vec_copy[1] == 1);
    REQUIRE(vec_copy[2] == 2);

    // Testing move assign operator

    vector_t<int> vec_moved;
    vec_moved = std::move(vec_copy);

    REQUIRE(vec_moved[0] == 0);
    REQUIRE(vec_moved[1] == 1);
    REQUIRE(vec_moved[2] == 2);
  }

  // Testing move constructor

  vector_t<int> vec_moved(std::move(vec));

  REQUIRE(vec_moved[0] == 0);
  REQUIRE(vec_moved[1] == 1);
  REQUIRE(vec_moved[2] == 2);
}

/// Lifetime observation code
namespace lt {

struct observer_t;

/// Default construction counter
static unsigned construction_default;

/// Copy construction counter
static unsigned construction_copy;

/// Move construction counter
static unsigned construction_move;

/// Copy assignment counter
static unsigned assign_copy;

/// Move assignment counter
static unsigned assign_move;

/// Destruction counter
static unsigned destruction;

/// Reinitializes counters
void zero() {
  construction_default = 0;
  construction_copy = 0;
  construction_move = 0;
  assign_copy = 0;
  assign_move = 0;
  destruction = 0;
}

/// The observer_t class counts the number of constructions, assignments, and
/// copies in static variables for lifetime management observation.
struct observer_t {
  ~observer_t() { destruction++; }
  observer_t() { construction_default++; }
  observer_t(observer_t &&) { construction_move++; }
  observer_t(observer_t const &) { construction_copy++; }
  observer_t &operator=(observer_t &&) { return assign_move++, *this; }
  observer_t &operator=(observer_t const &) { return assign_copy++, *this; }
};

} // namespace lt

/// Series of tests to validate
TEST_CASE("Resize, reserve, and value lifetime management") {
  lt::zero();

  vector_t<lt::observer_t> vec;
  vector_t<lt::observer_t> const &cref = vec;

  REQUIRE(lt::construction_default == 0);
  REQUIRE(lt::construction_copy == 0);
  REQUIRE(lt::construction_move == 0);
  REQUIRE(lt::assign_copy == 0);
  REQUIRE(lt::assign_move == 0);
  REQUIRE(lt::destruction == 0);
  lt::zero();

  // Increasing size from 0 to 4 should default construct 4 values

  vec.resize(4);

  REQUIRE(cref.size() == 4);

  REQUIRE(lt::construction_default == 4);
  REQUIRE(lt::construction_copy == 0);
  REQUIRE(lt::construction_move == 0);
  REQUIRE(lt::assign_copy == 0);
  REQUIRE(lt::assign_move == 0);
  REQUIRE(lt::destruction == 0);
  lt::zero();

  // Resizing to equal size should do nothing

  vec.resize(4);

  REQUIRE(cref.size() == 4);

  REQUIRE(lt::construction_default == 0);
  REQUIRE(lt::construction_copy == 0);
  REQUIRE(lt::construction_move == 0);
  REQUIRE(lt::assign_copy == 0);
  REQUIRE(lt::assign_move == 0);
  REQUIRE(lt::destruction == 0);
  lt::zero();

  // Resizing from 4 to 8 should default construct 4 values

  vec.resize(8);

  REQUIRE(cref.size() == 8);

  REQUIRE(lt::construction_default == 4);
  REQUIRE(lt::construction_copy == 0);
  REQUIRE(lt::construction_move == 4);
  REQUIRE(lt::assign_copy == 0);
  REQUIRE(lt::assign_move == 0);
  REQUIRE(lt::destruction == 4);
  lt::zero();

  // Resizing from 8 to 0 should destroy 8 values

  vec.resize(0);

  REQUIRE(cref.size() == 0);

  REQUIRE(lt::construction_default == 0);
  REQUIRE(lt::construction_copy == 0);
  REQUIRE(lt::construction_move == 0);
  REQUIRE(lt::assign_copy == 0);
  REQUIRE(lt::assign_move == 0);
  REQUIRE(lt::destruction == 8);
  lt::zero();

  // Reserving when empty should allocate memory
  // with no effect on object lifetime

  {
    vector_t<lt::observer_t> new_vec;
    new_vec.reserve(1024);

    REQUIRE(lt::construction_default == 0);
    REQUIRE(lt::construction_copy == 0);
    REQUIRE(lt::construction_move == 0);
    REQUIRE(lt::assign_copy == 0);
    REQUIRE(lt::assign_move == 0);
    REQUIRE(lt::destruction == 0);
    lt::zero();
  }

  // The destruction of an empty vector with capacity for 1024 values
  // should not destroy any object

  REQUIRE(lt::construction_default == 0);
  REQUIRE(lt::construction_copy == 0);
  REQUIRE(lt::construction_move == 0);
  REQUIRE(lt::assign_copy == 0);
  REQUIRE(lt::assign_move == 0);
  REQUIRE(lt::destruction == 0);
  lt::zero();

  {
    // Creating a vector with 32 values should default construct 32 values

    vector_t<lt::observer_t> new_vec(32);

    REQUIRE(lt::construction_default == 32);
    REQUIRE(lt::construction_copy == 0);
    REQUIRE(lt::construction_move == 0);
    REQUIRE(lt::assign_copy == 0);
    REQUIRE(lt::assign_move == 0);
    REQUIRE(lt::destruction == 0);
    lt::zero();

    // Reserving more space should move construct 32 values
    // during the transfer of objects from a one buffer to the other

    new_vec.reserve(1024);

    REQUIRE(lt::construction_default == 0);
    REQUIRE(lt::construction_copy == 0);
    REQUIRE(lt::construction_move == 32);
    REQUIRE(lt::assign_copy == 0);
    REQUIRE(lt::assign_move == 0);
    REQUIRE(lt::destruction == 32);
    lt::zero();
  }

  // The destruction of new_vec should destroy 32 objects

  REQUIRE(lt::construction_default == 0);
  REQUIRE(lt::construction_copy == 0);
  REQUIRE(lt::construction_move == 0);
  REQUIRE(lt::assign_copy == 0);
  REQUIRE(lt::assign_move == 0);
  REQUIRE(lt::destruction == 32);
  lt::zero();
}

TEST_CASE("Lifetime management and element access") {
  {
    vector_t<lt::observer_t> a;
    a.reserve(1024);

    lt::zero();

    a.emplace_back();
    a.emplace_back();
    a.emplace_back();
    a.emplace_back();

    REQUIRE(lt::construction_default == 4);
    REQUIRE(lt::construction_copy == 0);
    REQUIRE(lt::construction_move == 0);
    REQUIRE(lt::assign_copy == 0);
    REQUIRE(lt::assign_move == 0);
    REQUIRE(lt::destruction == 0);
    lt::zero();
  }

  REQUIRE(lt::construction_default == 0);
  REQUIRE(lt::construction_copy == 0);
  REQUIRE(lt::construction_move == 0);
  REQUIRE(lt::assign_copy == 0);
  REQUIRE(lt::assign_move == 0);
  REQUIRE(lt::destruction == 4);
  lt::zero();
}

TEST_CASE("Lifetime management and copy/move semantics") {
  vector_t<lt::observer_t> a(32);
  lt::zero();

  {
    // Copy construct from a vector holding 32 values
    // should copy-construct 32 values

    vector_t<lt::observer_t> b(a);

    REQUIRE(lt::construction_default == 0);
    REQUIRE(lt::construction_copy == 32);
    REQUIRE(lt::construction_move == 0);
    REQUIRE(lt::assign_copy == 0);
    REQUIRE(lt::assign_move == 0);
    REQUIRE(lt::destruction == 0);
  }

  {
    // Copy assign from a vector holding 32 values
    // should copy-construct (not copy-assign) 32 values

    vector_t<lt::observer_t> b;
    lt::zero();

    b = a;

    REQUIRE(lt::construction_default == 0);
    REQUIRE(lt::construction_copy == 32);
    REQUIRE(lt::construction_move == 0);
    REQUIRE(lt::assign_copy == 0);
    REQUIRE(lt::assign_move == 0);
    REQUIRE(lt::destruction == 0);
    lt::zero();
  }

  {
    vector_t<lt::observer_t> b(32);
    lt::zero();

    // Move construct from a vector holding 32 values should do nothing

    vector_t<lt::observer_t> b_moved(std::move(b));

    REQUIRE(lt::construction_default == 0);
    REQUIRE(lt::construction_copy == 0);
    REQUIRE(lt::construction_move == 0);
    REQUIRE(lt::assign_copy == 0);
    REQUIRE(lt::assign_move == 0);
    REQUIRE(lt::destruction == 0);
    lt::zero();
  }

  // The destruction of both vectors should destroy 32 values

  REQUIRE(lt::construction_default == 0);
  REQUIRE(lt::construction_copy == 0);
  REQUIRE(lt::construction_move == 0);
  REQUIRE(lt::assign_copy == 0);
  REQUIRE(lt::assign_move == 0);
  REQUIRE(lt::destruction == 32);
  lt::zero();

  {
    vector_t<lt::observer_t> b(32);
    lt::zero();

    // Move construct from a vector holding 32 values still should do nothing

    vector_t<lt::observer_t> b_moved;
    b_moved = std::move(b);

    REQUIRE(lt::construction_default == 0);
    REQUIRE(lt::construction_copy == 0);
    REQUIRE(lt::construction_move == 0);
    REQUIRE(lt::assign_copy == 0);
    REQUIRE(lt::assign_move == 0);
    REQUIRE(lt::destruction == 0);
    lt::zero();
  }

  // The destruction of both vectors should destroy 32 values

  REQUIRE(lt::construction_default == 0);
  REQUIRE(lt::construction_copy == 0);
  REQUIRE(lt::construction_move == 0);
  REQUIRE(lt::assign_copy == 0);
  REQUIRE(lt::assign_move == 0);
  REQUIRE(lt::destruction == 32);
  lt::zero();
}
