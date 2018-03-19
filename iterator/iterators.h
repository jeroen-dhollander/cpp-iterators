#pragma once

#ifndef _CPP_ITERATORS_ITERATORS_H_
#define _CPP_ITERATORS_ITERATORS_H_

#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

namespace iterators {
template <typename>
class Chained;
template <typename>
class Enumerated;
template <typename>
class Iterated;
template <typename>
class ReferencedUnique;
template <typename>
class Referenced;
template <typename>
class Reversed;
template <typename, typename>
class Joined;
template <typename, typename>
class Mapped;
template <typename, typename>
class Filtered;

namespace details {
// clang-format off
template <typename T> struct is_unique_pointer_helper : std::false_type {};
template <typename T> struct is_unique_pointer_helper<std::unique_ptr<T>> : std::true_type {};
template <typename T> struct is_unique_pointer : is_unique_pointer_helper<typename std::remove_cv_t<T>> {};
// clang-format on

template <typename T>
struct remove_cvref {
  typedef std::remove_cv_t<std::remove_reference_t<T>> type;
};
template <typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

// Returns true if T is a collection over unique_ptr, e.g. vector<std::unique_ptr<int>>
template <typename T>
struct is_unique_pointer_collection : is_unique_pointer<typename remove_cvref_t<T>::value_type> {};
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
auto MapKeys(_Iterable&& map);

// Iterates over the values of a std::map
template <typename _Iterable>
auto MapValues(_Iterable&& map);

// Returns an iterator over the elements for which filter(element) returns 'true'
template <typename _Iterable, typename FilterFunction>
auto Filter(_Iterable&& data, FilterFunction filter) -> Filtered<_Iterable, FilterFunction>;

//-----------------------------------------------------------------------------
// Implementation
//-----------------------------------------------------------------------------

namespace details {
// clang-format off
// Note: 'std::is_const' is pretty strict, e.g. 'std::is_const<const int&>' returns 'false'.
//       So we're using this construct that checks if it is const in any way
template<typename T> struct is_const_type : std::false_type {};
template<typename T> struct is_const_type<const T> : std::true_type {};
template<typename T> struct is_const_type<const T&> : std::true_type {};
template<typename T> struct is_const_type<const T*> : std::true_type {};
template<typename T> struct is_const_type<T*const> : std::true_type {};
template<typename T> struct is_const_type<const T*&> : std::true_type {};
template<typename T> struct is_const_type<const T&&> : std::true_type {};
template<typename T> struct is_const_type<const T*&&> : std::true_type {};
// clang-format on

template <typename T>
constexpr bool is_const_type_v = is_const_type<T>::value;

template <typename T1, typename T2>
constexpr bool is_any_const_v = std::disjunction_v<is_const_type<T1>, is_const_type<T2>>;

// Returns T::iterator if T is non_const,
// returns T::const_iterator if it is const.
// e.g. non_const_iterator_t<vector<int>> --> vector<int>::iterator
// e.g. non_const_iterator_t<const vector<int>> --> vector<int>::const_iterator
template <typename T>
using non_const_iterator_t = typename std::conditional_t<is_const_type_v<T>, typename remove_cvref_t<T>::const_iterator,
                                                         typename remove_cvref_t<T>::iterator>;

// Returns T::reverse_iterator if T is non_const,
// returns T::const_reverse_iterator if it is const.
// e.g. non_const_reverse_iterator_t<vector<int>> --> vector<int>::reverse_iterator
// e.g. non_const_reverse_iterator_t<const vector<int>> --> vector<int>::const_reverse_iterator
template <typename T>
using non_const_reverse_iterator_t =
    typename std::conditional_t<is_const_type_v<T>, typename remove_cvref_t<T>::const_reverse_iterator,
                                typename remove_cvref_t<T>::reverse_iterator>;

}  // namespace details

// Returned value when iterating Enumerate
template <typename T>
class Item {
 public:
  Item() : Item(0, nullptr) {
  }

  Item(int position, T* value) : position_(position), value_(value) {
  }

  int Position() const {
    return position_;
  }
  const T& Value() const {
    return *value_;
  }
  T& Value() {
    return *value_;
  }

