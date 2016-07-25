# Code Style

See the [Hacking Gosu wiki page](https://github.com/gosu/gosu/wiki/Hacking-Gosu) for an actual guide on how to contribute to Gosu.
This file only details the code style that patches should follow.

## C++ and Objective-C

* Use `JavaClassNames`, `javaVariables.javaMethods()`, `AND_JAVA_CONSTANTS`.
  For C enums, follow the Delphi style: `enum AlphaMode { amDefault, amAdditive };`
  For C++11 enums, follow the Java style: `enum class AlphaMode { DEFAULT, ADDITIVE };`
* Don't worry about trailing spaces or Windows/UNIX newlines.
  But please ensure that your commit does not change lines or files that you haven't touched otherwise.
* Indent in multiples of four spaces (not tabs).
* Place `{` on its own line in C and C++, but on the same line *inside* Objective C methods.
  The opening brace `{` for an Objective-C method should be on its own line, though.
* Align the pointer star to the left in C and C++ (`char* str`), but to the right in Objective C (`NSString *string`).
* Place spaces only around infix operators: `((-4 + 6) * --a[76])`
* Do not use spaces after function names: `f(foo, bar)`
* Put a space after flow control keywords: `return 5`, `if (cond)`, `switch (x)`.

When in doubt, go with the flow and emulate the formatting of the code around your contribution.

## Markdown and Comments

Please start each sentence on a new line.
This avoids large diffs where a single word is added or removed, and whole paragraphs have to be re-aligned.

Changes to the public interface must be commented in a way that matches the current [Doxygen settings in Gosu](https://github.com/gosu/libgosu.org/blob/master/Doxyfile).

## Commit Messages

Try to follow [this guideline](http://chris.beams.io/posts/git-commit/) on formatting commit messages.
