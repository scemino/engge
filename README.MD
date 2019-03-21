# engge

engge is an **experimental** game engine, its purpose is to be able (one day) to run Thimbleweed park.

[![https://i.imgur.com/En75Mzx.png](https://i.imgur.com/En75Mzx.png)](https://www.youtube.com/watch?v=09VEPoX5SZk&t=1s)

# Prerequisites

* Buy [Thimbleweed park](https://thimbleweedpark.com)
* Go to your installation folder and copy these files:  **ThimbleweedPark.ggpack1** and **ThimbleweedPark.ggpack2** to **engge** directory
* **engge** has these following dependencies
    * [SFML library](https://www.sfml-dev.org/)
    * [nlohmann/json library](https://github.com/nlohmann/json): the headers are located under *include/nlohmann* directory
    * [squirrel](http://www.squirrel-lang.org/): this repository contains a modified version of the official squirrel library in *extlibs/squirrel*

# Compiling

```
mkdir build
cd build
cmake ..
cmake --build .
cd ..
```

# Running

Just type `./build/engge`, don't forget the prerequisites.

# Disclaimer

This project is experimental, you won't be able to run the full game