 private:
  int position_;
  T* value_;
};

template <typename T>
class Enumerated {
 public:
  template <typename, typename>
  class _Iterator;
  using _collection_type = typename std::remove_reference<T>::type;
  using _collection_value_type = typename _collection_type::value_type;
  using _collection_const_iterator = typename _collection_type::const_iterator;
  using _collection_iterator = typename _collection_type::iterator;
  using _collection_const_reverse_iterator = typename _collection_type::const_reverse_iterator;
  using _collection_reverse_iterator = typename _collection_type::reverse_iterator;
  using _Item = Item<_collection_value_type>;
  using _non_const_iterator = _Iterator<_collection_iterator, _Item>;
  using _non_const_reverse_iterator = _Iterator<_collection_reverse_iterator, _Item>;

  using value_type = _Item;
  using const_iterator = _Iterator<_collection_const_iterator, const _Item>;
  using iterator = typename std::conditional_t<details::is_const_type_v<T>, const_iterator, _non_const_iterator>;
  using const_reverse_iterator = _Iterator<_collection_const_reverse_iterator, const _Item>;
  using reverse_iterator =
      typename std::conditional_t<details::is_const_type_v<T>, const_reverse_iterator, _non_const_reverse_iterator>;

  explicit Enumerated(T&& iterable) : iterable_(std::forward<T>(iterable)) {
  }

  const_iterator begin() const {
    return MakeConstIterator(std::cbegin(iterable_));
  }

  const_iterator end() const {
    return MakeConstIterator(std::cend(iterable_));
  }

  iterator begin() {
    return MakeIterator(std::begin(iterable_));
  }

  iterator end() {
    return MakeIterator(std::end(iterable_));
  }

  const_reverse_iterator rbegin() const {
    return MakeConstReverseIterator(std::crbegin(iterable_));
  }

  const_reverse_iterator rend() const {
    return MakeConstReverseIterator(std::crend(iterable_));
  }

  reverse_iterator rbegin() {
    return MakeReverseIterator(std::rbegin(iterable_));
  }

  reverse_iterator rend() {
    return MakeReverseIterator(std::rend(iterable_));
  }

  template <typename __iterator, typename __return_type>
  class _Iterator {
   public:
    _Iterator(__iterator begin, __iterator end, int position, int position_delta)
        : begin_(begin), end_(end), position_(position), position_delta_(position_delta), item_{} {
      SetItem();
    }

    __return_type& operator*() {
      return item_;
    }

    __return_type& operator*() const {
      // Dereferencing a const or a non-const iterator should not make a difference
      // (as the constness of the iterator has no bearing on the returned value of the collection).
      // However, as our iterator owns the return-value struct in this particular case,
      // we must cast the const away to achieve this
      return const_cast<__return_type&>(item_);
    }

    void operator++() {
      ++begin_;
      position_ += position_delta_;
      SetItem();
    }

    bool operator==(const _Iterator& other) const {
      return begin_ == other.begin_;
    }
    bool operator!=(const _Iterator& other) const {
      return !(*this == other);
    }

   private:
    void SetItem() {
      if (begin_ != end_)
        item_ = _Item{position_, &NonConstValue()};
    }

    _collection_value_type& NonConstValue() {
      return const_cast<_collection_value_type&>(*begin_);
    }

    __iterator begin_;
    __iterator end_;
    int position_;
    int position_delta_;
    _Item item_;
  };

 private:
  constexpr static int kIncrement = 1;
  constexpr static int kDecrement = -1;

  std::size_t size() const {
    return iterable_.size();
  }

  const_iterator MakeConstIterator(_collection_const_iterator begin_iterator) const {
    return const_iterator(begin_iterator, std::cend(iterable_), 0, kIncrement);
  }

  iterator MakeIterator(_collection_iterator begin_iterator) {
    return iterator(begin_iterator, std::end(iterable_), 0, kIncrement);
  }

  const_reverse_iterator MakeConstReverseIterator(_collection_const_reverse_iterator begin_iterator) const {
    return const_reverse_iterator(begin_iterator, std::rend(iterable_), size() - 1, kDecrement);
  }

