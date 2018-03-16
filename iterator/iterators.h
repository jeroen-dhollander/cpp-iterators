#pragma once

#ifndef _ITERATOR_UTILITIES_H_
#define _ITERATOR_UTILITIES_H_

#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

namespace iterators {
template <class>
class Chained;
template <class>
class Enumerated;
template <class>
class Iterated;
template <class>
class ReferencedUnique;
template <class>
class Referenced;
template <class>
class Reversed;
template <class, class>
class Joined;
template <class, typename>
class Mapped;
template <class, typename>
class Filtered;

namespace details {
// clang-format off
template <class T> struct is_unique_pointer_helper : std::false_type {};
template <class T> struct is_unique_pointer_helper<std::unique_ptr<T>> : std::true_type {};
template <class T> struct is_unique_pointer : is_unique_pointer_helper<typename std::remove_cv_t<T>> {};
// clang-format on

template <class T>
struct remove_cvref {
  typedef std::remove_cv_t<std::remove_reference_t<T>> type;
};
template <class T>
using remove_cvref_t = typename remove_cvref<T>::type;

// Returns true if T is a collection over unique_ptr, e.g. vector<std::unique_ptr<int>>
template <class T>
struct is_unique_pointer_collection : is_unique_pointer<typename remove_cvref_t<T>::value_type> {};

// clang-format off
// Note: 'std::is_const' is pretty strict, e.g. 'std::is_const<const int&>' returns 'false'.
//       So we're using this construct that checks if it is const in any way
template<class T> struct is_const_type : std::false_type {};
template<class T> struct is_const_type<const T> : std::true_type {};
template<class T> struct is_const_type<const T&> : std::true_type {};
template<class T> struct is_const_type<const T*> : std::true_type {};
template<class T> struct is_const_type<T*const> : std::true_type {};
template<class T> struct is_const_type<const T*&> : std::true_type {};
template<class T> struct is_const_type<const T&&> : std::true_type {};
template<class T> struct is_const_type<const T*&&> : std::true_type {};
// clang-format on

template <class T, bool>
struct non_const_iterator_helper {
  typedef typename remove_cvref_t<T>::iterator type;
};
template <class T>
struct non_const_iterator_helper<T, true> {
  typedef typename remove_cvref_t<T>::const_iterator type;
};

// Returns T::iterator if the iterator is non_const,
// returns T::const_iterator if it is const.
// e.g. non_const_iterator_t<vector<int>> --> vector<int>::iterator
// e.g. non_const_iterator_t<const vector<int>> --> vector<int>::const_iterator
template <class T>
using non_const_iterator_t = typename non_const_iterator_helper<T, is_const_type<T>::value>::type;

}  // namespace details

// Allows you to enumerate over the elements of a given collection.
//
// This means that iterating over the returned object will return
// iterator objects with 2 fields:
//    - the position of the element in the collection
//    - a reference to the value of the element
//
// For example, this:
//
// vector<char> values {{'A', 'B', 'C'}};
// for (const auto & item: Enumerate(values))
//     printf("%u: %c\n", item.position, item.value);
//
// will print
//     0: A
//     1: B
//     2: C
template <typename T>
auto Enumerate(T&& iterable) -> Enumerated<T>;

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
auto Iterate(T&& iterable) -> Iterated<T>;

// Allows you to chain a collection of collection,
// e.g. to collect all the elements of a vector of lists
// Simply write this:
//
// vector<list<int>> collection
// for (const int & value : Chain(collection))
// {
//      // do-something
// }
template <typename T>
auto Chain(T&& iterable) -> Chained<T>;

// Allows you to iterate over a collection of pointers using references
//
// This is a nice way to hide how you internally store your objects.
//
// Simply write this:
//
// vector<MyObject*> collection_of_pointers
// for (const MyObject & object : AsReferences(collection_of_pointers))
// {
//      // do-something
// }
template <typename T, typename std::enable_if_t<!details::is_unique_pointer_collection<T>::value, int> = 1>
auto AsReferences(T&& iterable) -> Referenced<T>;

// Allows you to iterate over a collection of unique_ptr's using references
//
// This is a nice way to hide how you internally store your objects.
//
// Simply write this:
//
// vector<std::unique_ptr<MyObject>> collection_of_unique_pointers
// for (const MyObject & object : AsReferences(collection_of_unique_pointers))
// {
//      // do-something
// }
template <typename T, typename std::enable_if_t<details::is_unique_pointer_collection<T>::value, int> = 1>
auto AsReferences(T&& iterable) -> ReferencedUnique<T>;

// Allows you to iterate back-to-front over a collection
//
// Simply write this:
//
// for (auto & element : Reverse(my_collection))
// {
//      // do-something
// }
template <typename T>
auto Reverse(T&& iterable) -> Reversed<T>;

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
auto Join(T1&& iterable_1, T2&& iterable_2) -> Joined<T1, T2>;

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
auto Map(_Iterable&& data, Function mapping_function) -> Mapped<_Iterable, Function>;

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

// Returns an iterator over the elements for which filter(element) returns 'true'
template <typename _Iterable, typename FilterFunction>
auto Filter(_Iterable&& data, FilterFunction filter) -> Filtered<_Iterable, FilterFunction>;

//-----------------------------------------------------------------------------
// Implementation
//-----------------------------------------------------------------------------

template <typename T>
class Enumerated {
 public:
  struct Item;
  using _collection_type = typename std::remove_reference<T>::type;
  using _collection_value_type = typename _collection_type::value_type;
  using _collection_const_iterator = typename _collection_type::const_iterator;
  using _collection_iterator = typename _collection_type::iterator;

