# 夕月

## 基于 C++ 的基础功能库

**夕月(xiyue)** 是一个基于 C++ 编写的基础功能库，包含了一些自己实现的平时可能会经常用到的一些功能。

这个库目前包含以下功能：

* `ConstString` 引用字符串；
* `Json` 库。
* `Variant` 可变类型。

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

