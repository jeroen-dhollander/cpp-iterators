# Detailed library content
At the time of writing, the library contains the following utilities:

| Method | Description | Details
| --- | --- | --- |
| `Iterate(collection)` | Creates an iterator over a collection. |[details](#iterate) |
| `Enumerate(collection)` <br> `collection.enumerate()` | Iterates over 'position, value' objects.<br> Use `item.Position()` and `item.Value()` on the returned object | [details](#enumerate) |
| `Reverse(collection)`<br>`collection.Reverse()` | Iterates over the collection, last-to-first | [details](#reverse) |
| `Map(collection, mapping_function)`<br>`collection.map(mapping_function)` |  Applies the mapping-function to all the items in the collection. | [details](#map) |
| `MapKeys(collection)`<br>`MapValues(collection)` | Specialized functions that iterate over the keys/values of a `std::map` | |
| `Filter(collection, filter_function)`<br>`collection.filter(filter_function)` |  Only returns the elements for which the `filter_function` returns `true`.  | [details](#filter) |
| `AsReferences(collection)`<br>`collection.AsReferences()` | Converts a collection of pointers/unique-pointers to a collection of references. | [details](#asreferences) |
| `Join(collection_1, collection_2)` | First walks the elements in the first collection, then the ones in the second collection.<br>Both collections must use the same `value` type. |[details](#join) |
| `Chain(collection_of_collections)` | Chains the values of a collection of collections.<br>e.g. `vector<list<int>>` |[details](#chain) |
| `Zip(collection_1, collection_2)` | Creates tuples of the elements in both collections.<br>Use `item.First()`, `item.Second()` on the returned object. | [details](#zip) |

# The problem

With the introduction of range-based for loops in C++11, looping over a collection is as easy as writing

```
for (auto value : collection)
    <use value>
```

As these for loops rely on iterators, in order to use range-based for loops over your own data-types, you either need to:
    - Expose your internal STL collection containing your data
    - Write your own iterator
    - Copy your data in a temporary STL collection

All these methods have drawbacks, ranging from performance overhead to writing boilerplate code for each class.

Furthermore, there are limitations to the range-based for-loops
- You can only iterate forward (and not in reverse)
- Each class can only have a single iterator defined (you can not define 2 iterators for the same class, e.g. for children and attributes).
- You can not manipulate the elements while iterating (like mapping, filtering) without manually having to write a specialized iterator.
- Iterators often leak implementation details (like whether you stored the data as unique_ptr or not).


This library hopes to solve many if not all of these issues,
by providing a set of flexible and reusable iterator classes that allow you to do all the above and more.
# The solution

This library introduces a set of **lazy evaluated iterators** that make it easy to do common operations like
* **filter** elements from a collection
* **chain** 2 collections together
* **map** elements to something else
and much more.

# Example
## XmlElement
Suppose we're writing our own `XmlElement` class.
Each `XmlElement` has a name, namespace, a set of children, and a map of attributes.
```
class XmlElement {
 private:
  std::string name_;
  std::string namespace_;
  std::vector<std::unique_ptr<XmlElement>> children_;
  std::map<std::string, std::string> attributes_;
};
```

## Children
Now we want to allow the user to iterate over the children. A simple approach would be to simply add these methods
```
  auto Children() const { return children_; }
  auto Children() {  return children_; }    
```
However this has a bunch of drawbacks:
* The caller has to iterate over `unique_ptr<XmlElement>`. They don't care about the `unique_ptr`. That's an implementation detail of our class, and we do not want to leak this through the API (after all, we might later want to drop the `unique_ptr`, without impacting our API).
* The non-const version allows the caller to add/remove children, which is not something we want.

With the `iterators` library, we can simply write this:
```
  auto Children() const { return iterators::AsReferences(children_); }
  auto Children() { return iterators::AsReferences(children_); }
```
and now the user can use this in a range-based for-loop
```
for (XmlElement & child: parent.Children())
     <snip>
```
## Children in namespace
Another method we want to provide is to allow iteration over all the children that are part of a given namespace.
This can be achieved with the `Filter` iterator
```
  auto ChildrenInNamespace(const std::string& name_space) const {
    return iterators::Filter(Children(),
                             [name_space](const XmlElement& child) { return child.Namespace() == name_space; });
  }
```
To make this look even nicer, all iterators have a build-in method to do the filtering, so we could have simply written this
```
  auto ChildrenInNamespace(const std::string& name_space) const {
    return Children().filter([name_space](const XmlElement& child) { return child.Namespace() == name_space; });
  }
````
## Attributes
To iterate over the names of all the attributes, we use `Map` to extract the keys from `attributes_`:
```
  auto AttributeNames() {
    return iterators::Map(attributes_, [](const auto& pair) { return pair.first; });
````
Here we map the 'key, value' pair returned by iterating over an `std::map` to the key.

As extracting a key from a `std::map` is a very common operator, a build-in operator `MapKeys` is provided for us, so this example becomes
```
  auto AttributeNames() { return iterators::MapKeys(attributes_); }
```
## Full implementation
For reference, here's the `XmlElement` class we created so far
```
#include <map>
#include <memory>
#include <vector>
#include "iterators.h"

class XmlElement {
 public:
  auto Children() const { return iterators::AsReferences(children_); }
  auto Children() { return iterators::AsReferences(children_); }
  auto ChildrenInNamespace(const std::string& name_space) const {
    return Children().filter([name_space](const XmlElement& child) { return child.Namespace() == name_space; });
  }

  auto AttributeNames() { return iterators::MapKeys(attributes_); }

  const std::string& Namespace() const { return namespace_; }

 private:
  std::string name_;
  std::string namespace_;
  std::vector<std::unique_ptr<XmlElement>> children_;
  std::map<std::string, std::string> attributes_;
};
```
While incomplete, it still provides a lot of functionality in only a few lines of code.

# Library methods

## Iterate

Iterates over all values in a given collection.

```
std::vector<char> collection{'A', 'B', 'C'};

for (char value : iterators::Iterate(collection))
    printf("%c, ", value);

// prints "A, B, C, "
```

**use-case**:
* return values without allowing the caller to add/remove elements.

## Enumerate

Iterates over position/value objects.
Use `item.Position()` and `item.Value()` to retrieve the position/value respectively.

```
std::vector<char> collection{'A', 'B', 'C'};

for (const auto & item : iterators::Enumerate(collection))
    printf("%i: %c, ", item.Position(), item.Value());

// prints "0: A, 1: B, 2: C, "
```

If your collection is already part of `iterators`, you can use the member function `collection.enumerate()`.

## Reverse

Iterates over the collection, last-to-first.
The collection must have a bidirectional iterator,
i.e. `rbegin` and `rend` must be defined.

```
std::vector<char> collection{'A', 'B', 'C'};

for (char value : iterators::Reverse(collection))
    printf("%c, ", value);

// prints "C, B, A, "
```

If your collection is already part of `iterators`, you can use the member function `collection.reverse()`.
### Map

Applies a mapping-function to all the items in the input list.
The mapping-function must take a reference to collection value-type  as input.

```
std::vector<char> collection{'A', 'B', 'C'};

auto mapped = iterators::Map(
    collection,
    [](char & value) { return (int)value; }
);

for (int value: mapped)
    printf("%i, ", value);

// prints "41, 42, 43, "
```

The mapping function can be a lambda or a real function.
If the collection is `const`, the mapping function must take a **const** reference to the collection value-type as input.


If your collection is already part of `iterators`, you can use the member function `collection.map(<function>)`.

### Filter

Iterates over the elements for which the filter-function returns `true`.
The filter-function must take a const reference to the collection value-type.

```
std::vector<char> collection{'A', 'B', 'C'};

auto filtered = iterators::Filter(
    collection,
    [](char & value) { return value != 'B'; }
);

for (char value: filtered )
    printf("%c, ", value);

// prints "A, C, "
```

If your collection is already part of `iterators`, you can use the member function `collection.filter(<function>)`.

### AsReferences

Allows you to iterate over a collection of pointers (or unique-pointers) using references


```
char values[] = {'A', 'B', 'C'};

std::vector<char*> collection{&values[0], &values[1], &values[2]};

for (char value: iterators::AsReferences(collection))
    printf("%c, ", value);

// prints "A, B, C, "
```

**use-cases**:
* Allows you to hide the fact that you internally use pointers to your data.

### Join

Iterates over the values of 2 collections,
returning first the values of the first collection,
next the values of the second collection.

Both collections do not need to be the same type,
but their value types must be the same.

```
std::vector<char> collection_1{'A'};
std::list<char> collection_2{'B', 'C'};

for (char value: iterators::Join(collection_1, collection_2))
    printf("%c, ", value);

// prints "A, B, C, "
```

If either of the collections has `const` values (e.g. `list<const A*>`),
the resulting iterator returns `const` values.

### Chain
Iterates over the values of a collection of collections,
effectively flattening out the input.

```
std::vector<list<char>> collection{
    {'A', 'B'},
    {},
    {'C'}
};

for (char value: iterators::Chain(collection))
    printf("%c, ", value);

// prints "A, B, C, "
```

### Zip

Iterator that aggregates elements from each of the collections.

Returns an iterator of element on which you can use `item.First()` and `item.Second()`.


```
std::vector<int> collection_1{'1', '11', '111'};
std::vector<char> collection_2{'A', 'B', 'C'};

for (auto & item: iterators::Zip(collection_1, collection_2))
    printf("[%i: %c], ", item.First(), item.Second());

// prints "[1: A], [11: B], [111: C], "
```

# Notes
* Most operators can be chained, so you can write `collection.filter(<lambda>).map(<lambda>).reverse().enumerate()`
* All operators (except `reverse`) can operate both on forward-only and bidirectional iterators.
* All operators are **logically const correct**, meaning that if your input collection is `const` (or contains `const` elements), the iterators will return `const` values.




<p xmlns:dct="http://purl.org/dc/terms/">
  <a rel="license"
     href="http://creativecommons.org/publicdomain/zero/1.0/">
    <img src="http://i.creativecommons.org/p/zero/1.0/88x31.png" style="border-style: none;" alt="CC0" />
  </a>
  <br />
  To the extent possible under law,
  <a rel="dct:publisher"
     href="https://github.com/jeroen-dhollander">
    <span property="dct:title">Jeroen Dhollander</span></a>
  has waived all copyright and related or neighboring rights to
  <span property="dct:title">Iterator-utilities</span>.
</p>