  class iterator;
  class const_iterator;
  using value_type = Item;

  // Return value when iterating a non-const version
  struct Item {
    int position;
    _collection_value_type& value;
  };
  // Return value when iterating a const version
  struct ConstItem {
    int position;
    const _collection_value_type& value;
  };
  // Used so we can update the stored item
  struct PointerItem {
    int position;
    const _collection_value_type* value;
  };
  // Internal representation of an item
  union UnionItem {
    UnionItem() : pointer_item() {
    }

    ConstItem const_item;
    Item item;
    PointerItem pointer_item;
  };

  explicit Enumerated(T&& iterable) : iterable_(std::forward<T>(iterable)) {
  }

  auto begin() const {
    return MakeIterator(std::begin(iterable_));
  }

  auto end() const {
    return MakeIterator(std::end(iterable_));
  }

  auto begin() {
    return MakeIterator(std::begin(iterable_));
  }

  auto end() {
    return MakeIterator(std::end(iterable_));
  }

  class iterator {
   public:
    iterator(_collection_iterator begin, _collection_iterator end, int position)
        : begin_(begin), end_(end), position_(position), item_{} {
      SetItem();
    }

    Item& operator*() {
      return item_.item;
    }

    const Item& operator*() const {
      return item_.item;
    }

    void operator++() {
      ++begin_;
      ++position_;
      SetItem();
    }

    bool operator==(const iterator& other) const {
      return begin_ == other.begin_;
    }
    bool operator!=(const iterator& other) const {
      return !(*this == other);
    }

   private:
    void SetItem() {
      if (begin_ != end_)
        item_.pointer_item = PointerItem{position_, &*begin_};
    }

    _collection_iterator begin_;
    _collection_iterator end_;
    int position_;
    UnionItem item_;
  };

  class const_iterator {
   public:
    const_iterator(_collection_const_iterator begin, _collection_const_iterator end, int position)
        : begin_(begin), end_(end), position_(position), item_{} {
      SetItem();
    }

    ConstItem& operator*() {
      return item_.const_item;
    }

    const ConstItem& operator*() const {
      return item_.item;
    }

    void operator++() {
      ++begin_;
      ++position_;
      SetItem();
    }

