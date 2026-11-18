# Code Style

This file describes the C++ and Ruby code styles in this repository.
If you are looking for instructions on how to customize and compile Gosu, see the [Hacking Gosu wiki page](https://github.com/gosu/gosu/wiki/Hacking-Gosu).

## Whitespace and Line Lengths

Do not use lines longer than 100 characters, except in Markdown files, where each line should be one sentence.  
**Rationale:** Shorter lines introduce distracting line breaks.
Longer lines require horizontal scrolling in editors (depending on screen size) and on GitHub.

## C++: Use clang-format with hints

Gosu provides a .clang-format file that yields acceptable results for most C++ code.
Sometimes, code will look a bit more consistent if you add a `//` at the end of a line to force a line break.

## C++: Do not insist on "uniform initialization"

The C++ Core Guidelines [recommend using `{}` for initialization almost everywhere](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es23-prefer-the--initializer-syntax). ([Practical example.](https://github.com/isocpp/CppCoreGuidelines/pull/1973/files))
This project follows more traditional rules:

* Use `{}` only to initialize containers and for designated initializers.
* Use `()` to call constructors.
* Use `=` to initialize values.

Examples: `const int x = 5;`, not `const int x{5};`. `const Point p { .x = 1, .y = 2 };`, not `const Point p{1, 2}`.

Rationale: Using braces for everything is more "consistent" in the same way that LISP consistently uses parentheses for everything.
That has not made it more legible.

Related reading: [The Knightmare of Initialization in C++](https://quuxplusone.github.io/blog/2019/02/18/knightmare-of-initialization/) by Arthur O'Dwyer.

## Ruby: Use rufo

* Use [the `rufo` gem](https://github.com/ruby-formatter/rufo) with the default settings. 
  [Visual Studio Code has a plugin](https://marketplace.visualstudio.com/items?itemName=mbessey.vscode-rufo) to format code on save.
* Use simple (dumb, boring) Ruby code where possible.  
  **Rationale:** Ruby/Gosu is often used by novices.
* Use `"` for strings, only use `'` when absolutely necessary.
  **Rationale:** Consistency.
* Do not replace all uses of `if not` by `unless`.
  There is a time and place for both.  
  **Rationale:** The English word "unless" is [used to describe exceptional conditions](https://dictionary.cambridge.org/us/dictionary/english/unless).  
  See also [this discussion on GitHub](https://github.com/bbatsov/ruby-style-guide/issues/329).
* Use `and`, `or` and `not` in conditions.
  **Example:** `if admin? and not undeletable?`  
  **Example:** `while object = stack.pop and object.valid?`  
  **Rationale:** Easy to read, consistency with other control flow operators.
* Use `&&`, `||` and `!` in Boolean algebra.  
  **Example:** `should_delete = is_admin && !is_undeletable`  
  **Example:** `puts(description || "no description")`  
  **Rationale:** Consistency with other binary operators (`|`/`&` and `||=`/`&&=`), no problems with operator precedence.
