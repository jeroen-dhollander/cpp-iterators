
#include "iterator_utilities.h"
#include <list>
#include <vector>
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace iterator {
using std::list;
using std::vector;
using testing::ElementsAre;
using testing::ElementsAreArray;

list<int> AnyCollection() {
  return list<int>{1, 3, 5};
}

list<int> AnyCollectionReversed() {
  return {5, 3, 1};
}

template <typename _Iterable>
auto GetConstValues(const _Iterable& iterable) -> std::list<typename _Iterable::value_type> {
  std::list<typename _Iterable::value_type> values{};
  for (const auto& value : iterable)
    values.push_back(value);
  return values;
}

template <typename _Iterable>
auto GetNonConstValues(_Iterable* iterable) -> std::list<typename _Iterable::value_type> {
  std::list<typename _Iterable::value_type> values{};
  for (auto& value : *iterable)
    values.push_back(value);
  return values;
}

TEST(IterateTest, can_iterate_const_collection) {
  list<int> collection{AnyCollection()};

  auto const_iterator = Iterate(std::as_const(collection));
  EXPECT_THAT(GetConstValues(const_iterator), ElementsAreArray(collection));
}

TEST(IterateTest, can_const_iterate_non_const_collection) {
  list<int> collection{AnyCollection()};

  auto iterator = Iterate(collection);
  EXPECT_THAT(GetConstValues(iterator), ElementsAreArray(collection));
}

TEST(IterateTest, can_non_const_iterate_non_const_collection) {
  list<int> collection{AnyCollection()};

  auto iterator = Iterate(collection);
  EXPECT_THAT(GetNonConstValues(&iterator), ElementsAreArray(collection));
}

TEST(IterateTest, can_const_iterate_rvalue_collection) {
  auto iterator = Iterate(AnyCollection());
  EXPECT_THAT(GetConstValues(iterator), ElementsAreArray(AnyCollection()));
}

TEST(IterateTest, can_non_const_iterate_rvalue_collection) {
  auto iterator = Iterate(AnyCollection());
  EXPECT_THAT(GetNonConstValues(&iterator), ElementsAreArray(AnyCollection()));
}

TEST(ReverseTest, can_iterate_const_collection) {
  list<int> collection{AnyCollection()};

  auto const_iterator = Reverse(std::as_const(collection));
  EXPECT_THAT(GetConstValues(const_iterator), ElementsAreArray(AnyCollectionReversed()));
}

TEST(ReverseTest, can_const_iterate_non_const_collection) {
  list<int> collection{AnyCollection()};

  auto iterator = Reverse(collection);
  EXPECT_THAT(GetConstValues(iterator), ElementsAreArray(AnyCollectionReversed()));
}

TEST(ReverseTest, can_non_const_iterate_non_const_collection) {
  list<int> collection{AnyCollection()};

  auto iterator = Reverse(collection);
  EXPECT_THAT(GetNonConstValues(&iterator), ElementsAreArray(AnyCollectionReversed()));
}

TEST(ReverseTest, can_const_iterate_rvalue_collection) {
  auto iterator = Reverse(AnyCollection());

  EXPECT_THAT(GetConstValues(iterator), ElementsAreArray(AnyCollectionReversed()));
}

TEST(ReverseTest, can_non_const_iterate_rvalue_collection) {
  auto iterator = Reverse(AnyCollection());
  EXPECT_THAT(GetNonConstValues(&iterator), ElementsAreArray(AnyCollectionReversed()));
}

TEST(JoinTest, can_iterate_2_const_collections) {
  list<int> first{1, 2, 3};
  vector<int> second{4, 5, 6};

  const auto& const_first = first;
  const auto& const_second = second;

  auto const_iterator = Join(const_first, const_second);
  EXPECT_THAT(GetConstValues(const_iterator), ElementsAre(1, 2, 3, 4, 5, 6));
}

TEST(JoinTest, can_const_iterate_2_non_const_collection) {
  list<int> first{1, 2, 3};
  vector<int> second{4, 5, 6};

  auto iterator = Join(first, second);
  EXPECT_THAT(GetConstValues(iterator), ElementsAre(1, 2, 3, 4, 5, 6));
}

TEST(JoinTest, can_non_const_iterate_2_non_const_collection) {
  list<int> first{1, 2, 3};
  vector<int> second{4, 5, 6};

  auto iterator = Join(first, second);

  EXPECT_THAT(GetNonConstValues(&iterator), ElementsAre(1, 2, 3, 4, 5, 6));
}

TEST(JoinTest, can_const_iterate_2_rvalue_collection) {
  auto iterator = Join(list<int>{1, 2, 3}, vector<int>{4, 5, 6});
  EXPECT_THAT(GetConstValues(iterator), ElementsAre(1, 2, 3, 4, 5, 6));
}

TEST(JoinTest, can_non_const_iterate_2_rvalue_collection) {
  auto iterator = Join(list<int>{1, 2, 3}, vector<int>{4, 5, 6});
  EXPECT_THAT(GetNonConstValues(&iterator), ElementsAre(1, 2, 3, 4, 5, 6));
}

TEST(MapTest, can_const_map_const_collection) {
  std::list<int> input{1, 3, 5};

  auto map = Map(std::as_const(input), [](const auto& value) { return std::to_string(value); });
  EXPECT_THAT(GetConstValues(map), ElementsAre("1", "3", "5"));
}

TEST(MapTest, can_non_const_map_const_collection) {
  // Note it might seem weird we can non-const iterate over a const-collection,
  // but in reality we're not iterating over the collection,
  // we're actually iterating over the return values of the mapping-function
  std::list<int> input{1, 3, 5};

  auto map = Map(std::as_const(input), [](const auto& value) { return std::to_string(value); });
  EXPECT_THAT(GetNonConstValues(&map), ElementsAre("1", "3", "5"));
}

TEST(MapTest, can_const_map_non_const_collection) {
  std::list<int> input{1, 3, 5};

  auto map = Map(input, [](const auto& value) { return std::to_string(value); });
  EXPECT_THAT(GetConstValues(map), ElementsAre("1", "3", "5"));
}

TEST(MapTest, can_non_const_map_non_const_collection) {
  std::list<int> input{1, 3, 5};

  auto map = Map(input, [](const auto& value) { return std::to_string(value); });
  EXPECT_THAT(GetNonConstValues(&map), ElementsAre("1", "3", "5"));
}

TEST(MapTest, can_const_map_rvalue_collection) {
  std::list<int> input{1, 3, 5};

  auto map = Map(std::move(input), [](const auto& value) { return std::to_string(value); });
  EXPECT_THAT(GetConstValues(map), ElementsAre("1", "3", "5"));
}

TEST(MapTest, can_non_const_map_rvalue_collection) {
  std::list<int> input{1, 3, 5};

  auto map = Map(std::move(input), [](const auto& value) { return std::to_string(value); });
  EXPECT_THAT(GetNonConstValues(&map), ElementsAre("1", "3", "5"));
}

}  // namespace iterator
