# Ϧ��

## ���� C++ �Ļ������ܿ�

**Ϧ��(xiyue)** ��һ������ C++ ��д�Ļ������ܿ⣬������һЩ�Լ�ʵ�ֵ�ƽʱ���ܻᾭ���õ���һЩ���ܡ�

�����Ŀǰ�������¹��ܣ�

* `ConstString` �����ַ�����
* `Json` �⡣
* `Variant` �ɱ����͡�
* `XyRe` ������ʽ���档

## Variant �ɱ�����

��һ�ָ߼��� `union` �ṹ����Ա�������������͡����磺

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

## `XyRe` ������ʽ����

��һ������ DFA(�������ʽ) �� NFA ��ϵ�������ʽ���棬֧������������ʽ�﷨��

ת���ַ���

* \f����ҳ�����ȼ��� \x0c
* \n�����з����ȼ��� \x0a
* \r���س������ȼ��� \x0d
* \s���հ��ַ����ȼ��� [ \f\n\r\t\v]
* \S���ǿհ��ַ����ȼ��� [^ \f\n\r\t\v]
* \t���Ʊ�����ȼ��� \x09
* \v����ֱ�Ʊ�����ȼ��� \x0b
* \b��ƥ�䵥�ʱ߽�
* \B��ƥ��ǵ��ʱ߽�
* \d��ƥ��һ�����֣��ȼ��� [0-9]
* \D��ƥ��һ�������֣��ȼ��� [^0-9]
* \w��ƥ��һ�������ַ����ȼ��� [a-zA-Z0-9_]
* \W��ƥ��һ���ǵ����ַ����ȼ��� [^a-zA-Z0-9_]
* \G��ȫ��ƥ������Ч������ƥ����һ��ƥ�������λ��
* \xXX��ת�� ASCII �ַ�
* \uXXXX��ת�� unicode �ַ�
* \0XXX��ת��˽��� ASCII �ַ�
* \0��ƥ�� NUL
* \1 ~ \9���������÷���
* \X������ X �ַ�����

Ԫ�ַ���

* ^��ƥ������
* $��ƥ����β
* .��ƥ�������ַ�
* [xxx]���ַ���
* [^xxx]�������ַ���

���ʣ�

* *��ƥ��0�ε����
* +��ƥ��1�ε����
* ?��ƥ��0�λ�1��
* {n}��ƥ��̶� n ��
* {n,}��ƥ�� n �ε����
* {n,m}��ƥ�� n �ε� m ��
* *?����̰��ƥ��
* +?
* ??
* {n}?����Ч���ȼ��� {n}
* {n,}?
* {n,m}?
* *+��ռ������
* ++
* ?+
* {n}+
* {n,}+
* {n,m}+

* �����飬(xxx)
* �̻����飬(?>xxx)
* �ǲ����飬(?:xxx)
* Look Ahead ���ԣ�(?=xxx) �� (?!xxx)
* Look Behind ���ԣ�(?<=xxx) �� (?<!xxx)
* ���������飬(?<name>xxx)
* �����������ã�\k<name>
* ƥ��ģʽ���أ�(?imx) ... (?-imx)��i Ϊ���Դ�Сд��m Ϊ����ƥ��(Ӱ�� ^ �� $)��x Ϊ��ɢģʽ
* �ڲ�ģʽ���أ�(?imx:xxx)
* ע�ͣ�(?#xxxx)
* (�ݲ�ʵ��)Unicode ���ԣ�\p{Script_Extensions=Greek}
