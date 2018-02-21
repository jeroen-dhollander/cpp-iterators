#pragma once

#ifndef _ITERATOR_UTILITIES_H_
#define _ITERATOR_UTILITIES_H_

#include <iterator>
#include <type_traits>
#include <utility>

namespace iterator {
template <class>
class ReverseIterator;
template <class>
class Iterator;
template <class, class>
class JoinedIterator;

// Allows you to return an iterator over a collection, without actually having to expose access to the collection
// itself.
// Simply write this:
//
// class MyClass
// {
//     public:
//          const_iterator = IteratorConst<MyCollection>;
//          iterator = Iterator<MyCollection>;
//
//          const_iterator IterateStuff() const
//          {
//              return Iterate(stuff);
//          }
//
//          iterator IterateStuff()
//          {
//              return Iterate(stuff);
//          }
//
//     private:
//         MyCollection stuff_;
// };
template <typename T>
auto Iterate(T&& iterable) -> Iterator<T>;

// Allows you to iterate back-to-front over a collection
//
// Simply write this:
//
// for (auto & element : Reverse(my_collection))
// {
//      // do-something
// }
template <typename T>
auto Reverse(T&& iterable) -> ReverseIterator<T>;

//
// Allows you to iterate over 2 collections.
// First we'll walk the elements of the first collection,
// then the ones of the second.
//
// Simply write this:
//
// for (auto & element : Joined(collection_1, collection_2))
// {
//      // do-something
// }
//
template <typename T1, typename T2>
auto Join(T1&& iterable_1, T2&& iterable_2) -> JoinedIterator<T1, T2>;

//-----------------------------------------------------------------------------
// Implementation
//-----------------------------------------------------------------------------

template <typename T>
struct Iterator {
 public:
  using _Iterable = typename std::remove_reference_t<T>;
  using value_type = typename _Iterable::value_type;
  using const_iterator = typename _Iterable::const_iterator;
  using iterator = typename _Iterable::iterator;

  explicit Iterator(T&& iterable) : iterable_(std::forward<T>(iterable)) {
  }

  auto begin() {
    return std::begin(iterable_);
  }

  auto end() {
    return std::end(iterable_);
  }

  auto begin() const {
    return std::begin(iterable_);
  }

  auto end() const {
    return std::end(iterable_);
  }

 private:
  T iterable_;
};

template <typename T>
auto Iterate(T&& iterable) -> Iterator<T> {
  return Iterator<T>{std::forward<T>(iterable)};
}

template <typename T>
struct ReverseIterator {
 public:
  using _Iterable = typename std::remove_reference_t<T>;
  using value_type = typename _Iterable::value_type;
  using const_iterator = typename _Iterable::const_reverse_iterator;
  using iterator = typename _Iterable::reverse_iterator;

  explicit ReverseIterator(T&& iterable) : iterable_(std::forward<T>(iterable)) {
  }

  auto begin() {
    return std::rbegin(iterable_);
  }

  auto end() {
    return std::rend(iterable_);
  }

  auto begin() const {
    return std::rbegin(iterable_);
  }

  auto end() const {
    return std::rend(iterable_);
  }

 private:
  T iterable_;
};

template <typename T>
auto Reverse(T&& iterable) -> ReverseIterator<T> {
  return ReverseIterator<T>{std::forward<T>(iterable)};
}

template <class T1, class T2>
class JoinedIterator {
 public:
  template <class, class>
  class _Iterator;
  using _iterable_1 = typename std::remove_reference_t<T1>;
  using _const_iterator_1 = typename _iterable_1::const_iterator;
  using _iterator_1 = typename _iterable_1::iterator;

  using _iterable_2 = typename std::remove_reference_t<T2>;
  using _const_iterator_2 = typename _iterable_2::const_iterator;
  using _iterator_2 = typename _iterable_2::iterator;

  using iterator = _Iterator<typename _iterable_1::iterator, typename _iterable_2::iterator>;
  using const_iterator = _Iterator<typename _iterable_1::const_iterator, typename _iterable_2::const_iterator>;
  using value_type = typename _iterable_1::value_type;

  static_assert((std::is_same<typename _iterable_1::value_type, typename _iterable_2::value_type>::value),
                "value_type must be same type for both collections");

  JoinedIterator(T1&& data_1, T2&& data_2) : first_(std::forward<T1>(data_1)), second_(std::forward<T2>(data_2)) {
  }

  auto begin() {
    return MakeIterator(std::begin(first_), std::begin(second_));
  }

  auto end() {
    return MakeIterator(std::end(first_), std::end(second_));
  }

  auto begin() const {
    return MakeIterator(std::begin(first_), std::begin(second_));
  }

  auto end() const {
    return MakeIterator(std::end(first_), std::end(second_));
  }

  template <class _FirstIterator, class _SecondIterator>
  class _Iterator {
   public:
    _Iterator(_FirstIterator first, _FirstIterator first_end, _SecondIterator second, _SecondIterator second_end)
        : first_(first), first_end_(first_end), second_(second), second_end_(second_end) {
    }
    auto& operator*() {
      if (first_ != first_end_)
        return *first_;
      return *second_;
    }

    auto& operator*() const {
      if (first_ != first_end_)
        return *first_;
      return *second_;
    }

    void operator++() {
      if (first_ != first_end_)
        ++first_;
      else
        ++second_;
    }

    bool operator==(const _Iterator& other) const {
      return first_ == other.first_ && second_ == other.second_;
    }
    bool operator!=(const _Iterator& other) const {
      return !(*this == other);
    }

   private:
    _FirstIterator first_;
    _FirstIterator first_end_;
    _SecondIterator second_;
    _SecondIterator second_end_;
  };

 private:
  const_iterator MakeIterator(_const_iterator_1 first_begin, _const_iterator_2 second_begin) const {
    return const_iterator(first_begin, std::cend(first_), second_begin, std::cend(second_));
  }

  iterator MakeIterator(_iterator_1 first_begin, _iterator_2 second_begin) {
    return iterator(first_begin, std::end(first_), second_begin, std::end(second_));
  }

  T1 first_;
  T2 second_;
};

template <typename T1, typename T2>
auto Join(T1&& iterable_1, T2&& iterable_2) -> JoinedIterator<T1, T2> {
  return JoinedIterator<T1, T2>{std::forward<T1>(iterable_1), std::forward<T2>(iterable_2)};
}

}  // namespace iterator

#endif  // _ITERATOR_UTILITIES_H_
