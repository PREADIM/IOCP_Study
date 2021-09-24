#pragma once

// dynamic_cast 같은것들로 cast를 할순있지만, 다이나믹 캐스트는 무거운 작업이기때문에 남발은 불가.
// 그렇기 때문에 서로 cast가 가능한 클래스인지 판단하기 위한 TypeCast 기법이다.
// 언리얼엔진 안에는 이런 TypeCast가 알아서 구현이 되어있다.
// 꼭 사용해야하는건 아니지만, 템플릿 공부에도 도움이되고 쓸만하다.


#include "Types.h"

#pragma region TypeList

template<typename... T>
struct TypeList; //형식상 구현해 두는것

template<typename T, typename U>
struct TypeList<T, U> //특수화
{
	using Head = T;
	using Tail = U;
};

template<typename T, typename... U>
struct TypeList<T, U...> //특수화
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
	enum { value = 1 + Length<TypeList<U...>>::value }; //재귀적인 특성으로 몇개있는지 계산하는 기법.
};

#pragma endregion


#pragma region TypeAt
template<typename TL, int32 index>
struct TypeAt;


template<typename Head, typename...Tail>
struct TypeAt<TypeList<Head, Tail...>, 0> // 0번째 인덱스를 가리키면 그냥 Head를 가리키는 것이다.
{
	using Result = Head;
};

template<typename Head, typename... Tail, int32 index>
struct TypeAt<TypeList<Head, Tail...>, index>
{
	using Result = typename TypeAt<TypeList<Tail...>, index - 1>::Result;
	// typename TypeAt 이렇게 붙여준이유는 using Result가 타입을 의미하는건지 값(멤버 변수)를 의미하는건지 알려주기 위함.
	// typename을 붙여주면 타입을 지정하는 것이다. 
	// 한마디로 typename TypeAt<TypeList<Tail...>, index - 1>::Result에서 Result의 타입을 using Result에 저장.
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
	enum { temp = IndexOf<TypeList<Tail...>, T>::value }; // T와 Head가 일지하는 곳이 찾아지면 temp값이 정해짐.

public:
	enum { value = (temp == -1) ? -1 : temp +1 }; // temp값 0이 리턴되면 차례차례 value값도 계산이되어 리턴된다.
	//그렇기 때문에 결국엔 몇번째 인덱스인지 알수있게된다.
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
		// 컴파일러가 알아서 최적의 Test 함수를 호출할테니 그것이 sizeof(Small)과 같은지 확인한다.
		// From이 To로 캐스팅이 가능하다면 Small Test(To)가 호출될것이고 아니면 Big이 호출될 것이다.
		// 왜냐하면 결국엔 sizeof(Test(MakeFrom()))는 sizeof(Test([From]))을 가리키기 때문. 
		// Small과 Big은 그저 가능하냐 안하냐의 헬퍼 함수일뿐이지 큰 의미는 없다. (bool형 true와 false를 쓰기 애매하기때문.)
	};
};


#pragma endregion



/* 
	★ 위의 작업들은 사실상 이 TypeCast를 위한 사전 작업이라고 볼수 있다.
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
		MakeTable(int2Type<i + 1>(), int2Type<0>()); // j를 초기화 시켜서 다시 i+1 한뒤에 다시 실행.
	}


	template<int32 j>
	static void MakeTable(int2Type<length>, int2Type<j>) // 끝과 끝까지 다 돈것이므로 할게없음.
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

	using TL = typename From::TL; // 해당하는 클래스안에 using TL이 있어야한다.

	if(TypeConversion<TL>::CanConvert(ptr->_typeid, IndexOf<TL, remove_pointer_t<To>>::value))
		return static_cast<To>(ptr);
	// 또한 해당하는 클래스 안에 _typeid가 있어야 동작한다.

	return nullptr;
}

// shared_ptr  버전

template<typename To, typename From>
std::shared_ptr<To> TypeCast(std::shared_ptr<From> ptr)
{
	if (ptr == nullptr)
		return nullptr;

	using TL = typename From::TL; // 해당하는 클래스안에 using TL이 있어야한다.

	if (TypeConversion<TL>::CanConvert(ptr->_typeid, IndexOf<TL, remove_pointer_t<To>>::value))
		return static_pointer_cast<To>(ptr);
	// 또한 해당하는 클래스 안에 _typeid가 있어야 동작한다.

	return nullptr;
}


template<typename To, typename From>
bool CanCast(From* ptr)
{
	if (ptr == nullptr)
		return false;

	using TL = typename From::TL; // 해당하는 클래스안에 using TL이 있어야한다.
	return TypeConversion<TL>::CanConvert(ptr->_typeid, IndexOf<TL, remove_pointer_t<To>>::value);
	// 또한 해당하는 클래스 안에 _typeid가 있어야 동작한다.
}


// shared_ptr 버전

template<typename To, typename From>
bool CanCast(std::shared_ptr<From> ptr)
{
	if (ptr == nullptr)
		return false;

	using TL = typename From::TL; // 해당하는 클래스안에 using TL이 있어야한다.
	return TypeConversion<TL>::CanConvert(ptr->_typeid, IndexOf < TL, remove_pointer_t<To>>::value);
		// 또한 해당하는 클래스 안에 _typeid가 있어야 동작한다.
}
#pragma endregion


// 타입 캐스트 매크로 

#define DECLARE_TL		using TL = TL; int32 _typeid;
#define INIT_TL(Type)	_typeid = IndexOf<TL, Type>::value; //해당 타입이 몇번째 인덱스인지.