    bool operator==(const const_iterator& other) const {
      return begin_ == other.begin_;
    }
    bool operator!=(const const_iterator& other) const {
      return !(*this == other);
    }

   private:
    void SetItem() {
      if (begin_ != end_)
        item_.pointer_item = PointerItem{position_, &*begin_};
    }

    _collection_const_iterator begin_;
    _collection_const_iterator end_;
    int position_;
    UnionItem item_;
  };

 private:
  const_iterator MakeIterator(_collection_const_iterator begin) const {
    return const_iterator(begin, std::end(iterable_), 0);
  }

  const_iterator MakeIterator(_collection_const_iterator begin) {
    return const_iterator(begin, std::end(iterable_), 0);
  }

  iterator MakeIterator(_collection_iterator begin) {
    return iterator(begin, std::end(iterable_), 0);
  }

  T iterable_;
};

template <typename T>
auto Enumerate(T&& iterable) -> Enumerated<T> {
  return Enumerated<T>{std::forward<T>(iterable)};
}

template <typename T>
class Iterated {
 public:
  using _iterable = typename std::remove_reference_t<T>;

  using value_type = typename _iterable::value_type;
  using const_iterator = typename _iterable::const_iterator;
  using iterator = details::non_const_iterator_t<T>;

  explicit Iterated(T&& iterable) : iterable_(std::forward<T>(iterable)) {
  }

  iterator begin() {
    return std::begin(iterable_);
  }

  iterator end() {
    return std::end(iterable_);
  }

  const_iterator begin() const {
    return std::cbegin(iterable_);
  }

  const_iterator end() const {
    return std::cend(iterable_);
  }

 private:
  T iterable_;
};

template <typename T>
auto Iterate(T&& iterable) -> Iterated<T> {
  return Iterated<T>{std::forward<T>(iterable)};
}

template <class T>
class Chained {
 public:
  template <class, class>
  class _Iterator;
  using _outer_collection = details::remove_cvref_t<T>;
  using _outer_const_iterator = typename _outer_collection::const_iterator;
  using _outer_iterator = typename _outer_collection::iterator;
  using _inner_collection = details::remove_cvref_t<typename _outer_collection::value_type>;
  using _inner_const_iterator = typename _inner_collection::const_iterator;
  using _inner_iterator = typename _inner_collection::iterator;

  using iterator = _Iterator<_outer_iterator, _inner_iterator>;
  using const_iterator = _Iterator<_outer_const_iterator, _inner_const_iterator>;
  using value_type = typename _inner_collection::value_type;

  Chained(T&& data) : data_(std::forward<T>(data)) {
  }

  auto begin() {
    return MakeIterator(std::begin(data_));
  }

  auto end() {
    return MakeIterator(std::end(data_));
  }

  auto begin() const {
    return MakeIterator(std::begin(data_));
  }

  auto end() const {
    return MakeIterator(std::end(data_));
  }

  template <class __outer_iterator, class __inner_iterator>
  class _Iterator {
   public:
    _Iterator(__outer_iterator begin, __outer_iterator end)
        : outer_begin_(begin), outer_end_(end), inner_begin_(), inner_end_() {
      InitializeInnerCollection();
      SkipEmptyInnerCollections();
    }
    auto& operator*() {
      return *inner_begin_;
    }

    auto& operator*() const {
      return *inner_begin_;
    }

    void operator++() {
      ++inner_begin_;
      SkipEmptyInnerCollections();
    }

    bool operator==(const _Iterator& other) const {
      return IsAtEnd() == other.IsAtEnd();
    }

    bool operator!=(const _Iterator& other) const {
      return !(*this == other);
    }

   private:
    bool IsAtEnd() const {
      return outer_begin_ == outer_end_;
    }

    bool IsAtEndOfInnerCollection() const {
      return inner_begin_ == inner_end_;
    }

    // Initialized 'begin' and 'end' for the inner collection
    void InitializeInnerCollection() {
      if (!IsAtEnd()) {
        inner_begin_ = std::begin(*outer_begin_);
        inner_end_ = std::end(*outer_begin_);
      }
    }

