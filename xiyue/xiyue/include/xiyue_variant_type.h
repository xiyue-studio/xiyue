#include <typeindex>
#include <type_traits>

namespace xiyue
{
	template <size_t... args>
	struct MaxSize;

	template<size_t n>
	struct MaxSize<n> : std::integral_constant<size_t, n> {};

	template<size_t l, size_t r, size_t... args>
	struct MaxSize<l, r, args...> : std::integral_constant<size_t,
		l >= r ? MaxSize<l, args...>::value : MaxSize<r, args...>::value> {};

	template<typename... Args>
	struct MaxAlignSize : std::integral_constant<size_t, MaxSize<std::alignment_of<Args>::value...>::value> {};

	template<typename TestType, typename... Types>
	struct TypeContained;

	template<typename TestType, typename T, typename... Rest>
	struct TypeContained<TestType, T, Rest...> : std::conditional<
		std::is_same<TestType, T>::value, std::true_type, TypeContained<TestType, Rest...>>::type
	{};

	template<typename TestType, typename T>
	struct TypeContained<TestType, T> : std::conditional<
		std::is_same<TestType, T>::value, std::true_type, std::false_type>::type
	{};

	template<typename... Args>
	struct VariantDataHelper;

	template<typename T, typename... Args>
	struct VariantDataHelper<T, Args...>
	{
		inline static void destroy(std::type_index index, void* data)
		{
			if (index == std::type_index(typeid(T)))
			{
				reinterpret_cast<T*>(data)->~T();
			}
			else
			{
				VariantDataHelper<Args...>::destroy(index, data);
			}
		}

		inline static void copy(std::type_index index, const void* src, void* dst)
		{
			if (index == std::type_index(typeid(T)))
			{
				new(dst)T(*reinterpret_cast<const T*>(src));
			}
			else
			{
				VariantDataHelper<Args...>::copy(index, src, dst);
			}
		}

		inline static void move(std::type_index index, void* src, void* dst)
		{
			if (index == std::type_index(typeid(T)))
			{
				new(dst)T(std::move(*reinterpret_cast<T*>(src)));
			}
			else
			{
				VariantDataHelper<Args...>::move(index, src, dst);
			}
		}
	};

	template<>
	struct VariantDataHelper<> {
		inline static void destroy(std::type_index, void*) {}
		inline static void copy(std::type_index, const void*, void*) {}
		inline static void move(std::type_index, void*, void*) {}
	};

	template<typename... Args>
	class Variant
	{
	public:
		enum {
			bufferSize = MaxSize<sizeof(Args)...>::value,
			alignSize = MaxAlignSize<Args...>::value
		};

	private:
		using DataType = typename std::aligned_storage<bufferSize, alignSize>::type;

		using VariantDataHelperType = VariantDataHelper<Args...>;

	public:
		template <typename T,
			typename = typename std::enable_if<TypeContained<typename std::remove_reference<T>::type, Args...>::value>::type>
		Variant(T&& value) : m_typeIndex(typeid(void))
		{
			using InitType = typename std::remove_reference<T>::type;
			new(&m_data)InitType(std::forward<T>(value));
			m_typeIndex = std::type_index(typeid(T));
		}

		Variant(void) : m_typeIndex(typeid(void))
		{
		}

		Variant(Variant<Args...>&& r) : m_typeIndex(r.m_typeIndex)
		{
			VariantDataHelperType::move(r.m_typeIndex, &r.m_data, &m_data);
		}

		Variant(const Variant<Args...>& r) : m_typeIndex(r.m_typeIndex)
		{
			VariantDataHelperType::copy(r.m_typeIndex, &r.m_data, &m_data);
		}

		~Variant()
		{
			VariantDataHelperType::destroy(m_typeIndex, &m_data);
		}

	public:
		Variant& operator=(const Variant& r)
		{
			if (&r != this)
			{
				VariantDataHelperType::destroy(m_typeIndex, &m_data);
				VariantDataHelperType::copy(r.m_typeIndex, &r.m_data, &m_data);
				m_typeIndex = r.m_typeIndex;
			}

			return *this;
		}

		Variant& operator=(Variant&& r)
		{
			VariantDataHelperType::destroy(m_typeIndex, &m_data);
			VariantDataHelperType::move(r.m_typeIndex, &r.m_data, &m_data);
			m_typeIndex = r.m_typeIndex;
			return *this;
		}

		template <typename T,
			typename = typename std::enable_if<TypeContained<T, Args...>::value>::type>
		Variant& operator=(const T& r)
		{
			using Type = typename std::decay<T>::type;
			VariantDataHelperType::destroy(m_typeIndex, &m_data);
			new(&m_data)Type(r);
			m_typeIndex = std::type_index(typeid(T));

			return *this;
		}

		template <typename T,
			typename = typename std::enable_if<TypeContained<T, Args...>::value>::type>
		operator T&()
		{
			return get<T>();
		}

		template <typename T,
			typename = typename std::enable_if<TypeContained<T, Args...>::value>::type>
			operator const T&() const
		{
			return get<T>();
		}

	public:
		template <typename T>
		inline bool is() const
		{
			return m_typeIndex == std::type_index(typeid(T));
		}

		inline bool isNull() const
		{
			return m_typeIndex == std::type_index(typeid(void));
		}

		inline std::type_index getType() const
		{
			return m_typeIndex;
		}

		template <typename T>
		inline typename std::decay<T>::type& get()
		{
			using Type = typename std::decay<T>::type;
			if (!is<Type>())
				throw std::bad_cast();
			return *reinterpret_cast<Type*>(&m_data);
		}

	private:
		DataType m_data;
		std::type_index m_typeIndex;
	};
}
