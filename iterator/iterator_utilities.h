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
template <class, typename>
class Mapper;

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

// Applies the mapping-function to all the items in the input list.
// the mapping-function must take a reference _Iterable::value_type as input.
//
// Simply write this:
//
// std::list<int> collection = ...;
// for (std::string string_value : Map(collection, [](const auto & int_value) { return std::to_string(int_value); }))
// {
//       // do-something
// }
//
template <typename _Iterable, typename Function>
auto Map(_Iterable&& data, Function mapping_function) -> Mapper<_Iterable, Function>;

// Iterates over the keys of a std::map
template <typename _Iterable>
auto MapKeys(_Iterable&& map) {
  return Map(std::forward<_Iterable>(map), [](const auto& map_pair) { return map_pair.first; });
}

// Iterates over the values of a std::map
template <typename _Iterable>
auto MapValues(_Iterable&& map) {
  return Map(std::forward<_Iterable>(map), [](const auto& map_pair) { return map_pair.second; });
}

//-----------------------------------------------------------------------------
// Implementation
//-----------------------------------------------------------------------------

template <typename T>
class Iterator {
 public:
  using _iterable = typename std::remove_reference_t<T>;

  using value_type = typename _iterable::value_type;
  using const_iterator = typename _iterable::const_iterator;
  using iterator = typename _iterable::iterator;

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
class ReverseIterator {
 public:
  using _iterable = typename std::remove_reference_t<T>;

  using value_type = typename _iterable::value_type;
  using const_iterator = typename _iterable::const_reverse_iterator;
  using iterator = typename _iterable::reverse_iterator;

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

template <typename T, typename Function>
class Mapper {
 public:
  template <class, typename>
  class _Iterator;
  using _iterable = typename std::remove_reference_t<T>;
  using _iterable_value_type = typename _iterable::value_type;
  using _iterable_const_iterator = typename _iterable::const_iterator;
  using _iterable_iterator = typename _iterable::iterator;

  using iterator = _Iterator<_iterable_iterator, typename std::remove_reference_t<Function>>;
  using const_iterator = _Iterator<_iterable_const_iterator, typename std::remove_reference_t<Function>>;
  using value_type = typename std::result_of<Function(_iterable_value_type&)>::type;

  Mapper(T&& iterable, Function&& mapping_function)
      : iterable_(std::forward<T>(iterable)), mapping_function_(std::forward<Function>(mapping_function)) {
  }

  auto begin() {
    return MakeIterator(std::begin(iterable_));
  }

  auto end() {
    return MakeIterator(std::end(iterable_));
  }

  auto begin() const {
    return MakeIterator(std::begin(iterable_));
  }

  auto end() const {
    return MakeIterator(std::end(iterable_));
  }

  template <typename __iterable, typename _function>
  class _Iterator {
   public:
    _Iterator(__iterable begin, __iterable end, const _function& mapping_function)
        : begin_(begin), end_(end), mapping_function_(mapping_function) {
    }

    auto operator*() {
      return mapping_function_(*begin_);
    }

    const auto& operator*() const {
      return mapping_function_(*begin_);
    }

    void operator++() {
      ++begin_;
    }

    bool operator==(const _Iterator& other) const {
      return begin_ == other.begin_;
    }
    bool operator!=(const _Iterator& other) const {
      return !(*this == other);
    }

   private:
    __iterable begin_;
    __iterable end_;
    const _function& mapping_function_;
  };

 private:
  const_iterator MakeIterator(_iterable_const_iterator begin_iterator) const {
    return const_iterator(begin_iterator, std::cend(iterable_), mapping_function_);
  }

  iterator MakeIterator(_iterable_iterator begin_iterator) {
    return iterator(begin_iterator, std::end(iterable_), mapping_function_);
  }

  T iterable_;
  Function mapping_function_;
};

// Applies the mapping-function to all the items in the input list.
// the mapping-function must take a reference _Iterable::value_type as input.
template <typename _Iterable, typename Function>
auto Map(_Iterable&& data, Function mapping_function) -> Mapper<_Iterable, Function> {
  return Mapper<_Iterable, Function>(std::forward<_Iterable>(data), std::forward<Function>(mapping_function));
}

}  // namespace iterator

#endif  // _ITERATOR_UTILITIES_H_
