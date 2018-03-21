#pragma once

#ifndef _CPP_ITERATORS_ITERATORS_H_
#define _CPP_ITERATORS_ITERATORS_H_

#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

namespace iterators {
template <typename> class Chained;
template <typename> class ForwardChained;
template <typename> class Enumerated;
template <typename> class ForwardEnumerated;
template <typename> class Iterated;
template <typename> class ForwardIterated;
template <typename> class ReferencedUnique;
template <typename> class ForwardReferencedUnique;
template <typename> class Referenced;
template <typename> class ForwardReferenced;
template <typename> class Reversed;
template <typename> class ForwardReversed;
template <typename, typename> class Joined;
template <typename, typename> class ForwardJoined;
template <typename, typename> class Mapped;
template <typename, typename> class ForwardMapped;
template <typename, typename> class Filtered;
template <typename, typename> class ForwardFiltered;

namespace details {

// These are defined in C++14/C++17 but not in C++11
template <bool B, class T, class F> using conditional_t = typename std::conditional<B, T, F>::type;
template <typename...> struct conjunction : std::false_type {};
template <typename B1> struct conjunction<B1> : B1 {};
template <typename B1, class... Bn>
struct conjunction<B1, Bn...> : conditional_t<bool(B1::value), conjunction<Bn...>, std::false_type> {};
template <bool B, typename T = void> using enable_if_t = typename std::enable_if<B, T>::type;
template <typename T> using remove_cv_t = typename std::remove_cv<T>::type;
template <typename T> using remove_reference_t = typename std::remove_reference<T>::type;
template <typename T> using remove_pointer_t = typename std::remove_pointer<T>::type;

template <typename T> struct remove_cvref { typedef remove_cv_t<remove_reference_t<T>> type; };
template <typename T> using remove_cvref_t = typename remove_cvref<T>::type;

template <typename T> struct is_unique_pointer_helper : std::false_type {};
template <typename T> struct is_unique_pointer_helper<std::unique_ptr<T>> : std::true_type {};
template <typename T> struct is_unique_pointer : is_unique_pointer_helper<remove_cv_t<T>> {};

// Returns true if T is a collection over unique_ptr, e.g. vector<std::unique_ptr<int>>
template <typename T>
struct is_unique_pointer_collection : is_unique_pointer<typename remove_cvref_t<T>::value_type> {};

// From https://stackoverflow.com/a/257382/3490116 , adapted to use decltype/declval
template <typename T> class has_rbegin {
  typedef char one;
  typedef long two;

  template <typename C> static one test(decltype(std::declval<remove_cvref_t<C>>().rbegin())*);
  template <typename C> static two test(...);

 public:
  enum { value = sizeof(test<T>(nullptr)) == sizeof(one) };
};

// True if 'T' is a collection that supports forward and reverse iterating
template <typename T> using is_bidirectional_collection = has_rbegin<T>;

template <typename T1, typename T2>
using are_bidirectional_collections = conjunction<is_bidirectional_collection<T1>, is_bidirectional_collection<T2>>;

// True if both the outer and the nested collection are a bidirectional collection
template <typename T>
using is_nested_bidirectional_collection = are_bidirectional_collections<T, typename remove_cvref_t<T>::value_type>;

}  // namespace details

// Allows you to enumerate over the elements of a given collection.
//
// This means that iterating over the returned object will return
// iterator objects with 2 methods:
//    - Postion(): the position of the element in the collection
//    - Value(): a reference to the value of the element
//
// For example, this:
//
// vector<char> values {{'A', 'B', 'C'}};
// for (const auto & item: Enumerate(values))
//     printf("%u: %c\n", item.Position(), item.Value());
//
// will print
//     0: A
//     1: B
//     2: C
template <typename T, typename details::enable_if_t<details::is_bidirectional_collection<T>::value, int> = 1>
auto Enumerate(T&& iterable) {
  return Enumerated<T>{std::forward<T>(iterable)};
}
template <typename T, typename details::enable_if_t<!details::is_bidirectional_collection<T>::value, int> = 1>
auto Enumerate(T&& iterable) {
  return ForwardEnumerated<T>{std::forward<T>(iterable)};
}

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
template <typename T, typename details::enable_if_t<details::is_bidirectional_collection<T>::value, int> = 1>
auto Iterate(T&& iterable) {
  return Iterated<T>{std::forward<T>(iterable)};
}
template <typename T, typename details::enable_if_t<!details::is_bidirectional_collection<T>::value, int> = 1>
auto Iterate(T&& iterable) {
  return ForwardIterated<T>{std::forward<T>(iterable)};
}

// Allows you to chain a collection of collection,
// e.g. to collect all the elements of a vector of lists
// Simply write this:
//
// vector<list<int>> collection
// for (const int & value : Chain(collection))
// {
//      // do-something
// }
template <typename T, typename details::enable_if_t<details::is_nested_bidirectional_collection<T>::value, int> = 1>
auto Chain(T&& iterable) {
  return Chained<T>{std::forward<T>(iterable)};
}

template <typename T, typename details::enable_if_t<!details::is_nested_bidirectional_collection<T>::value, int> = 1>
auto Chain(T&& iterable) {
  return ForwardChained<T>{std::forward<T>(iterable)};
}

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
template <typename T, typename details::enable_if_t<!details::is_unique_pointer_collection<T>::value, int> = 1,
          typename details::enable_if_t<details::is_bidirectional_collection<T>::value, int> = 1>
auto AsReferences(T&& iterable) {
  return Referenced<T>{std::forward<T>(iterable)};
}
template <typename T, typename details::enable_if_t<!details::is_unique_pointer_collection<T>::value, int> = 1,
          typename details::enable_if_t<!details::is_bidirectional_collection<T>::value, int> = 1>
auto AsReferences(T&& iterable) {
  return ForwardReferenced<T>{std::forward<T>(iterable)};
}

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
template <typename T, typename details::enable_if_t<details::is_unique_pointer_collection<T>::value, int> = 1,
          typename details::enable_if_t<details::is_bidirectional_collection<T>::value, int> = 1>
auto AsReferences(T&& iterable) {
  return ReferencedUnique<T>{std::forward<T>(iterable)};
}
template <typename T, typename details::enable_if_t<details::is_unique_pointer_collection<T>::value, int> = 1,
          typename details::enable_if_t<!details::is_bidirectional_collection<T>::value, int> = 1>
auto AsReferences(T&& iterable) {
  return ForwardReferencedUnique<T>{std::forward<T>(iterable)};
}

// Allows you to iterate back-to-front over a collection
//
// Simply write this:
//
// for (auto & element : Reverse(my_collection))
// {
//      // do-something
//
template <typename T> auto Reverse(T&& iterable) {
  static_assert(details::is_bidirectional_collection<T>::value,
                "Can only reverse collections that have bidirectional iterators");
  return Reversed<T>{std::forward<T>(iterable)};
}

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
template <typename T1, typename T2,
          typename details::enable_if_t<details::are_bidirectional_collections<T1, T2>::value, int> = 1>
auto Join(T1&& iterable_1, T2&& iterable_2) {
  return Joined<T1, T2>{std::forward<T1>(iterable_1), std::forward<T2>(iterable_2)};
}
template <typename T1, typename T2,
          typename details::enable_if_t<!details::are_bidirectional_collections<T1, T2>::value, int> = 1>
auto Join(T1&& iterable_1, T2&& iterable_2) {
  return ForwardJoined<T1, T2>{std::forward<T1>(iterable_1), std::forward<T2>(iterable_2)};
}

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
template <typename T, typename Function,
          typename details::enable_if_t<details::is_bidirectional_collection<T>::value, int> = 1>
auto Map(T&& data, Function mapping_function) {
  return Mapped<T, Function>(std::forward<T>(data), std::forward<Function>(mapping_function));
}
template <typename T, typename Function,
          typename details::enable_if_t<!details::is_bidirectional_collection<T>::value, int> = 1>
auto Map(T&& data, Function mapping_function) {
  return ForwardMapped<T, Function>(std::forward<T>(data), std::forward<Function>(mapping_function));
}

// Iterates over the keys of a std::map
template <typename T> auto MapKeys(T&& map) {
  return Map(std::forward<T>(map), [](const auto& map_pair) { return map_pair.first; });
}

// Iterates over the values of a std::map
template <typename T> auto MapValues(T&& map) {
  return Map(std::forward<T>(map), [](const auto& map_pair) { return map_pair.second; });
}

// Returns an iterator over the elements for which filter(element) returns 'true'
template <typename T, typename FilterFunction,
          typename details::enable_if_t<details::is_bidirectional_collection<T>::value, int> = 1>
auto Filter(T&& data, FilterFunction filter) {
  return Filtered<T, FilterFunction>(std::forward<T>(data), std::forward<FilterFunction>(filter));
}
template <typename T, typename FilterFunction,
          typename details::enable_if_t<!details::is_bidirectional_collection<T>::value, int> = 1>
auto Filter(T&& data, FilterFunction filter) {
  return ForwardFiltered<T, FilterFunction>(std::forward<T>(data), std::forward<FilterFunction>(filter));
}

//-----------------------------------------------------------------------------
// Implementation
//-----------------------------------------------------------------------------

namespace details {

// These are defined in C++14/C++17 but not in C++11
template <typename...> struct disjunction : std::false_type {};
template <typename B1> struct disjunction<B1> : B1 {};
template <typename B1, class... Bn>
struct disjunction<B1, Bn...> : conditional_t<bool(B1::value), B1, disjunction<Bn...>> {};
template <typename _Container> constexpr auto cbegin(const _Container& container) { return std::begin(container); }
template <typename _Container> constexpr auto cend(const _Container& container) { return std::end(container); }
template <typename _Container> constexpr auto rbegin(_Container& container) { return container.rbegin(); }
template <typename _Container> constexpr auto rbegin(const _Container& container) { return container.rbegin(); }
template <typename _Container> constexpr auto rend(_Container& container) { return container.rend(); }
template <typename _Container> constexpr auto rend(const _Container& container) { return container.rend(); }
template <typename _Container> constexpr auto crbegin(const _Container& container) { return container.rbegin(); }
template <typename _Container> constexpr auto crend(const _Container& container) { return container.rend(); }

// Note: 'std::is_const' is pretty strict, e.g. 'std::is_const<const int&>' returns 'false'.
//       So we're using this construct that checks if it is const in any way
template <typename T> struct is_const_type : std::false_type {};
template <typename T> struct is_const_type<const T> : std::true_type {};
template <typename T> struct is_const_type<const T&> : std::true_type {};
template <typename T> struct is_const_type<const T*> : std::true_type {};
template <typename T> struct is_const_type<T* const> : std::true_type {};
template <typename T> struct is_const_type<const T*&> : std::true_type {};
template <typename T> struct is_const_type<const T&&> : std::true_type {};
template <typename T> struct is_const_type<const T*&&> : std::true_type {};

// True if T1 or T2 is const
template <typename T1, typename T2> using is_any_const = disjunction<is_const_type<T1>, is_const_type<T2>>;

// The collection type is considered const if:
//   - it is 'const T'
//   - its non-const iterator returns a const value
template <typename T>
using is_const_collection =
    disjunction<is_const_type<T>, is_const_type<decltype(*std::declval<typename remove_cvref_t<T>::iterator>())>>;

// Returns T::iterator if T is non_const,
// returns T::const_iterator if it is const.
// e.g. non_const_iterator_t<vector<int>> --> vector<int>::iterator
// e.g. non_const_iterator_t<const vector<int>> --> vector<int>::const_iterator
template <typename T>
using non_const_iterator_t = conditional_t<is_const_type<T>::value, typename remove_cvref_t<T>::const_iterator,
                                           typename remove_cvref_t<T>::iterator>;

// Returns T::reverse_iterator if T is non_const,
// returns T::const_reverse_iterator if it is const.
// e.g. non_const_reverse_iterator_t<vector<int>> --> vector<int>::reverse_iterator
// e.g. non_const_reverse_iterator_t<const vector<int>> --> vector<int>::const_reverse_iterator
template <typename T>
using non_const_reverse_iterator_t =
    conditional_t<is_const_type<T>::value, typename remove_cvref_t<T>::const_reverse_iterator,
                  typename remove_cvref_t<T>::reverse_iterator>;

}  // namespace details

// Returned value when iterating Enumerate
template <typename T> class Item {
 public:
  Item() : Item(0, nullptr) {}

  Item(int position, T* value) : position_(position), value_(value) {}

  int Position() const { return position_; }
  const T& Value() const { return *value_; }
  T& Value() { return *value_; }

 private:
  int position_;
  T* value_;
};

template <typename _iterator, typename _return_type> class EnumeratedIterator {
 public:
  using _collection_value_type = details::remove_cvref_t<decltype(std::declval<_return_type>().Value())>;
  using _Item = Item<_collection_value_type>;

  EnumeratedIterator(_iterator begin, _iterator end, int position, int position_delta)
      : begin_(begin), end_(end), position_(position), position_delta_(position_delta), item_{} {
    SetItem();
  }

  _return_type& operator*() { return item_; }

  _return_type& operator*() const {
    // Dereferencing a const or a non-const iterator should not make a difference
    // (as the constness of the iterator has no bearing on the returned value of the collection).
    // However, as our iterator owns the return-value struct in this particular case,
    // we must cast the const away to achieve this
    return const_cast<_return_type&>(item_);
  }

  void operator++() {
    ++begin_;
    position_ += position_delta_;
    SetItem();
  }

  bool operator==(const EnumeratedIterator& other) const { return begin_ == other.begin_; }
  bool operator!=(const EnumeratedIterator& other) const { return !(*this == other); }

 private:
  void SetItem() {
    if (begin_ != end_)
      item_ = _Item{position_, &NonConstValue()};
  }

  _collection_value_type& NonConstValue() { return const_cast<_collection_value_type&>(*begin_); }

  _iterator begin_;
  _iterator end_;
  int position_;
  int position_delta_;
  _Item item_;
};

template <typename T> class ForwardEnumerated {
 public:
  using _collection_type = typename std::remove_reference<T>::type;
  using _collection_value_type = typename _collection_type::value_type;
  using _collection_const_iterator = typename _collection_type::const_iterator;
  using _collection_iterator = typename _collection_type::iterator;
  using _Item = Item<_collection_value_type>;
  using _non_const_iterator = EnumeratedIterator<_collection_iterator, _Item>;

  using value_type = _Item;
  using const_iterator = EnumeratedIterator<_collection_const_iterator, const _Item>;
  using iterator =
      typename details::conditional_t<details::is_const_collection<T>::value, const_iterator, _non_const_iterator>;

  explicit ForwardEnumerated(T&& iterable) : iterable_(std::forward<T>(iterable)) {}

  const_iterator begin() const {
    return const_iterator{details::cbegin(iterable_), details::cend(iterable_), 0, kIncrement};
  }

  const_iterator end() const {
    return const_iterator{details::cend(iterable_), details::cend(iterable_), 0, kIncrement};
  }

  iterator begin() { return iterator{std::begin(iterable_), std::end(iterable_), 0, kIncrement}; }

  iterator end() { return iterator{std::end(iterable_), std::end(iterable_), 0, kIncrement}; }

 protected:
  constexpr static int kIncrement = 1;

  T iterable_;
};

template <typename T> class Enumerated : public ForwardEnumerated<T> {
 public:
  using typename ForwardEnumerated<T>::_collection_type;
  using typename ForwardEnumerated<T>::_collection_value_type;
  using _Item = Item<_collection_value_type>;
  using _collection_const_reverse_iterator = typename _collection_type::const_reverse_iterator;
  using _collection_reverse_iterator = typename _collection_type::reverse_iterator;
  using _non_const_reverse_iterator = EnumeratedIterator<_collection_reverse_iterator, _Item>;

  using const_reverse_iterator = EnumeratedIterator<_collection_const_reverse_iterator, const _Item>;
  using reverse_iterator = typename details::conditional_t<details::is_const_collection<T>::value,
                                                           const_reverse_iterator, _non_const_reverse_iterator>;

  explicit Enumerated(T&& iterable) : ForwardEnumerated<T>(std::forward<T>(iterable)) {}

  const_reverse_iterator rbegin() const {
    return const_reverse_iterator{details::crbegin(this->iterable_), details::crend(this->iterable_), MaxPosition(),
                                  kDecrement};
  }

  const_reverse_iterator rend() const {
    return const_reverse_iterator{details::crend(this->iterable_), details::crend(this->iterable_), MaxPosition(),
                                  kDecrement};
  }

  reverse_iterator rbegin() {
    return reverse_iterator{details::rbegin(this->iterable_), details::rend(this->iterable_), MaxPosition(),
                            kDecrement};
  }

  reverse_iterator rend() {
    return reverse_iterator{details::rend(this->iterable_), details::rend(this->iterable_), MaxPosition(), kDecrement};
  }

 private:
  int MaxPosition() const { return static_cast<int>(size()) - 1; }

  std::size_t size() const { return this->iterable_.size(); }

  constexpr static int kDecrement = -1;
};

template <typename T> class ForwardIterated {
 public:
  using _iterable = typename details::remove_reference_t<T>;

  using value_type = typename _iterable::value_type;
  using const_iterator = typename _iterable::const_iterator;
  using iterator = details::non_const_iterator_t<T>;

  explicit ForwardIterated(T&& iterable) : iterable_(std::forward<T>(iterable)) {}

  iterator begin() { return std::begin(iterable_); }
  iterator end() { return std::end(iterable_); }
  const_iterator begin() const { return details::cbegin(iterable_); }
  const_iterator end() const { return details::cend(iterable_); }

 protected:
  T iterable_;
};

template <typename T> class Iterated : public ForwardIterated<T> {
 public:
  using typename ForwardIterated<T>::_iterable;
  using const_reverse_iterator = typename _iterable::const_reverse_iterator;
  using reverse_iterator = details::non_const_reverse_iterator_t<T>;

  explicit Iterated(T&& iterable) : ForwardIterated<T>(std::forward<T>(iterable)) {}

  reverse_iterator rbegin() { return details::rbegin(this->iterable_); }
  reverse_iterator rend() { return details::rend(this->iterable_); }
  const_reverse_iterator rbegin() const { return details::crbegin(this->iterable_); }
  const_reverse_iterator rend() const { return details::crend(this->iterable_); }
};

template <typename _outer_iterator, typename _inner_iterator> class ChainedIterator {
 public:
  using _inner_iterator_collection = typename _outer_iterator::value_type;
  using GetIteratorFunction = _inner_iterator (*)(_inner_iterator_collection&);

  ChainedIterator(_outer_iterator begin, _outer_iterator end, GetIteratorFunction inner_begin_getter,
                  GetIteratorFunction inner_end_getter)
      : outer_begin_(begin),
        outer_end_(end),
        inner_begin_(),
        inner_end_(),
        get_inner_begin_(inner_begin_getter),
        get_inner_end_(inner_end_getter) {
    InitializeInnerCollection();
    SkipEmptyInnerCollections();
  }
  auto& operator*() const { return *inner_begin_; }

  void operator++() {
    ++inner_begin_;
    SkipEmptyInnerCollections();
  }

  bool operator==(const ChainedIterator& other) const { return IsAtEnd() == other.IsAtEnd(); }

  bool operator!=(const ChainedIterator& other) const { return !(*this == other); }

 private:
  bool IsAtEnd() const { return outer_begin_ == outer_end_; }

  bool IsAtEndOfInnerCollection() const { return inner_begin_ == inner_end_; }

  // Initialized 'begin' and 'end' for the inner collection
  void InitializeInnerCollection() {
    if (!IsAtEnd()) {
      inner_begin_ = get_inner_begin_(const_cast<_inner_iterator_collection&>(*outer_begin_));
      inner_end_ = get_inner_end_(const_cast<_inner_iterator_collection&>(*outer_begin_));
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

  _outer_iterator outer_begin_;
  _outer_iterator outer_end_;
  _inner_iterator inner_begin_;
  _inner_iterator inner_end_;
  GetIteratorFunction get_inner_begin_;
  GetIteratorFunction get_inner_end_;
};

template <typename T> class ForwardChained {
 public:
  using _outer_collection = details::remove_cvref_t<T>;
  using _outer_const_iterator = typename _outer_collection::const_iterator;
  using _outer_non_const_iterator = typename _outer_collection::iterator;
  using _outer_iterator =
      details::conditional_t<details::is_const_type<T>::value, _outer_const_iterator, _outer_non_const_iterator>;

  using _inner_collection = details::remove_cvref_t<typename _outer_collection::value_type>;
  using _inner_const_iterator = typename _inner_collection::const_iterator;
  using _inner_non_const_iterator = typename _inner_collection::iterator;
  using _inner_iterator =
      details::conditional_t<details::is_const_type<T>::value, _inner_const_iterator, _inner_non_const_iterator>;

  using value_type = typename _inner_collection::value_type;
  using const_iterator = ChainedIterator<_outer_const_iterator, _inner_const_iterator>;
  using iterator = ChainedIterator<_outer_iterator, _inner_iterator>;

  ForwardChained(T&& data) : data_(std::forward<T>(data)) {}

  iterator begin() { return iterator{std::begin(data_), std::end(data_), GetBegin, GetEnd}; }
  iterator end() { return iterator{std::end(data_), std::end(data_), GetBegin, GetEnd}; }

  const_iterator begin() const {
    return const_iterator{details::cbegin(data_), details::cend(data_), GetConstBegin, GetConstEnd};
  }

  const_iterator end() const {
    return const_iterator{details::cend(data_), details::cend(data_), GetConstBegin, GetConstEnd};
  }

 protected:
  static _inner_iterator GetBegin(_inner_collection& collection) { return std::begin(collection); }
  static _inner_iterator GetEnd(_inner_collection& collection) { return std::end(collection); }
  static _inner_const_iterator GetConstBegin(_inner_collection& collection) { return details::cbegin(collection); }
  static _inner_const_iterator GetConstEnd(_inner_collection& collection) { return details::cend(collection); }

  T data_;
};

template <typename T> class Chained : public ForwardChained<T> {
 public:
  using typename ForwardChained<T>::_outer_collection;
  using typename ForwardChained<T>::_inner_collection;

  using _outer_const_reverse_iterator = typename _outer_collection::const_reverse_iterator;
  using _outer_non_const_reverse_iterator = typename _outer_collection::reverse_iterator;
  using _outer_reverse_iterator =
      details::conditional_t<details::is_const_type<T>::value, _outer_const_reverse_iterator,
                             _outer_non_const_reverse_iterator>;

  using _inner_const_reverse_iterator = typename _inner_collection::const_reverse_iterator;
  using _inner_non_const_reverse_iterator = typename _inner_collection::reverse_iterator;
  using _inner_reverse_iterator =
      details::conditional_t<details::is_const_type<T>::value, _inner_const_reverse_iterator,
                             _inner_non_const_reverse_iterator>;

  using const_reverse_iterator = ChainedIterator<_outer_const_reverse_iterator, _inner_const_reverse_iterator>;
  using reverse_iterator = ChainedIterator<_outer_reverse_iterator, _inner_reverse_iterator>;

  Chained(T&& data) : ForwardChained<T>(std::forward<T>(data)) {}

  reverse_iterator rbegin() {
    return reverse_iterator{details::rbegin(this->data_), details::rend(this->data_), GetReverseBegin, GetReverseEnd};
  }

  reverse_iterator rend() {
    return reverse_iterator{details::rend(this->data_), details::rend(this->data_), GetReverseBegin, GetReverseEnd};
  }

  const_reverse_iterator rbegin() const {
    return const_reverse_iterator{details::crbegin(this->data_), details::crend(this->data_), GetConstReverseBegin,
                                  GetConstReverseEnd};
  }

  const_reverse_iterator rend() const {
    return const_reverse_iterator{details::crend(this->data_), details::crend(this->data_), GetConstReverseBegin,
                                  GetConstReverseEnd};
  }

 private:
  static _inner_reverse_iterator GetReverseBegin(_inner_collection& collection) { return details::rbegin(collection); }
  static _inner_reverse_iterator GetReverseEnd(_inner_collection& collection) { return details::rend(collection); }
  static _inner_const_reverse_iterator GetConstReverseBegin(_inner_collection& collection) {
    return details::crbegin(collection);
  }

  static _inner_const_reverse_iterator GetConstReverseEnd(_inner_collection& collection) {
    return details::crend(collection);
  }
};

template <typename _iterator, typename _return_value> class ReferencedIterator {
 public:
  ReferencedIterator(_iterator begin, _iterator end) : begin_(begin), end_(end) {}

  _return_value& operator*() const { return **begin_; }

  void operator++() { ++begin_; }

  bool operator==(const ReferencedIterator& other) const { return begin_ == other.begin_; }
  bool operator!=(const ReferencedIterator& other) const { return !(*this == other); }

 private:
  _iterator begin_;
  _iterator end_;
};

template <typename T> class ForwardReferenced {
 public:
  using _collection_type = typename details::remove_reference_t<T>;
  using _collection_value_type = details::remove_pointer_t<typename _collection_type::value_type>;
  using _collection_const_iterator = typename _collection_type::const_iterator;
  using _collection_iterator = typename _collection_type::iterator;

  using value_type = _collection_value_type;
  using const_iterator = ReferencedIterator<_collection_const_iterator, const value_type>;
  using _non_const_iterator = ReferencedIterator<_collection_iterator, value_type>;
  using iterator =
      typename details::conditional_t<details::is_const_type<T>::value, const_iterator, _non_const_iterator>;

  explicit ForwardReferenced(T&& iterable) : iterable_(std::forward<T>(iterable)) {}

  iterator begin() { return iterator{std::begin(iterable_), std::end(iterable_)}; }
  iterator end() { return iterator{std::end(iterable_), std::end(iterable_)}; }
  const_iterator begin() const { return const_iterator{details::cbegin(iterable_), details::cend(iterable_)}; }
  const_iterator end() const { return const_iterator{details::cend(iterable_), details::cend(iterable_)}; }

 protected:
  T iterable_;
};

template <typename T> class Referenced : public ForwardReferenced<T> {
 public:
  using typename ForwardReferenced<T>::_collection_type;
  using typename ForwardReferenced<T>::value_type;
  using _collection_const_reverse_iterator = typename _collection_type::const_reverse_iterator;
  using _collection_reverse_iterator = typename _collection_type::reverse_iterator;

  using const_reverse_iterator = ReferencedIterator<_collection_const_reverse_iterator, const value_type>;
  using _non_const_reverse_iterator = ReferencedIterator<_collection_reverse_iterator, value_type>;
  using reverse_iterator = typename details::conditional_t<details::is_const_type<T>::value, const_reverse_iterator,
                                                           _non_const_reverse_iterator>;
  explicit Referenced(T&& iterable) : ForwardReferenced<T>(std::forward<T>(iterable)) {}

  reverse_iterator rbegin() {
    return reverse_iterator{details::rbegin(this->iterable_), details::rend(this->iterable_)};
  }

  reverse_iterator rend() { return reverse_iterator{details::rend(this->iterable_), details::rend(this->iterable_)}; }

  const_reverse_iterator rbegin() const {
    return const_reverse_iterator{details::crbegin(this->iterable_), details::crend(this->iterable_)};
  }

  const_reverse_iterator rend() const {
    return const_reverse_iterator{details::crend(this->iterable_), details::crend(this->iterable_)};
  }
};

template <typename T, typename details::enable_if_t<!details::is_unique_pointer_collection<T>::value, int>>
auto AsReferences(T&& iterable) -> Referenced<T> {
  return Referenced<T>{std::forward<T>(iterable)};
}

template <typename _iterator, typename _return_value> class ReferencedUniqueIterator {
 public:
  ReferencedUniqueIterator(_iterator begin, _iterator end) : begin_(begin), end_(end) {}

  _return_value& operator*() const {
    auto& unique_pointer = *begin_;
    auto* pointer = unique_pointer.get();
    return *pointer;
  }

  void operator++() { ++begin_; }

  bool operator==(const ReferencedUniqueIterator& other) const { return begin_ == other.begin_; }
  bool operator!=(const ReferencedUniqueIterator& other) const { return !(*this == other); }

 private:
  _iterator begin_;
  _iterator end_;
};

template <typename T> class ForwardReferencedUnique {
 public:
  using _collection_type = typename details::remove_reference_t<T>;
  using _collection_value_type = typename _collection_type::value_type;
  using _collection_const_iterator = typename _collection_type::const_iterator;
  using _collection_iterator = typename _collection_type::iterator;

  using value_type = typename _collection_value_type::element_type;
  using _non_const_iterator = ReferencedUniqueIterator<_collection_iterator, value_type>;
  using const_iterator = ReferencedUniqueIterator<_collection_const_iterator, const value_type>;
  using iterator =
      typename details::conditional_t<details::is_const_type<T>::value, const_iterator, _non_const_iterator>;

  explicit ForwardReferencedUnique(T&& iterable) : iterable_(std::forward<T>(iterable)) {}

  iterator begin() { return iterator{std::begin(iterable_), std::end(iterable_)}; }
  iterator end() { return iterator{std::end(iterable_), std::end(iterable_)}; }
  const_iterator begin() const { return const_iterator{details::cbegin(iterable_), details::cend(iterable_)}; }
  const_iterator end() const { return const_iterator{details::cend(iterable_), details::cend(iterable_)}; }

 protected:
  T iterable_;
};

template <typename T> class ReferencedUnique : public ForwardReferencedUnique<T> {
 public:
  using typename ForwardReferencedUnique<T>::_collection_type;
  using typename ForwardReferencedUnique<T>::value_type;
  using _collection_const_reverse_iterator = typename _collection_type::const_reverse_iterator;
  using _collection_reverse_iterator = typename _collection_type::reverse_iterator;

  using _non_const_reverse_iterator = ReferencedUniqueIterator<_collection_reverse_iterator, value_type>;
  using const_reverse_iterator = ReferencedUniqueIterator<_collection_const_reverse_iterator, const value_type>;
  using reverse_iterator = typename details::conditional_t<details::is_const_type<T>::value, const_reverse_iterator,
                                                           _non_const_reverse_iterator>;

  explicit ReferencedUnique(T&& iterable) : ForwardReferencedUnique<T>(std::forward<T>(iterable)) {}

  reverse_iterator rbegin() {
    return reverse_iterator{details::rbegin(this->iterable_), details::rend(this->iterable_)};
  }

  reverse_iterator rend() { return reverse_iterator{details::rend(this->iterable_), details::rend(this->iterable_)}; }

  const_reverse_iterator rbegin() const {
    return const_reverse_iterator{details::crbegin(this->iterable_), details::crend(this->iterable_)};
  }

  const_reverse_iterator rend() const {
    return const_reverse_iterator{details::crend(this->iterable_), details::crend(this->iterable_)};
  }
};

template <typename T> class Reversed {
 public:
  using _iterable = typename details::remove_reference_t<T>;

  using value_type = typename _iterable::value_type;
  using const_iterator = typename _iterable::const_reverse_iterator;
  using iterator = details::non_const_reverse_iterator_t<T>;
  using const_reverse_iterator = typename _iterable::const_iterator;
  using reverse_iterator = details::non_const_iterator_t<T>;

  explicit Reversed(T&& iterable) : iterable_(std::forward<T>(iterable)) {}

  iterator begin() { return details::rbegin(iterable_); }
  iterator end() { return details::rend(iterable_); }
  const_iterator begin() const { return details::crbegin(iterable_); }
  const_iterator end() const { return details::crend(iterable_); }
  reverse_iterator rbegin() { return std::begin(iterable_); }
  reverse_iterator rend() { return std::end(iterable_); }
  const_reverse_iterator rbegin() const { return details::cbegin(iterable_); }
  const_reverse_iterator rend() const { return details::cend(iterable_); }

 private:
  T iterable_;
};

template <typename _FirstIterator, typename _SecondIterator> class JoinedIterator {
 public:
  JoinedIterator(_FirstIterator first, _FirstIterator first_end, _SecondIterator second, _SecondIterator second_end)
      : first_(first), first_end_(first_end), second_(second), second_end_(second_end) {}
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

  bool operator==(const JoinedIterator& other) const { return first_ == other.first_ && second_ == other.second_; }
  bool operator!=(const JoinedIterator& other) const { return !(*this == other); }

 private:
  _FirstIterator first_;
  _FirstIterator first_end_;
  _SecondIterator second_;
  _SecondIterator second_end_;
};

template <typename T1, typename T2> class ForwardJoined {
 public:
  using _iterable_1 = typename details::remove_reference_t<T1>;
  using _const_iterator_1 = typename _iterable_1::const_iterator;
  using _iterator_1 = typename _iterable_1::iterator;

  using _iterable_2 = typename details::remove_reference_t<T2>;
  using _const_iterator_2 = typename _iterable_2::const_iterator;
  using _iterator_2 = typename _iterable_2::iterator;

  static_assert((std::is_same<typename _iterable_1::value_type, typename _iterable_2::value_type>::value),
                "value_type must be same type for both collections");

  using _non_const_iterator = JoinedIterator<typename _iterable_1::iterator, typename _iterable_2::iterator>;

  using value_type = typename _iterable_1::value_type;
  using const_iterator = JoinedIterator<typename _iterable_1::const_iterator, typename _iterable_2::const_iterator>;
  using iterator =
      typename details::conditional_t<details::is_any_const<T1, T2>::value, const_iterator, _non_const_iterator>;

  ForwardJoined(T1&& data_1, T2&& data_2) : first_(std::forward<T1>(data_1)), second_(std::forward<T2>(data_2)) {}

  iterator begin() { return iterator{std::begin(first_), std::end(first_), std::begin(second_), std::end(second_)}; }

  iterator end() { return iterator{std::end(first_), std::end(first_), std::end(second_), std::end(second_)}; }

  const_iterator begin() const {
    return const_iterator{details::cbegin(first_), details::cend(first_), details::cbegin(second_),
                          details::cend(second_)};
  }

  const_iterator end() const {
    return const_iterator{details::cend(first_), details::cend(first_), details::cend(second_), details::cend(second_)};
  }

 protected:
  T1 first_;
  T2 second_;
};

template <typename T1, typename T2> class Joined : public ForwardJoined<T1, T2> {
 public:
  using typename ForwardJoined<T1, T2>::_iterable_1;
  using typename ForwardJoined<T1, T2>::_iterable_2;
  using _const_reverse_iterator_1 = typename _iterable_1::const_reverse_iterator;
  using _reverse_iterator_1 = typename _iterable_1::reverse_iterator;

  using _const_reverse_iterator_2 = typename _iterable_2::const_reverse_iterator;
  using _reverse_iterator_2 = typename _iterable_2::reverse_iterator;

  using _non_const_reverse_iterator =
      JoinedIterator<typename _iterable_2::reverse_iterator, typename _iterable_1::reverse_iterator>;
  using const_reverse_iterator =
      JoinedIterator<typename _iterable_2::const_reverse_iterator, typename _iterable_1::const_reverse_iterator>;
  using reverse_iterator = typename details::conditional_t<details::is_any_const<T1, T2>::value, const_reverse_iterator,
                                                           _non_const_reverse_iterator>;
  Joined(T1&& data_1, T2&& data_2) : ForwardJoined<T1, T2>(std::forward<T1>(data_1), std::forward<T2>(data_2)) {}

  reverse_iterator rbegin() {
    return reverse_iterator{details::rbegin(this->second_), details::rend(this->second_), details::rbegin(this->first_),
                            details::rend(this->first_)};
  }

  reverse_iterator rend() {
    return reverse_iterator{details::rend(this->second_), details::rend(this->second_), details::rend(this->first_),
                            details::rend(this->first_)};
  }

  const_reverse_iterator rbegin() const {
    return const_reverse_iterator{details::crbegin(this->second_), details::crend(this->second_),
                                  details::crbegin(this->first_), details::crend(this->first_)};
  }

  const_reverse_iterator rend() const {
    return const_reverse_iterator{details::crend(this->second_), details::crend(this->second_),
                                  details::crend(this->first_), details::crend(this->first_)};
  }
};

template <typename _iterable, typename _function> class MappedIterator {
 public:
  MappedIterator(_iterable begin, _iterable end, const _function& mapping_function)
      : begin_(begin), end_(end), mapping_function_(mapping_function) {}

  auto operator*() const { return mapping_function_(*begin_); }

  void operator++() { ++begin_; }

  bool operator==(const MappedIterator& other) const { return begin_ == other.begin_; }
  bool operator!=(const MappedIterator& other) const { return !(*this == other); }

 private:
  _iterable begin_;
  _iterable end_;
  const _function& mapping_function_;
};

template <typename T, typename Function> class ForwardMapped {
 public:
  using _iterable = typename details::remove_reference_t<T>;
  using _iterable_value_type = typename _iterable::value_type;
  using _iterable_const_iterator = typename _iterable::const_iterator;
  using _iterable_iterator = typename _iterable::iterator;
  using _non_const_iterator = MappedIterator<_iterable_iterator, typename details::remove_reference_t<Function>>;

  using value_type = typename std::result_of<Function(_iterable_value_type&)>::type;
  using const_iterator = MappedIterator<_iterable_const_iterator, typename details::remove_reference_t<Function>>;
  using iterator =
      typename details::conditional_t<details::is_const_type<T>::value, const_iterator, _non_const_iterator>;

  ForwardMapped(T&& iterable, Function&& mapping_function)
      : iterable_(std::forward<T>(iterable)), mapping_function_(std::forward<Function>(mapping_function)) {}

  iterator begin() { return iterator{std::begin(iterable_), std::end(iterable_), mapping_function_}; }

  iterator end() { return iterator{std::end(iterable_), std::end(iterable_), mapping_function_}; }

  const_iterator begin() const {
    return const_iterator{details::cbegin(iterable_), details::cend(iterable_), mapping_function_};
  }

  const_iterator end() const {
    return const_iterator{details::cend(iterable_), details::cend(iterable_), mapping_function_};
  }

 protected:
  T iterable_;
  Function mapping_function_;
};

template <typename T, typename Function> class Mapped : public ForwardMapped<T, Function> {
 public:
  using typename ForwardMapped<T, Function>::_iterable;
  using _iterable_const_reverse_iterator = typename _iterable::const_reverse_iterator;
  using _iterable_reverse_iterator = typename _iterable::reverse_iterator;
  using _non_const_reverse_iterator =
      MappedIterator<_iterable_reverse_iterator, typename details::remove_reference_t<Function>>;

  using const_reverse_iterator =
      MappedIterator<_iterable_const_reverse_iterator, typename details::remove_reference_t<Function>>;
  using reverse_iterator = typename details::conditional_t<details::is_const_type<T>::value, const_reverse_iterator,
                                                           _non_const_reverse_iterator>;

  Mapped(T&& iterable, Function&& mapping_function)
      : ForwardMapped<T, Function>(std::forward<T>(iterable), std::forward<Function>(mapping_function)) {}

  reverse_iterator rbegin() {
    return reverse_iterator{details::rbegin(this->iterable_), details::rend(this->iterable_), this->mapping_function_};
  }

  reverse_iterator rend() {
    return reverse_iterator{details::rend(this->iterable_), details::rend(this->iterable_), this->mapping_function_};
  }

  const_reverse_iterator rbegin() const {
    return const_reverse_iterator{details::crbegin(this->iterable_), details::crend(this->iterable_),
                                  this->mapping_function_};
  }

  const_reverse_iterator rend() const {
    return const_reverse_iterator{details::crend(this->iterable_), details::crend(this->iterable_),
                                  this->mapping_function_};
  }
};

template <typename _iterable, typename _function> class FilterIterator {
 public:
  FilterIterator(_iterable begin, _iterable end, const _function& filter) : begin_(begin), end_(end), filter_(filter) {
    SkipFilteredEntries();
  }

  auto& operator*() const { return *begin_; }

  void operator++() {
    ++begin_;
    SkipFilteredEntries();
  }

  bool operator==(const FilterIterator& other) const { return begin_ == other.begin_; }
  bool operator!=(const FilterIterator& other) const { return !(*this == other); }

 private:
  bool IsEnd() const { return begin_ == end_; }

  bool IsFiltered() const { return !filter_(*begin_); }

  void SkipFilteredEntries() {
    while (!IsEnd() && IsFiltered())
      ++begin_;
  }

  _iterable begin_;
  _iterable end_;
  const _function& filter_;
};

template <typename T, typename FilterFunction> class ForwardFiltered {
 public:
  using _iterable = typename details::remove_reference_t<T>;
  using _iterable_const_iterator = typename _iterable::const_iterator;
  using _iterable_iterator = typename _iterable::iterator;
  using _non_const_iterator = FilterIterator<_iterable_iterator, typename details::remove_reference_t<FilterFunction>>;

  using value_type = typename _iterable::value_type;
  using const_iterator = FilterIterator<_iterable_const_iterator, typename details::remove_reference_t<FilterFunction>>;
  using iterator =
      typename details::conditional_t<details::is_const_type<T>::value, const_iterator, _non_const_iterator>;

  ForwardFiltered(T&& iterable, FilterFunction&& filter)
      : iterable_(std::forward<T>(iterable)), filter_(std::forward<FilterFunction>(filter)) {}

  iterator begin() { return iterator{std::begin(iterable_), std::end(iterable_), filter_}; }
  iterator end() { return iterator{std::end(iterable_), std::end(iterable_), filter_}; }
  const_iterator begin() const { return const_iterator{details::cbegin(iterable_), details::cend(iterable_), filter_}; }
  const_iterator end() const { return const_iterator{details::cend(iterable_), details::cend(iterable_), filter_}; }

 protected:
  T iterable_;
  FilterFunction filter_;
};

template <typename T, typename FilterFunction> class Filtered : public ForwardFiltered<T, FilterFunction> {
 public:
  using typename ForwardFiltered<T, FilterFunction>::_iterable;
  using _iterable_const_reverse_iterator = typename _iterable::const_reverse_iterator;
  using _iterable_reverse_iterator = typename _iterable::reverse_iterator;
  using _non_const_reverse_iterator =
      FilterIterator<_iterable_reverse_iterator, typename details::remove_reference_t<FilterFunction>>;

  using const_reverse_iterator =
      FilterIterator<_iterable_const_reverse_iterator, typename details::remove_reference_t<FilterFunction>>;
  using reverse_iterator = typename details::conditional_t<details::is_const_type<T>::value, const_reverse_iterator,
                                                           _non_const_reverse_iterator>;

  Filtered(T&& iterable, FilterFunction&& filter)
      : ForwardFiltered<T, FilterFunction>(std::forward<T>(iterable), std::forward<FilterFunction>(filter)) {}

  reverse_iterator rbegin() {
    return reverse_iterator{details::rbegin(this->iterable_), details::rend(this->iterable_), this->filter_};
  }

  reverse_iterator rend() {
    return reverse_iterator{details::rend(this->iterable_), details::rend(this->iterable_), this->filter_};
  }

  const_reverse_iterator rbegin() const {
    return const_reverse_iterator{details::crbegin(this->iterable_), details::crend(this->iterable_), this->filter_};
  }

  const_reverse_iterator rend() const {
    return const_reverse_iterator{details::crend(this->iterable_), details::crend(this->iterable_), this->filter_};
  }
};
}  // namespace iterators

#endif  // _CPP_ITERATORS_ITERATORS_H_
