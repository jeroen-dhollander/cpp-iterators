
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

TEST(IterateTest, can_iterate_const_collection) {
  list<int> collection{AnyCollection()};

  auto result = Iterate(std::as_const(collection));
  EXPECT_THAT(std::as_const(result), ElementsAreArray(collection));
}

TEST(IterateTest, can_const_iterate_non_const_collection) {
  list<int> collection{AnyCollection()};

  auto result = Iterate(collection);
  EXPECT_THAT(std::as_const(result), ElementsAreArray(collection));
}

TEST(IterateTest, can_non_const_iterate_non_const_collection) {
  list<int> collection{AnyCollection()};

  auto iterator = Iterate(collection);
  EXPECT_THAT(IncreaseAll(&iterator), ElementsAreArray(IncreaseAll(&collection)));
}

TEST(IterateTest, can_const_iterate_rvalue_collection) {
  auto result = Iterate(AnyCollection());
  EXPECT_THAT(std::as_const(result), ElementsAreArray(AnyCollection()));
}

TEST(IterateTest, can_non_const_iterate_rvalue_collection) {
  auto result = Iterate(AnyCollection());
  EXPECT_THAT(IncreaseAll(&result), ElementsAreArray(IncreaseAll(&AnyCollection())));
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