  reverse_iterator MakeReverseIterator(_collection_reverse_iterator begin_iterator) {
    return reverse_iterator(begin_iterator, std::rend(iterable_), size() - 1, kDecrement);
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
  using const_reverse_iterator = typename _iterable::const_reverse_iterator;
  using reverse_iterator = details::non_const_reverse_iterator_t<T>;

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

  reverse_iterator rbegin() {
    return std::rbegin(iterable_);
  }

  reverse_iterator rend() {
    return std::rend(iterable_);
  }

  const_reverse_iterator rbegin() const {
    return std::crbegin(iterable_);
  }

  const_reverse_iterator rend() const {
    return std::crend(iterable_);
  }

 private:
  T iterable_;
};

template <typename T>
auto Iterate(T&& iterable) -> Iterated<T> {
  return Iterated<T>{std::forward<T>(iterable)};
}

template <typename T>
class Chained {
 public:
  template <typename, typename>
  class _Iterator;

  using _outer_collection = details::remove_cvref_t<T>;
  using _outer_const_iterator = typename _outer_collection::const_iterator;
  using _outer_non_const_iterator = typename _outer_collection::iterator;
  using _outer_iterator =
      std::conditional_t<details::is_const_type_v<T>, _outer_const_iterator, _outer_non_const_iterator>;
  using _outer_const_reverse_iterator = typename _outer_collection::const_reverse_iterator;
  using _outer_non_const_reverse_iterator = typename _outer_collection::reverse_iterator;
  using _outer_reverse_iterator =
      std::conditional_t<details::is_const_type_v<T>, _outer_const_reverse_iterator, _outer_non_const_reverse_iterator>;

  using _inner_collection = details::remove_cvref_t<typename _outer_collection::value_type>;
  using _inner_const_iterator = typename _inner_collection::const_iterator;
  using _inner_non_const_iterator = typename _inner_collection::iterator;
  using _inner_iterator =
      std::conditional_t<details::is_const_type_v<T>, _inner_const_iterator, _inner_non_const_iterator>;
  using _inner_const_reverse_iterator = typename _inner_collection::const_reverse_iterator;
  using _inner_non_const_reverse_iterator = typename _inner_collection::reverse_iterator;
  using _inner_reverse_iterator =
      std::conditional_t<details::is_const_type_v<T>, _inner_const_reverse_iterator, _inner_non_const_reverse_iterator>;

  using value_type = typename _inner_collection::value_type;
  using const_iterator = _Iterator<_outer_const_iterator, _inner_const_iterator>;
  using iterator = _Iterator<_outer_iterator, _inner_iterator>;
  using const_reverse_iterator = _Iterator<_outer_const_reverse_iterator, _inner_const_reverse_iterator>;
  using reverse_iterator = _Iterator<_outer_reverse_iterator, _inner_reverse_iterator>;

  Chained(T&& data) : data_(std::forward<T>(data)) {
  }

  iterator begin() {
    return iterator{std::begin(data_), std::end(data_), GetBegin, GetEnd};
  }

  iterator end() {
    return iterator{std::end(data_), std::end(data_), GetBegin, GetEnd};
  }

  const_iterator begin() const {
    return const_iterator{std::cbegin(data_), std::cend(data_), GetConstBegin, GetConstEnd};
  }

  const_iterator end() const {
    return const_iterator{std::cend(data_), std::cend(data_), GetConstBegin, GetConstEnd};
  }

  reverse_iterator rbegin() {
    return reverse_iterator{std::rbegin(data_), std::rend(data_), GetReverseBegin, GetReverseEnd};
  }

  reverse_iterator rend() {
    return reverse_iterator{std::rend(data_), std::rend(data_), GetReverseBegin, GetReverseEnd};
  }

  const_reverse_iterator rbegin() const {
    return const_reverse_iterator{std::crbegin(data_), std::crend(data_), GetConstReverseBegin, GetConstReverseEnd};
  }

  const_reverse_iterator rend() const {
    return const_reverse_iterator{std::crend(data_), std::crend(data_), GetConstReverseBegin, GetConstReverseEnd};
  }

  template <typename __outer_iterator, typename __inner_iterator>
  class _Iterator {
   public:
    using _inner_iterator_collection = typename __outer_iterator::value_type;
    using GetIteratorFunction = __inner_iterator (*)(_inner_iterator_collection&);

    _Iterator(__outer_iterator begin, __outer_iterator end, GetIteratorFunction inner_begin_getter,
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

    __outer_iterator outer_begin_;
    __outer_iterator outer_end_;
    __inner_iterator inner_begin_;
    __inner_iterator inner_end_;
    GetIteratorFunction get_inner_begin_;
    GetIteratorFunction get_inner_end_;
  };

