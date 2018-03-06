# cpp-iterators

With the introduction of range-based for loops, it now makes more sense than ever to introduce iterators for all your classes.

However there are a lot of limitations to the range-based for loops:
    - You can only iterate forward (and not in reverse)
        - Each class can only have a single iterator defined (you can not define 2 iterators for the same class, e.g. for children and attributes).
	    - You can not manipulate the elements while iterating (like mapping, filtering) without manually having to write a specialized iterator.
	        - Similarly, iterators often leak implementation details (like whether you stored the data as unique_ptr or not).

		This library hopes to solve many if not all of these issues,
		by providing a set of flexible and reusable iterator classes that allow you to do all the above and more.

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
