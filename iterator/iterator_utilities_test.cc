
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
  for (const int& value : iterable)
    values.push_back(value);
  return values;
}

template <typename _Iterable>
auto GetNonConstValues(_Iterable* iterable) -> std::list<typename _Iterable::value_type> {
  std::list<typename _Iterable::value_type> values{};
  for (int& value : *iterable)
    values.push_back(value);
  return values;
}

TEST(IterateTest, can_iterate_const_collection) {
  list<int> collection{AnyCollection()};
  const auto& const_collection = collection;

  auto const_iterator = Iterate(const_collection);
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
  const auto& const_collection = collection;

  auto const_iterator = Reverse(const_collection);
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

}  // namespace iterator
