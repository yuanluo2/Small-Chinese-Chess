# Small-Chinese-Chess

##### 这是一个完全用C语言编写的中国象棋游戏，它跑在命令行下, 支持AI对战(AI基于min-max算法, 配合alpha-beta剪枝优化). 在这个项目之前, 我用C++14编写了这个游戏, 但是那个版本在我的树莓派板子上编译很慢, 另外一件离谱的事情是, 我能用的开发机器gcc版本太低, 但是我不能升级这些机器的工具链, 所以它们没法支持C++11/14，然后我就用C重写了这个游戏.

##### 游戏本身很简单, 上面永远是AI一方, 下面就是我们玩家一方. 输入 help 就能查看帮助页面, 就像下面的截图一样, 然后你就可以玩了. 网络对战功能我暂时没考虑, 可能以后我会支持那个功能, 谁知道呢 ? 

##### ================================================================================================================================================================================================================

##### Chinese chess game written in pure C language, running under the console. AI uses min-max, alpha-beta pruning algorithm. Before this project, I write this game with C++14, but that version costs a lot of compile time on my own raspberrypi device, and a very bad thing is, for some reason, most machines I can use only support a very low version gcc, but I can't update the toolchains! so C++11/14 is not supported, so I think it's time to rewrite this game.

##### game is very simple, the upper is always AI, down is you. enter 'help' would give you a help page, as you can see below, and just enjoy it. network battle is not considered, maybe I would support that in the future, who knows ?

![image](https://github.com/yuanluo2/Small-Chinese-Chess/assets/49439486/c827e195-acf9-42bd-87ca-dd30d0b4a749)
