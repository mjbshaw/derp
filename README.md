libderp++
---------

[Parsing with derivatives][1] is an interesting, relatively new algorithm for parsing. libderp++ is a C++ implementation of a <strong>der</strong>ivative-based <strong>p</strong>arser. It is primarily a research project investigating optimizations for derivatives for practical (real-world) languages and usages.

Usage
-----

libderp++ is a header-only library, and as such is meant to be very easy to use and experiment with.

Currently, a C++ compiler supporting C++11 is needed. There is nothing that inherently restricts libderp++ from working with a C++03 compiler, and a port to a C++03 compiler would be rather simple, but for the ease of development and ongoing research, libderp++ targets C++11.

Why?
----

Why not[‽][2] [Might][3] et al. have proven that derivatives can be used for parsing any context-free language. The original implementation worked for fairly small languages and inputs, but suffered from poor performance (both in memory and computing time) for larger languages and inputs.

This project is part of [Michael Bradshaw][4]'s bachelor's thesis on practical parsing with derivatives, and is an effort to show that derivatives aren't relegated to being just a beautiful theory with little practical application. libderp++ aims to be both easy to use and  efficient. libderp++ can be used to compute all parses of an ambiguous grammar, but since only one parse tree is typically desired (in real-world parsing), libderp++ is optimized for unambiguous grammars.

License
-------

See the LICENSE file for information regarding the license under which libderp++ is released.


  [1]: http://matt.might.net/papers/might2011derivatives.pdf
  [2]: http://en.wikipedia.org/wiki/Interrobang
  [3]: http://matt.might.net/
  [4]: http://mjb.io
