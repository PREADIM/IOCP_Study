#pragma once

// dynamic_cast �����͵�� cast�� �Ҽ�������, ���̳��� ĳ��Ʈ�� ���ſ� �۾��̱⶧���� ������ �Ұ�.
// �׷��� ������ ���� cast�� ������ Ŭ�������� �Ǵ��ϱ� ���� TypeCast ����̴�.
// �𸮾��� �ȿ��� �̷� TypeCast�� �˾Ƽ� ������ �Ǿ��ִ�.
// �� ����ؾ��ϴ°� �ƴ�����, ���ø� ���ο��� �����̵ǰ� �����ϴ�.


#include "Types.h"

#pragma region TypeList

template<typename... T>
struct TypeList; //���Ļ� ������ �δ°�

template<typename T, typename U>
struct TypeList<T, U> //Ư��ȭ
{
	using Head = T;
	using Tail = U;
};

template<typename T, typename... U>
struct TypeList<T, U...> //Ư��ȭ
{
	using Head = T;
	using Tail = TypeList<U...>;
};

#pragma endregion



#pragma region Length

template<typename T>
struct Length;

template<>
struct Length<TypeList<>>
{
	enum { value = 0 };
};


template<typename T, typename...U>
struct Length < TypeList<T, U...>>
{
	enum { value = 1 + Length<TypeList<U...>>::value }; //������� Ư������ ��ִ��� ����ϴ� ���.
};

#pragma endregion


#pragma region TypeAt
template<typename TL, int32 index>
struct TypeAt;


template<typename Head, typename...Tail>
struct TypeAt<TypeList<Head, Tail...>, 0> // 0��° �ε����� ����Ű�� �׳� Head�� ����Ű�� ���̴�.
{
	using Result = Head;
};

template<typename Head, typename... Tail, int32 index>
struct TypeAt<TypeList<Head, Tail...>, index>
{
	using Result = typename TypeAt<TypeList<Tail...>, index - 1>::Result;
	// typename TypeAt �̷��� �ٿ��������� using Result�� Ÿ���� �ǹ��ϴ°��� ��(��� ����)�� �ǹ��ϴ°��� �˷��ֱ� ����.
	// typename�� �ٿ��ָ� Ÿ���� �����ϴ� ���̴�. 
	// �Ѹ���� typename TypeAt<TypeList<Tail...>, index - 1>::Result���� Result�� Ÿ���� using Result�� ����.
};
#pragma endregion


#pragma region IndexOf
template<typename TL, typename T>
struct IndexOf;

template<typename Head, typename...Tail>
struct IndexOf<TypeList<Head, Tail...>, Head>
{
	enum { value = 0 };
};

template<typename T>
struct IndexOf<TypeList<>, T>
{
	enum { value = -1 };
};


template<typename Head, typename...Tail, typename T>
struct IndexOf<TypeList<Head, Tail...>, T>
{
private:
	enum { temp = IndexOf<TypeList<Tail...>, T>::value }; // T�� Head�� �����ϴ� ���� ã������ temp���� ������.

public:
	enum { value = (temp == -1) ? -1 : temp +1 }; // temp�� 0�� ���ϵǸ� �������� value���� ����̵Ǿ� ���ϵȴ�.
	//�׷��� ������ �ᱹ�� ���° �ε������� �˼��ְԵȴ�.
};


#pragma endregion



#pragma region Conversion
template<typename From, typename To>
class Conversion
{
private :
	using Small = __int8;
	using Big = __int32;

	static Small Test(const To&) { return 0; }
	static Big Test(...) { return 0; }
	static From MakeFrom() { return 0; }

public:
	enum
	{
		exists = sizeof(Test(MakeFrom())) == sizeof(Small)
		// �����Ϸ��� �˾Ƽ� ������ Test �Լ��� ȣ�����״� �װ��� sizeof(Small)�� ������ Ȯ���Ѵ�.
		// From�� To�� ĳ������ �����ϴٸ� Small Test(To)�� ȣ��ɰ��̰� �ƴϸ� Big�� ȣ��� ���̴�.
		// �ֳ��ϸ� �ᱹ�� sizeof(Test(MakeFrom()))�� sizeof(Test([From]))�� ����Ű�� ����. 
		// Small�� Big�� ���� �����ϳ� ���ϳ��� ���� �Լ��ϻ����� ū �ǹ̴� ����. (bool�� true�� false�� ���� �ָ��ϱ⶧��.)
	};
};


