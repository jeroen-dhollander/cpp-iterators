
#include "iterators.h"
#include <list>
#include <memory>
#include <vector>
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace iterators {
using std::list;
using std::string;
using std::vector;
using testing::ElementsAre;
using testing::ElementsAreArray;

// Prints a human-readable string of the given type, e.g. 'int const *'.
// https://stackoverflow.com/a/20170989/3490116
template <class T>
std::string type_name() {
  typedef typename std::remove_reference<T>::type TR;
  std::unique_ptr<char, void (*)(void*)> own(
#ifndef _MSC_VER
      abi::__cxa_demangle(typeid(TR).name(), nullptr, nullptr, nullptr),
#else
      nullptr,
#endif
      std::free);
  std::string r = own != nullptr ? own.get() : typeid(TR).name();
  if (std::is_const<TR>::value)
    r += " const";
  if (std::is_volatile<TR>::value)
    r += " volatile";
  if (std::is_lvalue_reference<T>::value)
    r += "&";
  else if (std::is_rvalue_reference<T>::value)
    r += "&&";
  return r;
}

// Checks that the type returned by '_actual' matches the '_expected' type.
// On failure, prints a human-readable message
#define EXPECT_TYPE(_expected, _actual) EXPECT_EQ(type_name<_expected>(), type_name<_actual>())

// Tests that the non_const iterator has the correct type, and returns values of the expected type
#define TEST_NON_CONST_ITERATOR(_iterable, _expected_type)           \
  using IterableClass = decltype(_iterable);                         \
  /* Test begin/end return instances of _iterable::iterator */       \
  EXPECT_TYPE(IterableClass::iterator, decltype(_iterable.begin())); \
  EXPECT_TYPE(IterableClass::iterator, decltype(_iterable.end()));   \
  /* Test iterator return value */                                   \
  EXPECT_TYPE(_expected_type, decltype(std::declval<IterableClass::iterator>().operator*()));

// Tests that the const iterator has the correct type, and returns values of the expected type
#define TEST_CONST_ITERATOR(_iterable, _expected_type)                                    \
  using IterableClass = decltype(_iterable);                                              \
  /* Test begin/end return instances of _iterable::const_iterator */                      \
  EXPECT_TYPE(IterableClass::const_iterator, decltype(std::as_const(_iterable).begin())); \
  EXPECT_TYPE(IterableClass::const_iterator, decltype(std::as_const(_iterable).end()));   \
  /* Test const_iterator return value */                                                  \
  EXPECT_TYPE(_expected_type, decltype(std::declval<IterableClass::const_iterator>().operator*()));

list<int> AnyCollection() {
  return list<int>{1, 3, 5};
}

list<int> AnyCollectionReversed() {
  return {5, 3, 1};
}

// Executes '++' on each value in the iterable.
// Used to ensure we can non-const access the elements
template <typename _Iterable>
auto& IncreaseAll(_Iterable* iterable) {
  for (auto& value : *iterable)
    value++;
  return *iterable;
}

// Doubles each value in the iterable.
// Used to ensure we can non-const access the elements
template <typename _Iterable>
auto DoubleAll(_Iterable* iterable) {
  for (auto& value : *iterable)
    value *= 2;
  return *iterable;
}

// Appends the given string to each (string) value in the iterable.
// Used to ensure we can non-const access the elements
template <typename _Iterable>
auto AppendAll(_Iterable* iterable, string suffix) {
  list<std::string> result;
  for (auto& value : *iterable) {
    value += suffix;
    result.push_back(value);
  }
  return result;
}

template <typename _Enumerator>
string FormatEnumerate(const _Enumerator& iterable) {
  string result{};
  for (const auto& item : iterable)
    result += std::to_string(item.position) + ": " + item.value + ", ";
  return result;
}

// Executes '++' on the values of the enumerator.
// Used to ensure we can non-const access the elements
template <typename _Enumerator>
auto IncrementValues(_Enumerator* iterable) {
  for (auto& item : *iterable)
    item.value++;
  return *iterable;
}

TEST(IsConstTest, SanityCheck) {
  // Test that our 'details::is_const_type' utility works as expected
#define IS_CONST(_type) details::is_const_type<_type>::value
  EXPECT_TRUE(IS_CONST(const int&));
  EXPECT_TRUE(IS_CONST(const int));
  EXPECT_TRUE(IS_CONST(const int*));
  EXPECT_TRUE(IS_CONST(int* const));
  EXPECT_TRUE(IS_CONST(const int*&));
  EXPECT_TRUE(IS_CONST(const int&&));
  EXPECT_TRUE(IS_CONST(const int*&&));
  EXPECT_TRUE(IS_CONST(int* const&));
  EXPECT_FALSE(IS_CONST(int&));
  EXPECT_FALSE(IS_CONST(int));
  EXPECT_FALSE(IS_CONST(int*));
  EXPECT_FALSE(IS_CONST(int*&));
  EXPECT_FALSE(IS_CONST(int&&));
  EXPECT_FALSE(IS_CONST(int*&&));
}

TEST(TypeNameTest, SanityCheck) {
  // Test that our 'type_name' works as expected
  EXPECT_EQ("int const&", type_name<const int&>());
  EXPECT_EQ("int const", type_name<const int>());
  EXPECT_EQ("int const *", type_name<const int*>());
  EXPECT_EQ("int * const", type_name<int* const>());
  EXPECT_EQ("int const *&", type_name<const int*&>());
  EXPECT_EQ("int const&&", type_name<const int&&>());
  EXPECT_EQ("int const *&&", type_name<const int*&&>());
  EXPECT_EQ("int * const&", type_name<int* const&>());
  EXPECT_EQ("int&", type_name<int&>());
  EXPECT_EQ("int", type_name<int>());
  EXPECT_EQ("int *", type_name<int*>());
  EXPECT_EQ("int *&", type_name<int*&>());
  EXPECT_EQ("int&&", type_name<int&&>());
  EXPECT_EQ("int *&&", type_name<int*&&>());
}

TEST(NonConstCollection, SanityCheck) {
  EXPECT_TYPE(vector<int>::iterator, details::non_const_iterator_t<vector<int>>);
  EXPECT_TYPE(vector<int>::iterator, details::non_const_iterator_t<vector<int>&>);
  EXPECT_TYPE(vector<int>::iterator, details::non_const_iterator_t<vector<int>&&>);
  EXPECT_TYPE(vector<int>::const_iterator, details::non_const_iterator_t<const vector<int>>);
  EXPECT_TYPE(vector<int>::const_iterator, details::non_const_iterator_t<const vector<int>&>);
}

TEST(EnumerateTest, can_enumerate_const_collection) {
  vector<char> collection{'A', 'B', 'C'};
  string expected{"0: A, 1: B, 2: C, "};

  const auto& const_collection = collection;

  auto const_iterator = Enumerate(const_collection);
  EXPECT_EQ(expected, FormatEnumerate(const_iterator));
}

TEST(EnumerateTest, can_const_enumerate_non_const_collection) {
  vector<char> collection{'A', 'B', 'C'};
  string expected{"0: A, 1: B, 2: C, "};

  auto iterator = Enumerate(collection);
  EXPECT_EQ(expected, FormatEnumerate(iterator));
}

TEST(EnumerateTest, can_non_const_enumerate_non_const_collection) {
  vector<char> collection{'A', 'B', 'C'};
  string expected{"0: B, 1: C, 2: D, "};

  auto iterator = Enumerate(collection);
  EXPECT_EQ(expected, FormatEnumerate(IncrementValues(&iterator)));
}

TEST(EnumerateTest, can_const_enumerate_rvalue_collection) {
  auto iterator = Enumerate(vector<char>{'A', 'B', 'C'});
  string expected{"0: A, 1: B, 2: C, "};

  EXPECT_EQ(expected, FormatEnumerate(iterator));
}

TEST(EnumerateTest, can_non_const_enumerate_rvalue_collection) {
  auto iterator = Enumerate(vector<char>{'A', 'B', 'C'});
  string expected{"0: B, 1: C, 2: D, "};

  EXPECT_EQ(expected, FormatEnumerate(IncrementValues(&iterator)));
}

TEST(IterateTest, ReturnsCorrectValues) {
  vector<int> collection{1, 3, 5};
  auto iterator = Iterate(collection);

  EXPECT_THAT(iterator, ElementsAre(1, 3, 5));
  EXPECT_THAT(std::as_const(iterator), ElementsAre(1, 3, 5));
}

TEST(IterateTest, CanModifyValues) {
  vector<int> collection{1, 3, 5};
  auto iterator = Iterate(collection);

  int& first = *iterator.begin();
  first = 123;

  EXPECT_THAT(iterator, ElementsAre(123, 3, 5));
}

TEST(IterateTest, IterateOverNonConstCollection) {
  vector<int> collection{1, 3, 5};
  auto iterator = Iterate(collection);

  TEST_NON_CONST_ITERATOR(iterator, int&);
  TEST_CONST_ITERATOR(iterator, int const&);
  EXPECT_TYPE(int, decltype(iterator)::value_type);
}

TEST(IterateTest, IterateOverConstCollection) {
  // Note: In this case, even iterating non-const uses a const_iterator
  vector<int> collection{1, 3, 5};
  auto iterator = Iterate(std::as_const(collection));

  TEST_NON_CONST_ITERATOR(iterator, int const&);
  TEST_CONST_ITERATOR(iterator, int const&);
  EXPECT_TYPE(int, decltype(iterator)::value_type);
}

TEST(IterateTest, IterateRvalueCollection) {
  vector<int> collection{1, 3, 5};
  auto iterator = Iterate(std::move(collection));

  TEST_NON_CONST_ITERATOR(iterator, int&);
  TEST_CONST_ITERATOR(iterator, int const&);
  EXPECT_TYPE(int, decltype(iterator)::value_type);
}

TEST(ReverseTest, can_iterate_const_collection) {
  list<int> collection{AnyCollection()};

  auto const_iterator = Reverse(std::as_const(collection));
  EXPECT_THAT(std::as_const(const_iterator), ElementsAreArray(AnyCollectionReversed()));
}

TEST(ReverseTest, can_const_iterate_non_const_collection) {
  list<int> collection{AnyCollection()};

  auto result = Reverse(collection);
  EXPECT_THAT(std::as_const(result), ElementsAreArray(AnyCollectionReversed()));
}

TEST(ReverseTest, can_non_const_iterate_non_const_collection) {
  list<int> collection{AnyCollection()};

  auto result = Reverse(collection);
  EXPECT_THAT(IncreaseAll(&result), ElementsAreArray(IncreaseAll(&AnyCollectionReversed())));
}

TEST(ReverseTest, can_const_iterate_rvalue_collection) {
  auto result = Reverse(AnyCollection());
  EXPECT_THAT(std::as_const(result), ElementsAreArray(AnyCollectionReversed()));
}

TEST(ReverseTest, can_non_const_iterate_rvalue_collection) {
  auto result = Reverse(AnyCollection());
  EXPECT_THAT(IncreaseAll(&result), ElementsAreArray(IncreaseAll(&AnyCollectionReversed())));
}

TEST(JoinTest, can_iterate_2_const_collections) {
  list<int> first{1, 2, 3};
  vector<int> second{4, 5, 6};

  const auto& const_first = first;
  const auto& const_second = second;

  auto const_iterator = Join(const_first, const_second);
  EXPECT_THAT(std::as_const(const_iterator), ElementsAre(1, 2, 3, 4, 5, 6));
}

TEST(JoinTest, can_const_iterate_2_non_const_collection) {
  list<int> first{1, 2, 3};
  vector<int> second{4, 5, 6};

  auto result = Join(first, second);
  EXPECT_THAT(std::as_const(result), ElementsAre(1, 2, 3, 4, 5, 6));
}

TEST(JoinTest, can_non_const_iterate_2_non_const_collection) {
  list<int> first{1, 2, 3};
  vector<int> second{4, 5, 6};

  auto result = Join(first, second);
  EXPECT_THAT(IncreaseAll(&result), ElementsAre(2, 3, 4, 5, 6, 7));
}

TEST(JoinTest, can_const_iterate_2_rvalue_collection) {
  auto result = Join(list<int>{1, 2, 3}, vector<int>{4, 5, 6});
  EXPECT_THAT(std::as_const(result), ElementsAre(1, 2, 3, 4, 5, 6));
}

TEST(JoinTest, can_non_const_iterate_2_rvalue_collection) {
  auto result = Join(list<int>{1, 2, 3}, vector<int>{4, 5, 6});
  EXPECT_THAT(IncreaseAll(&result), ElementsAre(2, 3, 4, 5, 6, 7));
}

TEST(MapTest, can_const_map_const_collection) {
  std::list<int> input{1, 3, 5};

  auto map = Map(std::as_const(input), [](const auto& value) { return std::to_string(value); });
  EXPECT_THAT(std::as_const(map), ElementsAre("1", "3", "5"));
}

TEST(MapTest, can_const_map_non_const_collection) {
  std::list<int> input{1, 3, 5};

  auto map = Map(input, [](const auto& value) { return std::to_string(value); });
  EXPECT_THAT(std::as_const(map), ElementsAre("1", "3", "5"));
}

TEST(MapTest, can_non_const_map_non_const_collection) {
  std::list<int> input{1, 3, 5};

  auto map = Map(input, [](const auto& value) { return std::to_string(value); });
  EXPECT_THAT(AppendAll(&map, "-suffix"), ElementsAre("1-suffix", "3-suffix", "5-suffix"));
}

TEST(MapTest, can_const_map_rvalue_collection) {
  std::list<int> input{1, 3, 5};

  auto map = Map(std::move(input), [](const auto& value) { return std::to_string(value); });
  EXPECT_THAT(std::as_const(map), ElementsAre("1", "3", "5"));
}

TEST(MapTest, can_non_const_map_rvalue_collection) {
  std::list<int> input{1, 3, 5};

  auto map = Map(std::move(input), [](const auto& value) { return std::to_string(value); });
  EXPECT_THAT(AppendAll(&map, "-suffix"), ElementsAre("1-suffix", "3-suffix", "5-suffix"));
}

TEST(MapTest, MapKeys__extract_keys_from_std_map) {
  std::map<string, int> input{{{"a", 1}, {"b", 2}}};

  EXPECT_THAT(MapKeys(input), ElementsAre("a", "b"));
}

TEST(MapTest, MapValues__extract_values_from_std_map) {
  std::map<string, int> input{{{"a", 1}, {"b", 2}}};

  EXPECT_THAT(MapValues(input), ElementsAre(1, 2));
}

TEST(FilterTest, can_filter_first_value) {
  std::list<int> input{1, 2};
  auto filter_function = [](const int& value) { return value > 1; };

  EXPECT_THAT(Filter(input, filter_function), ElementsAre(2));
}

TEST(FilterTest, can_filter_consecutive_values) {
  std::list<int> input{1, 2, 3, 4, 5};
  auto filter_function = [](const int& value) { return value == 1 || value == 5; };

  EXPECT_THAT(Filter(input, filter_function), ElementsAre(1, 5));
}

TEST(FilterTest, can_const_filter_const_collection) {
  std::list<int> input{1, 2, 3, 4, 5};
  auto filter_function = [](const int& value) { return value % 2 == 0; };

  auto result = Filter(std::as_const(input), filter_function);
  EXPECT_THAT(std::as_const(result), ElementsAre(2, 4));
}

TEST(FilterTest, can_const_filter_non_const_collection) {
  std::list<int> input{1, 2, 3, 4, 5};
  auto filter_function = [](const int& value) { return value % 2 == 0; };

  auto result = Filter(input, filter_function);
  EXPECT_THAT(std::as_const(result), ElementsAre(2, 4));
}

TEST(FilterTest, can_non_const_filter_non_const_collection) {
  std::list<int> input{1, 2, 3, 4, 5};
  auto filter_function = [](const int& value) { return value % 2 == 0; };

  auto result = Filter(input, filter_function);
  EXPECT_THAT(DoubleAll(&result), ElementsAre(4, 8));
}

TEST(FilterTest, can_const_filter_rvalue_collection) {
  std::list<int> input{1, 2, 3, 4, 5};
  auto filter_function = [](const int& value) { return value % 2 == 0; };

  auto result = Filter(std::move(input), filter_function);
  EXPECT_THAT(std::as_const(result), ElementsAre(2, 4));
}

TEST(FilterTest, can_non_const_filter_rvalue_collection) {
  std::list<int> input{1, 2, 3, 4, 5};
  auto filter_function = [](const int& value) { return value % 2 == 0; };

  auto result = Filter(std::move(input), filter_function);
  EXPECT_THAT(DoubleAll(&result), ElementsAre(4, 8));
}

TEST(AsReferences, can_const_iterate_const_pointer_collection) {
  int values[] = {1, 3, 5};
  std::list<int*> input{&values[0], &values[1], &values[2]};

  auto result = AsReferences(std::as_const(input));
  EXPECT_THAT(std::as_const(result), ElementsAreArray(values));
}

TEST(AsReferences, can_const_iterate_non_const_pointer_collection) {
  int values[] = {1, 3, 5};
  std::list<int*> input{&values[0], &values[1], &values[2]};

  auto result = AsReferences(input);
  EXPECT_THAT(std::as_const(result), ElementsAreArray(values));
}

TEST(AsReferences, can_non_const_iterate_non_const_pointer_collection) {
  int values[] = {1, 3, 5};
  std::list<int*> input{&values[0], &values[1], &values[2]};

  auto result = AsReferences(std::move(input));
  EXPECT_THAT(IncreaseAll(&result), ElementsAre(2, 4, 6));
}

TEST(AsReferences, can_const_iterate_rvalue_pointer_collection) {
  int values[] = {1, 3, 5};
  std::list<int*> input{&values[0], &values[1], &values[2]};

  auto result = AsReferences(std::move(input));
  EXPECT_THAT(std::as_const(result), ElementsAreArray(values));
}

TEST(AsReferences, can_non_const_iterate_rvalue_pointer_collection) {
  int values[] = {1, 3, 5};
  std::list<int*> input{&values[0], &values[1], &values[2]};

  auto result = AsReferences(std::move(input));
  EXPECT_THAT(IncreaseAll(&result), ElementsAre(2, 4, 6));
}

std::list<std::unique_ptr<int>> ToUniquePtrList(int* values, int values_size) {
  std::list<std::unique_ptr<int>> result{};
  for (int i = 0; i < values_size; i++)
    result.push_back(std::make_unique<int>(values[i]));
  return result;
}

TEST(AsReferences_unique_ptr, can_const_iterate_const_pointer_collection) {
  int values[] = {1, 3, 5};
  auto input{ToUniquePtrList(values, 3)};

  auto result = AsReferences(std::as_const(input));
  EXPECT_THAT(std::as_const(result), ElementsAreArray(values));
}

TEST(AsReferences_unique_ptr, can_const_iterate_non_const_pointer_collection) {
  int values[] = {1, 3, 5};
  auto input{ToUniquePtrList(values, 3)};

  auto result = AsReferences(input);
  EXPECT_THAT(std::as_const(result), ElementsAreArray(values));
}

TEST(AsReferences_unique_ptr, can_non_const_iterate_non_const_pointer_collection) {
  int values[] = {1, 3, 5};
  auto input{ToUniquePtrList(values, 3)};

  auto result = AsReferences(std::move(input));
  EXPECT_THAT(IncreaseAll(&result), ElementsAre(2, 4, 6));
}

TEST(AsReferences_unique_ptr, can_const_iterate_rvalue_pointer_collection) {
  int values[] = {1, 3, 5};
  auto input{ToUniquePtrList(values, 3)};

  auto result = AsReferences(std::move(input));
  EXPECT_THAT(std::as_const(result), ElementsAreArray(values));
}

TEST(AsReferences_unique_ptr, can_non_const_iterate_rvalue_pointer_collection) {
  int values[] = {1, 3, 5};
  auto input{ToUniquePtrList(values, 3)};

  auto result = AsReferences(std::move(input));
  EXPECT_THAT(IncreaseAll(&result), ElementsAre(2, 4, 6));
}

TEST(ChainTest, can_iterate_non_empty_collections) {
  vector<list<int>> collection{{1, 2, 3}, {4, 5, 6}};

  auto result = Chain(collection);
  EXPECT_THAT(result, ElementsAre(1, 2, 3, 4, 5, 6));
}

TEST(ChainTest, skips_over_empty_first_collections) {
  vector<list<int>> collection{{}, {}, {1, 2, 3}};

  auto result = Chain(collection);
  EXPECT_THAT(result, ElementsAre(1, 2, 3));
}

TEST(ChainTest, skips_over_empty_collections) {
  vector<list<int>> collection{{1}, {}, {}, {2, 3}, {}};

  auto result = Chain(collection);
  EXPECT_THAT(result, ElementsAre(1, 2, 3));
}

TEST(ChainTest, survives_empty_outer_collection) {
  vector<list<int>> collection{};

  auto result = Chain(collection);
  EXPECT_THAT(result, ElementsAre());
}

TEST(ChainTest, can_iterate_const_collections) {
  vector<list<int>> collection{{1, 2, 3}, {4, 5, 6}};

  auto result = Chain(std::as_const(collection));
  EXPECT_THAT(std::as_const(result), ElementsAre(1, 2, 3, 4, 5, 6));
}

TEST(ChainTest, can_const_iterate_non_const_collection) {
  vector<list<int>> collection{{1, 2, 3}, {4, 5, 6}};

  auto result = Chain(collection);
  EXPECT_THAT(std::as_const(result), ElementsAre(1, 2, 3, 4, 5, 6));
}

TEST(ChainTest, can_non_const_iterate_non_const_collection) {
  vector<list<int>> collection{{1, 2, 3}, {4, 5, 6}};

  auto result = Chain(collection);
  EXPECT_THAT(DoubleAll(&result), ElementsAre(2, 4, 6, 8, 10, 12));
}

TEST(ChainTest, can_const_iterate_rvalue_collection) {
  auto result = Chain(vector<list<int>>{{1, 2, 3}, {4, 5, 6}});

  EXPECT_THAT(std::as_const(result), ElementsAre(1, 2, 3, 4, 5, 6));
}

TEST(ChainTest, can_non_const_iterate_rvalue_collection) {
  auto result = Chain(vector<list<int>>{{1, 2, 3}, {4, 5, 6}});

  EXPECT_THAT(DoubleAll(&result), ElementsAre(2, 4, 6, 8, 10, 12));
}

}  // namespace iterators
