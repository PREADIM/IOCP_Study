#pragma once

#include "Types.h"
#include "Allocator.h"
#include <vector>
#include <list>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
using namespace std;

template<typename T, uint32 Size>
using Array = array<T, Size>;


template<typename T>
using Vector = vector<T, STLAllocator<T>>;

template<typename T>
using List = list<T, STLAllocator<T>>;


template<typename Key, typename Value, typename Pred = less<Key>>
using Map = map <Key, Value, Pred, STLAllocator<pair<const Key, Value>>>;


template<typename Key, typename Pred = less<Key>>
using Set = set<Key, Pred, STLAllocator<Key>>;

template<typename T>
using Deque = deque<T, STLAllocator<T>>;

template<typename T, typename Container = Deque<T>>
using Queue = queue<T, Container>;

template<typename T, typename Container = Deque<T>>
using Stack = stack<T, Container>;


using String = basic_string<char, char_traits<char>, STLAllocator<char>>;
using WString = basic_string<char, char_traits<char>, STLAllocator<char>>;

