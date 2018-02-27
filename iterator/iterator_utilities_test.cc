
#include "iterator_utilities.h"
#include <list>
#include <memory>
#include <vector>
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace iterator {
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
auto IncreaseAll(_Iterable* iterable) {
  for (auto& value : *iterable)
    value++;
  return *iterable;
}

// Doubles each value
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

}  // namespace iterator
