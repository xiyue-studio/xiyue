# 夕月

## 基于 C++ 的基础功能库

**夕月(xiyue)** 是一个基于 C++ 编写的基础功能库，包含了一些自己实现的平时可能会经常用到的一些功能。

这个库目前包含以下功能：

* `ConstString` 引用字符串；
* `Json` 库。
* `Variant` 可变类型。
* `XyRe` 正则表达式引擎。

## Variant 可变类型

是一种高级的 `union` 结构，成员可以是任意类型。例如：

```cpp
Variant<int, double, bool, wstring, vector<int>> v;

static_assert(v.bufferSize == sizeof(wstring), "Variant size incorrect.");

EXPECT_TRUE(v.isNull());
v = 1;
EXPECT_TRUE(v.is<int>());
EXPECT_EQ(v.get<int>(), 1);
v = 3.14159;
EXPECT_TRUE(v.is<double>());
EXPECT_FALSE(v.is<int>());
EXPECT_NEAR(v.get<double>(), 3.14159, 0.00001);
v = false;
EXPECT_TRUE(v.is<bool>());
EXPECT_EQ(v.get<bool>(), false);
v = wstring(L"Hello world!");
EXPECT_TRUE(v.is<wstring>());
EXPECT_EQ(v.get<wstring>(), L"Hello world!");

v = vector<int>({ 1, 2, 3 });
EXPECT_TRUE(v.is<vector<int>>());
EXPECT_EQ(v.get<vector<int>>().size(), 3u);

EXPECT_THROW(v.get<int>(), std::bad_cast);
double val = 0.0;
EXPECT_THROW(val = v, std::bad_cast);
```

## `XyRe` 正则表达式引擎

是一个基于 DFA(虚拟机形式) 和 NFA 混合的正则表达式引擎，支持以下正则表达式语法：

转义字符：

* \f，换页符，等价于 \x0c
* \n，换行符，等价于 \x0a
* \r，回车符，等价于 \x0d
* \s，空白字符，等价于 [ \f\n\r\t\v]
* \S，非空白字符，等价于 [^ \f\n\r\t\v]
* \t，制表符，等价于 \x09
* \v，垂直制表符，等价于 \x0b
* \b，匹配单词边界
* \B，匹配非单词边界
* \d，匹配一个数字，等价于 [0-9]
* \D，匹配一个非数字，等价于 [^0-9]
* \w，匹配一个单词字符，等价于 [a-zA-Z0-9_]
* \W，匹配一个非单词字符，等价于 [^a-zA-Z0-9_]
* \G，全局匹配中有效，必须匹配上一次匹配结束的位置
* \xXX，转义 ASCII 字符
* \uXXXX，转义 unicode 字符
* \0XXX，转义八进制 ASCII 字符
* \0，匹配 NUL
* \1 ~ \9，反向引用分组
* \X，代表 X 字符本身

元字符：

* ^，匹配行首
* $，匹配行尾
* .，匹配任意字符
* [xxx]，字符类
* [^xxx]，反向字符类

量词：

* *，匹配0次到多次
* +，匹配1次到多次
* ?，匹配0次或1次
* {n}，匹配固定 n 次
* {n,}，匹配 n 次到多次
* {n,m}，匹配 n 次到 m 次
* *?，非贪婪匹配
* +?
* ??
* {n}?，无效，等价于 {n}
* {n,}?
* {n,m}?
* *+，占有优先
* ++
* ?+
* {n}+
* {n,}+
* {n,m}+

* 捕获组，(xxx)
* 固化分组，(?>xxx)
* 非捕获组，(?:xxx)
* Look Ahead 断言，(?=xxx) 和 (?!xxx)
* Look Behind 断言，(?<=xxx) 和 (?<!xxx)
* 命名捕获组，(?<name>xxx)
* 命名反向引用，\k<name>
* 匹配模式开关，(?imx) ... (?-imx)，i 为忽略大小写，m 为多行匹配(影响 ^ 和 $)，x 为松散模式
* 内部模式开关，(?imx:xxx)
* 注释，(?#xxxx)
* (暂不实现)Unicode 属性，\p{Script_Extensions=Greek}