    // If we're at the end of the inner collection, advance to the next (non-empty) one
    void SkipEmptyInnerCollections() {
      while (!IsAtEnd() && IsAtEndOfInnerCollection()) {
        AdvanceOuterCollection();
      }
    }

    // Advance to the next inner collection
    void AdvanceOuterCollection() {
      ++outer_begin_;
      InitializeInnerCollection();
    }

    __outer_iterator outer_begin_;
    __outer_iterator outer_end_;
    __inner_iterator inner_begin_;
    __inner_iterator inner_end_;
  };

 private:
  const_iterator MakeIterator(_outer_const_iterator begin_iterator) const {
    return const_iterator(begin_iterator, std::cend(data_));
  }

  iterator MakeIterator(_outer_iterator begin_iterator) {
    return iterator(begin_iterator, std::end(data_));
  }

  T data_;
};

template <typename T>
auto Chain(T&& iterable) -> Chained<T> {
  return Chained<T>{std::forward<T>(iterable)};
}

template <typename T>
class Referenced {
 public:
  template <class>
  class _Iterator;

  using _collection_type = typename std::remove_reference_t<T>;
  using _collection_value_type = std::remove_pointer_t<typename _collection_type::value_type>;
  using _collection_const_iterator = typename _collection_type::const_iterator;
  using _collection_iterator = typename _collection_type::iterator;

  using value_type = _collection_value_type;
  using const_iterator = _Iterator<_collection_const_iterator>;
  using iterator = _Iterator<_collection_iterator>;

