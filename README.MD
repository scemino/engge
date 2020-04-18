# engge

[![Build Status](https://travis-ci.org/scemino/engge.svg?branch=master)](https://travis-ci.org/scemino/engge)
[![Build status](https://ci.appveyor.com/api/projects/status/i3u9klo4o762lacf?svg=true)](https://ci.appveyor.com/project/scemino/engge)
[![Twitter URL](https://img.shields.io/twitter/url?style=social&url=https%3A%2F%2Ftwitter.com%2Fengge_the_game)](https://twitter.com/engge_the_game)

engge is an **experimental** game engine, its purpose is to be able (one day) to run Thimbleweed park.

[![https://i.imgur.com/En75Mzx.png](https://i.imgur.com/En75Mzx.png)](https://www.youtube.com/watch?v=09VEPoX5SZk&t=1s)

## Prerequisites

* Buy [Thimbleweed park](https://thimbleweedpark.com)
* Go to your installation folder and copy these files:  **ThimbleweedPark.ggpack1** and **ThimbleweedPark.ggpack2** to **engge** directory
* **engge** has these following dependencies
  * [SFML](https://www.sfml-dev.org/): Simple and Fast Multimedia Library
  * [dear imgui](https://github.com/ocornut/imgui): Bloat-free Immediate Mode Graphical User interface for C++ with minimal dependencies
  * [imgui-sfml](https://github.com/eliasdaler/imgui-sfml): ImGui binding for use with SFML
  * [squirrel](http://www.squirrel-lang.org/): programming language Squirrel, this repository contains a modified version of the official squirrel library in *extlibs/squirrel*
  * [spdlog](https://github.com/gabime/spdlog): Fast C++ logging library.
  * [clipper](https://sourceforge.net/projects/polyclipping/): Polygon and line clipping and offsetting library.

## Compiling

```bash
mkdir build
cd build
cmake ..
cmake --build .
cd ..
```

## Running

Just type `./build/engge`, don't forget the prerequisites.

## Disclaimer

This project is experimental, you won't be able to run the full game

---

![CLion](https://github.com/JetBrains/logos/blob/master/web/clion/clion.svg)

[JetBrains](https://www.jetbrains.com/) have been kind enough to supply me with a free Open Source license of [CLion](https://www.jetbrains.com/clion).