#pragma endregion



/* 
	�� ���� �۾����� ��ǻ� �� TypeCast�� ���� ���� �۾��̶�� ���� �ִ�.
*/

#pragma region TypeCast

template<int32 N>
struct int2Type
{
	enum { value = N };
};


template<typename TL>
class TypeConversion
{
public:
	enum
	{
		length = Length<TL>::value
	};

	TypeConversion()
	{
		MakeTable(int2Type<0>(), int2Type<0>());
	}

	template<int32 i, int32 j>
	static void MakeTable(int2Type<i>, int2Type<j>)
	{
		using FromType = typename TypeAt<TL, i>::Result;
		using ToType = typename TypeAt<TL, j>::Result;

		if (Conversion<const FromType*, const ToType*>::exists)
			s_convert[i][j] = true;
		else
			s_convert[i][j] = false;

		MakeTable(int2Type<i>(), MakeTable<j + 1>());
	}


	template<int32 i>
	static void MakeTable(int2Type<i>, int2Type<length>)
	{
		MakeTable(int2Type<i + 1>(), int2Type<0>()); // j�� �ʱ�ȭ ���Ѽ� �ٽ� i+1 �ѵڿ� �ٽ� ����.
	}


	template<int32 j>
	static void MakeTable(int2Type<length>, int2Type<j>) // ���� ������ �� �����̹Ƿ� �ҰԾ���.
	{
	}


	static inline bool CanConvert(int32 from, int32 to)
	{
		static TypeConversion Conversion;
		return s_convert[from][to];
	}


public:
	static bool s_convert[length][length];
};

template<typename TL>
bool TypeConversion<TL>::s_convert[length][length];



template<typename To, typename From>
To TypeCast(From* ptr)
{
	if (ptr == nullptr)
		return nullptr;

	using TL = typename From::TL; // �ش��ϴ� Ŭ�����ȿ� using TL�� �־���Ѵ�.

	if(TypeConversion<TL>::CanConvert(ptr->_typeid, IndexOf<TL, remove_pointer_t<To>>::value))
		return static_cast<To>(ptr);
	// ���� �ش��ϴ� Ŭ���� �ȿ� _typeid�� �־�� �����Ѵ�.

	return nullptr;
}

// shared_ptr  ����

template<typename To, typename From>
std::shared_ptr<To> TypeCast(std::shared_ptr<From> ptr)
{
	if (ptr == nullptr)
		return nullptr;

	using TL = typename From::TL; // �ش��ϴ� Ŭ�����ȿ� using TL�� �־���Ѵ�.

	if (TypeConversion<TL>::CanConvert(ptr->_typeid, IndexOf<TL, remove_pointer_t<To>>::value))
		return static_pointer_cast<To>(ptr);
	// ���� �ش��ϴ� Ŭ���� �ȿ� _typeid�� �־�� �����Ѵ�.

	return nullptr;
}


template<typename To, typename From>
bool CanCast(From* ptr)
{
	if (ptr == nullptr)
		return false;

	using TL = typename From::TL; // �ش��ϴ� Ŭ�����ȿ� using TL�� �־���Ѵ�.
	return TypeConversion<TL>::CanConvert(ptr->_typeid, IndexOf<TL, remove_pointer_t<To>>::value);
	// ���� �ش��ϴ� Ŭ���� �ȿ� _typeid�� �־�� �����Ѵ�.
}


// shared_ptr ����

template<typename To, typename From>
bool CanCast(std::shared_ptr<From> ptr)
{
	if (ptr == nullptr)
		return false;

	using TL = typename From::TL; // �ش��ϴ� Ŭ�����ȿ� using TL�� �־���Ѵ�.
	return TypeConversion<TL>::CanConvert(ptr->_typeid, IndexOf < TL, remove_pointer_t<To>>::value);
		// ���� �ش��ϴ� Ŭ���� �ȿ� _typeid�� �־�� �����Ѵ�.
}
#pragma endregion


// Ÿ�� ĳ��Ʈ ��ũ�� 

#define DECLARE_TL		using TL = TL; int32 _typeid;
#define INIT_TL(Type)	_typeid = IndexOf<TL, Type>::value; //�ش� Ÿ���� ���° �ε�������.