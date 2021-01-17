# engge

[![Build](https://github.com/scemino/engge/workflows/Build/badge.svg)](https://github.com/scemino/engge/actions)
[![CodeFactor](https://www.codefactor.io/repository/github/scemino/engge/badge)](https://www.codefactor.io/repository/github/scemino/engge)
[![Twitter URL](https://img.shields.io/twitter/url?style=social&url=https%3A%2F%2Ftwitter.com%2Fengge_the_game)](https://twitter.com/engge_the_game)

engge is an adventure game engine able to run Thimbleweed Park.

It's an open source remake of Thimbleweed Park's engine.

The game is playable. What does it mean? Yes, it means you can play the game and you should be able to finish it. No, it doesn't mean that you won't face any bug.
It's still a project in active development, and there are a lot of issues, don't hesitate to contribute or to fill out a [bug report](https://github.com/scemino/engge/issues/new/choose).


[![https://i.imgur.com/En75Mzx.png](https://i.imgur.com/En75Mzx.png)](https://www.youtube.com/watch?v=09VEPoX5SZk&t=1s)

## Prerequisites

* Buy [Thimbleweed park](https://thimbleweedpark.com)
* Go to your installation folder and copy these files:  **ThimbleweedPark.ggpack1** and **ThimbleweedPark.ggpack2** to **engge** directory
* **engge** has these following dependencies
  * [ngf](https://github.com/scemino/EnggeFramework/): Engge Framework is a C++ framework based on SDL.
  * [SFML](https://www.sfml-dev.org/): Simple and Fast Multimedia Library (SFML-Audio only)
  * [squirrel](http://www.squirrel-lang.org/): programming language Squirrel, this repository contains a modified version of the official squirrel library in *extlibs/squirrel*
  * [spdlog](https://github.com/gabime/spdlog): Fast C++ logging library.
  * [clipper](https://sourceforge.net/projects/polyclipping/): Polygon and line clipping and offsetting library.

## Building

```bash
mkdir build
cd build
cmake ..
cmake --build .
cd ..
```

## Running

Just type `./build/src/engge`, don't forget the prerequisites.

## Input mappings

| Keyboard shortcut            |      Description      |
|------------------------------|-----------------------|
| <kbd>space</kbd>             |  Pause game           |
| <kbd>ESC</kbd>               |  Skip cutscene        |
| <kbd>1</kbd>                 |  Select actor 1       |
| <kbd>2</kbd>                 |  Select actor 2       |
| <kbd>3</kbd>                 |  Select actor 3       |
| <kbd>4</kbd>                 |  Select actor 4       |
| <kbd>5</kbd>                 |  Select actor 5       |
| <kbd>6</kbd>                 |  Select actor 6       |
| <kbd>9</kbd>                 |  Previous actor       |
| <kbd>0</kbd>                 |  Next actor           |
| <kbd>.</kbd>                 |  Skip text            |
| <kbd>Tab</kbd>               |  Show hotspots        |
| <kbd>Ctrl</kbd> <kbd>O</kbd> |  Show options         |
| <kbd>Ctrl</kbd> <kbd>U</kbd> |  Toggle HUD           |

---

![CLion](https://github.com/JetBrains/logos/blob/master/web/clion/clion.svg)

[JetBrains](https://www.jetbrains.com/) have been kind enough to supply me with a free Open Source license of [CLion](https://www.jetbrains.com/clion).
