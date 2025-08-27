# Metalang

## What is this?

After watching Sam H. Smith's talk at the Better Software Conference 2025,
I decided to attempt creating a compiler myself.

When you tell people nowadays that you want to create a new programming language,
most often they scoff at you and assume you have been living under a rock for the
past twenty years and haven't seen all of the "progress" in modern languages.

But most modern languages suck.

In high-performance scenarios, people are still using C or C++ to create serious
software. Why? Because nothing we have created since 1970 can beat their ability
to allow programmers to program their computers directly. That's insane! So I
decided to "give it a whack" so-to-speak and see where we end up.

The language that I am compiling is a new language I designed. The syntax is based
off of a mixture between C and Pascal, but who cares about syntax. The interesting
stuff is in my plans for metaprogramming (hence the name...) but that's foreshadowing.

If you want to learn more about sea of nodes, I suggest you check out the repositories
on GitHub. There are multiple implementations in different languages, so you can
follow along in whatever language you prefer. I will say that some of the code was a
bit tricky to figure out how to convert, since it's mostly written in OOP.

(Oh yeah, and I don't use an AST either.)

References:
- The talk at BSC: [https://youtu.be/NxiKlnUtyio](https://youtu.be/NxiKlnUtyio)
- The sea of nodes tutorial: [https://github.com/SeaOfNodes/Simple](https://github.com/SeaOfNodes/Simple)

## Quick Start

First, build the compiler:

```bat
cd compiler
build.bat
```

Then you should have an executable named `metalang_msvc_debug.exe` in the `build` directory.

Side note, the compiler is currently only compilable on Windows. Once I get around
to pulling out my Linux machine again, I may try to get it compiling there also.
