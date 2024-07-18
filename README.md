# RPG Maker MV/MZ 游戏翻译

基于我对一部分 RPG Maker MV/MZ 游戏的观察，我做出了这个简单的程序来将其翻译为中文。

# 缺陷

- 部分游戏做了很多修改，仅仅翻译/data下的json文件会使游戏无法正常运行甚至中断，需要更多的工作来适配此种游戏，而本项目暂不支持自动帮你完成。
- 解决办法(可能):不要翻译system.json,将其备份并于程序处理完成后替换。

# 本项目适配的源有：

- 百度翻译API
- translate-shell(repo : https://github.com/soimort/translate-shell)


# 使用方法

克隆本项目，进入目录

<code>
mkdir build

cd build

cmake ..

make
</code>

二进制文件会输出到build目录下,移动到上一级目录。

填写必要的配置项到tranconf.json

执行，输入游戏/data目录路径即可开始翻译。