 private:
  static _inner_iterator GetBegin(_inner_collection& collection) {
    return std::begin(collection);
  }

  static _inner_iterator GetEnd(_inner_collection& collection) {
    return std::end(collection);
  }

  static _inner_const_iterator GetConstBegin(_inner_collection& collection) {
    return std::cbegin(collection);
  }

  static _inner_const_iterator GetConstEnd(_inner_collection& collection) {
    return std::cend(collection);
  }

  static _inner_reverse_iterator GetReverseBegin(_inner_collection& collection) {
    return std::rbegin(collection);
  }

  static _inner_reverse_iterator GetReverseEnd(_inner_collection& collection) {
    return std::rend(collection);
  }

  static _inner_const_reverse_iterator GetConstReverseBegin(_inner_collection& collection) {
    return std::crbegin(collection);
  }

  static _inner_const_reverse_iterator GetConstReverseEnd(_inner_collection& collection) {
    return std::crend(collection);
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
  template <typename, typename>
  class _Iterator;

  using _collection_type = typename std::remove_reference_t<T>;
  using _collection_value_type = std::remove_pointer_t<typename _collection_type::value_type>;
  using _collection_const_iterator = typename _collection_type::const_iterator;
  using _collection_iterator = typename _collection_type::iterator;
  using _collection_const_reverse_iterator = typename _collection_type::const_reverse_iterator;
  using _collection_reverse_iterator = typename _collection_type::reverse_iterator;

  using value_type = _collection_value_type;
  using const_iterator = _Iterator<_collection_const_iterator, const value_type>;
  using _non_const_iterator = _Iterator<_collection_iterator, value_type>;
  using iterator = typename std::conditional_t<details::is_const_type_v<T>, const_iterator, _non_const_iterator>;
  using const_reverse_iterator = _Iterator<_collection_const_reverse_iterator, const value_type>;
  using _non_const_reverse_iterator = _Iterator<_collection_reverse_iterator, value_type>;
  using reverse_iterator =
      typename std::conditional_t<details::is_const_type_v<T>, const_reverse_iterator, _non_const_reverse_iterator>;

  explicit Referenced(T&& iterable) : iterable_(std::forward<T>(iterable)) {
  }

  iterator begin() {
    return iterator{std::begin(iterable_), std::end(iterable_)};
  }

  iterator end() {
    return iterator{std::end(iterable_), std::end(iterable_)};
  }

  const_iterator begin() const {
    return const_iterator{std::cbegin(iterable_), std::cend(iterable_)};
  }

  const_iterator end() const {
    return const_iterator{std::cend(iterable_), std::cend(iterable_)};
  }

  reverse_iterator rbegin() {
    return reverse_iterator{std::rbegin(iterable_), std::rend(iterable_)};
  }

  reverse_iterator rend() {
    return reverse_iterator{std::rend(iterable_), std::rend(iterable_)};
  }

  const_reverse_iterator rbegin() const {
    return const_reverse_iterator{std::crbegin(iterable_), std::crend(iterable_)};
  }

  const_reverse_iterator rend() const {
    return const_reverse_iterator{std::crend(iterable_), std::crend(iterable_)};
  }

  template <typename __iterator, typename __return_value>
  class _Iterator {
   public:
    _Iterator(__iterator begin, __iterator end) : begin_(begin), end_(end) {
    }

    __return_value& operator*() const {
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
  T iterable_;
};

template <typename T, typename std::enable_if_t<!details::is_unique_pointer_collection<T>::value, int>>
auto AsReferences(T&& iterable) -> Referenced<T> {
  return Referenced<T>{std::forward<T>(iterable)};
}

template <typename T>
class ReferencedUnique {
 public:
  template <typename, typename>
  class _Iterator;

  using _collection_type = typename std::remove_reference_t<T>;
  using _collection_value_type = typename _collection_type::value_type;
  using _collection_const_iterator = typename _collection_type::const_iterator;
  using _collection_iterator = typename _collection_type::iterator;
  using _collection_const_reverse_iterator = typename _collection_type::const_reverse_iterator;
  using _collection_reverse_iterator = typename _collection_type::reverse_iterator;

  using value_type = typename _collection_value_type::element_type;
  using _non_const_iterator = _Iterator<_collection_iterator, value_type>;
  using const_iterator = _Iterator<_collection_const_iterator, const value_type>;
  using iterator = typename std::conditional_t<details::is_const_type_v<T>, const_iterator, _non_const_iterator>;
  using _non_const_reverse_iterator = _Iterator<_collection_reverse_iterator, value_type>;
  using const_reverse_iterator = _Iterator<_collection_const_reverse_iterator, const value_type>;
  using reverse_iterator =
      typename std::conditional_t<details::is_const_type_v<T>, const_reverse_iterator, _non_const_reverse_iterator>;

  explicit ReferencedUnique(T&& iterable) : iterable_(std::forward<T>(iterable)) {
  }

  iterator begin() {
    return iterator{std::begin(iterable_), std::end(iterable_)};
  }

  iterator end() {
    return iterator{std::end(iterable_), std::end(iterable_)};
  }

  const_iterator begin() const {
    return const_iterator{std::cbegin(iterable_), std::cend(iterable_)};
  }

  const_iterator end() const {
    return const_iterator{std::cend(iterable_), std::cend(iterable_)};
  }

  reverse_iterator rbegin() {
    return reverse_iterator{std::rbegin(iterable_), std::rend(iterable_)};
  }

  reverse_iterator rend() {
    return reverse_iterator{std::rend(iterable_), std::rend(iterable_)};
  }

  const_reverse_iterator rbegin() const {
    return const_reverse_iterator{std::crbegin(iterable_), std::crend(iterable_)};
  }

  const_reverse_iterator rend() const {
    return const_reverse_iterator{std::crend(iterable_), std::crend(iterable_)};
  }

  template <typename __iterator, typename __return_value>
  class _Iterator {
   public:
    _Iterator(__iterator begin, __iterator end) : begin_(begin), end_(end) {
    }

    __return_value& operator*() const {
      auto& unique_pointer = *begin_;
      auto* pointer = unique_pointer.get();
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
  using iterator = details::non_const_reverse_iterator_t<T>;
  using const_reverse_iterator = typename _iterable::const_iterator;
  using reverse_iterator = details::non_const_iterator_t<T>;

  explicit Reversed(T&& iterable) : iterable_(std::forward<T>(iterable)) {
  }

  iterator begin() {
    return std::rbegin(iterable_);
  }

  iterator end() {
    return std::rend(iterable_);
  }

  const_iterator begin() const {
    return std::crbegin(iterable_);
  }

  const_iterator end() const {
    return std::crend(iterable_);
  }

  reverse_iterator rbegin() {
    return std::begin(iterable_);
  }

  reverse_iterator rend() {
    return std::end(iterable_);
  }

  const_reverse_iterator rbegin() const {
    return std::cbegin(iterable_);
  }

  const_reverse_iterator rend() const {
    return std::cend(iterable_);
  }

 private:
  T iterable_;
};

template <typename T>
auto Reverse(T&& iterable) -> Reversed<T> {
  return Reversed<T>{std::forward<T>(iterable)};
}

template <typename T1, typename T2>
class Joined {
 public:
  using _iterable_1 = typename std::remove_reference_t<T1>;
  using _const_iterator_1 = typename _iterable_1::const_iterator;
  using _iterator_1 = typename _iterable_1::iterator;
  using _const_reverse_iterator_1 = typename _iterable_1::const_reverse_iterator;
  using _reverse_iterator_1 = typename _iterable_1::reverse_iterator;

  using _iterable_2 = typename std::remove_reference_t<T2>;
  using _const_iterator_2 = typename _iterable_2::const_iterator;
  using _iterator_2 = typename _iterable_2::iterator;
  using _const_reverse_iterator_2 = typename _iterable_2::const_reverse_iterator;
  using _reverse_iterator_2 = typename _iterable_2::reverse_iterator;

  static_assert((std::is_same<typename _iterable_1::value_type, typename _iterable_2::value_type>::value),
                "value_type must be same type for both collections");

  template <typename, typename>
  class _Iterator;
  using _non_const_iterator = _Iterator<typename _iterable_1::iterator, typename _iterable_2::iterator>;
  using _non_const_reverse_iterator =
      _Iterator<typename _iterable_2::reverse_iterator, typename _iterable_1::reverse_iterator>;

  using value_type = typename _iterable_1::value_type;
  using const_iterator = _Iterator<typename _iterable_1::const_iterator, typename _iterable_2::const_iterator>;
  using iterator = typename std::conditional_t<details::is_any_const_v<T1, T2>, const_iterator, _non_const_iterator>;
  using const_reverse_iterator =
      _Iterator<typename _iterable_2::const_reverse_iterator, typename _iterable_1::const_reverse_iterator>;
  using reverse_iterator =
      typename std::conditional_t<details::is_any_const_v<T1, T2>, const_reverse_iterator, _non_const_reverse_iterator>;

  Joined(T1&& data_1, T2&& data_2) : first_(std::forward<T1>(data_1)), second_(std::forward<T2>(data_2)) {
  }

  iterator begin() {
    return iterator{std::begin(first_), std::end(first_), std::begin(second_), std::end(second_)};
  }

  iterator end() {
    return iterator{std::end(first_), std::end(first_), std::end(second_), std::end(second_)};
  }

  const_iterator begin() const {
    return const_iterator{std::cbegin(first_), std::cend(first_), std::cbegin(second_), std::cend(second_)};
  }

  const_iterator end() const {
    return const_iterator{std::cend(first_), std::cend(first_), std::cend(second_), std::cend(second_)};
  }

  reverse_iterator rbegin() {
    return reverse_iterator{std::rbegin(second_), std::rend(second_), std::rbegin(first_), std::rend(first_)};
  }

  reverse_iterator rend() {
    return reverse_iterator{std::rend(second_), std::rend(second_), std::rend(first_), std::rend(first_)};
  }

  const_reverse_iterator rbegin() const {
    return const_reverse_iterator{std::crbegin(second_), std::crend(second_), std::crbegin(first_), std::crend(first_)};
  }

  const_reverse_iterator rend() const {
    return const_reverse_iterator{std::crend(second_), std::crend(second_), std::crend(first_), std::crend(first_)};
  }

  template <typename _FirstIterator, typename _SecondIterator>
  class _Iterator {
   public:
    _Iterator(_FirstIterator first, _FirstIterator first_end, _SecondIterator second, _SecondIterator second_end)
        : first_(first), first_end_(first_end), second_(second), second_end_(second_end) {
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
  template <typename, typename>
  class _Iterator;
  using _iterable = typename std::remove_reference_t<T>;
  using _iterable_value_type = typename _iterable::value_type;
  using _iterable_const_iterator = typename _iterable::const_iterator;
  using _iterable_iterator = typename _iterable::iterator;
  using _iterable_const_reverse_iterator = typename _iterable::const_reverse_iterator;
  using _iterable_reverse_iterator = typename _iterable::reverse_iterator;
  using _non_const_iterator = _Iterator<_iterable_iterator, typename std::remove_reference_t<Function>>;
  using _non_const_reverse_iterator = _Iterator<_iterable_reverse_iterator, typename std::remove_reference_t<Function>>;

  using value_type = typename std::result_of<Function(_iterable_value_type&)>::type;
  using const_iterator = _Iterator<_iterable_const_iterator, typename std::remove_reference_t<Function>>;
  using iterator = typename std::conditional_t<details::is_const_type_v<T>, const_iterator, _non_const_iterator>;
  using const_reverse_iterator =
      _Iterator<_iterable_const_reverse_iterator, typename std::remove_reference_t<Function>>;
  using reverse_iterator =
      typename std::conditional_t<details::is_const_type_v<T>, const_reverse_iterator, _non_const_reverse_iterator>;

  Mapped(T&& iterable, Function&& mapping_function)
      : iterable_(std::forward<T>(iterable)), mapping_function_(std::forward<Function>(mapping_function)) {
  }

  iterator begin() {
    return iterator{std::begin(iterable_), std::end(iterable_), mapping_function_};
  }

  iterator end() {
    return iterator{std::end(iterable_), std::end(iterable_), mapping_function_};
  }

  const_iterator begin() const {
    return const_iterator{std::cbegin(iterable_), std::cend(iterable_), mapping_function_};
  }

  const_iterator end() const {
    return const_iterator{std::cend(iterable_), std::cend(iterable_), mapping_function_};
  }

  reverse_iterator rbegin() {
    return reverse_iterator{std::rbegin(iterable_), std::rend(iterable_), mapping_function_};
  }

  reverse_iterator rend() {
    return reverse_iterator{std::rend(iterable_), std::rend(iterable_), mapping_function_};
  }

  const_reverse_iterator rbegin() const {
    return const_reverse_iterator{std::crbegin(iterable_), std::crend(iterable_), mapping_function_};
  }

  const_reverse_iterator rend() const {
    return const_reverse_iterator{std::crend(iterable_), std::crend(iterable_), mapping_function_};
  }

  template <typename __iterable, typename _function>
  class _Iterator {
   public:
    _Iterator(__iterable begin, __iterable end, const _function& mapping_function)
        : begin_(begin), end_(end), mapping_function_(mapping_function) {
    }

    auto operator*() const {
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
  T iterable_;
  Function mapping_function_;
};

template <typename _Iterable, typename Function>
auto Map(_Iterable&& data, Function mapping_function) -> Mapped<_Iterable, Function> {
  return Mapped<_Iterable, Function>(std::forward<_Iterable>(data), std::forward<Function>(mapping_function));
}

template <typename _Iterable>
auto MapKeys(_Iterable&& map) {
  return Map(std::forward<_Iterable>(map), [](const auto& map_pair) { return map_pair.first; });
}

template <typename _Iterable>
auto MapValues(_Iterable&& map) {
  return Map(std::forward<_Iterable>(map), [](const auto& map_pair) { return map_pair.second; });
}

template <typename T, typename FilterFunction>
class Filtered {
 public:
  template <typename, typename>
  class _Iterator;
  using _iterable = typename std::remove_reference_t<T>;
  using _iterable_const_iterator = typename _iterable::const_iterator;
  using _iterable_iterator = typename _iterable::iterator;
  using _iterable_const_reverse_iterator = typename _iterable::const_reverse_iterator;
  using _iterable_reverse_iterator = typename _iterable::reverse_iterator;
  using _non_const_iterator = _Iterator<_iterable_iterator, typename std::remove_reference_t<FilterFunction>>;
  using _non_const_reverse_iterator =
      _Iterator<_iterable_reverse_iterator, typename std::remove_reference_t<FilterFunction>>;

  using value_type = typename _iterable::value_type;
  using const_iterator = _Iterator<_iterable_const_iterator, typename std::remove_reference_t<FilterFunction>>;
  using iterator = typename std::conditional_t<details::is_const_type_v<T>, const_iterator, _non_const_iterator>;
  using const_reverse_iterator =
      _Iterator<_iterable_const_reverse_iterator, typename std::remove_reference_t<FilterFunction>>;
  using reverse_iterator =
      typename std::conditional_t<details::is_const_type_v<T>, const_reverse_iterator, _non_const_reverse_iterator>;

  Filtered(T&& iterable, FilterFunction&& filter)
      : iterable_(std::forward<T>(iterable)), filter_(std::forward<FilterFunction>(filter)) {
  }

  iterator begin() {
    return iterator{std::begin(iterable_), std::end(iterable_), filter_};
  }

  iterator end() {
    return iterator{std::end(iterable_), std::end(iterable_), filter_};
  }

  const_iterator begin() const {
    return const_iterator{std::cbegin(iterable_), std::cend(iterable_), filter_};
  }

  const_iterator end() const {
    return const_iterator{std::cend(iterable_), std::cend(iterable_), filter_};
  }

  reverse_iterator rbegin() {
    return reverse_iterator{std::rbegin(iterable_), std::rend(iterable_), filter_};
  }

  reverse_iterator rend() {
    return reverse_iterator{std::rend(iterable_), std::rend(iterable_), filter_};
  }

  const_reverse_iterator rbegin() const {
    return const_reverse_iterator{std::crbegin(iterable_), std::crend(iterable_), filter_};
  }

  const_reverse_iterator rend() const {
    return const_reverse_iterator{std::crend(iterable_), std::crend(iterable_), filter_};
  }

  template <typename __iterable, typename _function>
  class _Iterator {
   public:
    _Iterator(__iterable begin, __iterable end, const _function& filter) : begin_(begin), end_(end), filter_(filter) {
      SkipFilteredEntries();
    }

    auto& operator*() const {
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
  T iterable_;
  FilterFunction filter_;
};

template <typename _Iterable, typename FilterFunction>
auto Filter(_Iterable&& data, FilterFunction filter) -> Filtered<_Iterable, FilterFunction> {
  return Filtered<_Iterable, FilterFunction>(std::forward<_Iterable>(data), std::forward<FilterFunction>(filter));
}

}  // namespace iterators

#endif  // _CPP_ITERATORS_ITERATORS_H_