  explicit Referenced(T&& iterable) : iterable_(std::forward<T>(iterable)) {
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

  template <class __iterator>
  class _Iterator {
   public:
    _Iterator(__iterator begin, __iterator end) : begin_(begin), end_(end) {
    }

    value_type& operator*() {
      return **begin_;
    }

    const value_type& operator*() const {
      return **begin_;
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
    __iterator begin_;
    __iterator end_;
  };

 private:
  const_iterator MakeIterator(_collection_const_iterator begin_iterator) const {
    return const_iterator(begin_iterator, std::cend(iterable_));
  }

  iterator MakeIterator(_collection_iterator begin_iterator) {
    return iterator(begin_iterator, std::end(iterable_));
  }

 private:
  T iterable_;
};

template <typename T, typename std::enable_if_t<!details::is_unique_pointer_collection<T>::value, int>>
auto AsReferences(T&& iterable) -> Referenced<T> {
  return Referenced<T>{std::forward<T>(iterable)};
}

template <typename T>
class ReferencedUnique {
 public:
  template <class>
  class _Iterator;

  using _collection_type = typename std::remove_reference_t<T>;
  using _collection_value_type = typename _collection_type::value_type;
  using _collection_const_iterator = typename _collection_type::const_iterator;
  using _collection_iterator = typename _collection_type::iterator;

  using value_type = typename _collection_value_type::element_type;
  using const_iterator = _Iterator<_collection_const_iterator>;
  using iterator = _Iterator<_collection_iterator>;

  explicit ReferencedUnique(T&& iterable) : iterable_(std::forward<T>(iterable)) {
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

  template <class __iterator>
  class _Iterator {
   public:
    _Iterator(__iterator begin, __iterator end) : begin_(begin), end_(end) {
    }

    value_type& operator*() {
      auto& unique_pointer = *begin_;
      auto pointer = unique_pointer.get();
      return *pointer;
    }

    const value_type& operator*() const {
      const auto& unique_pointer = *begin_;
      auto pointer = unique_pointer.get();
      return *pointer;
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
    __iterator begin_;
    __iterator end_;
  };

 private:
  const_iterator MakeIterator(_collection_const_iterator begin_iterator) const {
    return const_iterator(begin_iterator, std::cend(iterable_));
  }

  iterator MakeIterator(_collection_iterator begin_iterator) {
    return iterator(begin_iterator, std::end(iterable_));
  }

 private:
  T iterable_;
};

template <typename T, typename std::enable_if_t<details::is_unique_pointer_collection<T>::value, int>>
auto AsReferences(T&& iterable) -> ReferencedUnique<T> {
  return ReferencedUnique<T>{std::forward<T>(iterable)};
}

template <typename T>
class Reversed {
 public:
  using _iterable = typename std::remove_reference_t<T>;

  using value_type = typename _iterable::value_type;
  using const_iterator = typename _iterable::const_reverse_iterator;
  using iterator = typename _iterable::reverse_iterator;

  explicit Reversed(T&& iterable) : iterable_(std::forward<T>(iterable)) {
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
auto Reverse(T&& iterable) -> Reversed<T> {
  return Reversed<T>{std::forward<T>(iterable)};
}

template <class T1, class T2>
class Joined {
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

  Joined(T1&& data_1, T2&& data_2) : first_(std::forward<T1>(data_1)), second_(std::forward<T2>(data_2)) {
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
auto Join(T1&& iterable_1, T2&& iterable_2) -> Joined<T1, T2> {
  return Joined<T1, T2>{std::forward<T1>(iterable_1), std::forward<T2>(iterable_2)};
}

template <typename T, typename Function>
class Mapped {
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

  Mapped(T&& iterable, Function&& mapping_function)
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

template <typename _Iterable, typename Function>
auto Map(_Iterable&& data, Function mapping_function) -> Mapped<_Iterable, Function> {
  return Mapped<_Iterable, Function>(std::forward<_Iterable>(data), std::forward<Function>(mapping_function));
}

template <typename T, typename FilterFunction>
class Filtered {
 public:
  template <class, typename>
  class _Iterator;
  using _iterable = typename std::remove_reference_t<T>;
  using _iterable_const_iterator = typename _iterable::const_iterator;
  using _iterable_iterator = typename _iterable::iterator;

  using value_type = typename _iterable::value_type;
  using iterator = _Iterator<_iterable_iterator, typename std::remove_reference_t<FilterFunction>>;
  using const_iterator = _Iterator<_iterable_const_iterator, typename std::remove_reference_t<FilterFunction>>;

  Filtered(T&& iterable, FilterFunction&& filter)
      : iterable_(std::forward<T>(iterable)), filter_(std::forward<FilterFunction>(filter)) {
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
    _Iterator(__iterable begin, __iterable end, const _function& filter) : begin_(begin), end_(end), filter_(filter) {
      SkipFilteredEntries();
    }

    auto& operator*() {
      return *begin_;
    }

    const auto& operator*() const {
      return *begin_;
    }

    void operator++() {
      ++begin_;
      SkipFilteredEntries();
    }

    bool operator==(const _Iterator& other) const {
      return begin_ == other.begin_;
    }
    bool operator!=(const _Iterator& other) const {
      return !(*this == other);
    }

   private:
    bool IsEnd() const {
      return begin_ == end_;
    }

    bool IsFiltered() const {
      return !filter_(*begin_);
    }

    void SkipFilteredEntries() {
      while (!IsEnd() && IsFiltered())
        ++begin_;
    }

    __iterable begin_;
    __iterable end_;
    const _function& filter_;
  };

 private:
  const_iterator MakeIterator(_iterable_const_iterator begin_iterator) const {
    return const_iterator(begin_iterator, std::cend(iterable_), filter_);
  }

  iterator MakeIterator(_iterable_iterator begin_iterator) {
    return iterator(begin_iterator, std::end(iterable_), filter_);
  }

  T iterable_;
  FilterFunction filter_;
};

template <typename _Iterable, typename FilterFunction>
auto Filter(_Iterable&& data, FilterFunction filter) -> Filtered<_Iterable, FilterFunction> {
  return Filtered<_Iterable, FilterFunction>(std::forward<_Iterable>(data), std::forward<FilterFunction>(filter));
}

}  // namespace iterators

#endif  // _ITERATOR_UTILITIES_H_
