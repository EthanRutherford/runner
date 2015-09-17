#ifndef CALLABLE_H
#define CALLABLE_H
#include <tuple>
#include <utility>
#include <type_traits>

// definition of pointer to function
template <class R, class... Args>
using func_ptr = R (*)(Args...);

// variadic helper
// Indexer
template<int...> struct index_tuple{};
template<int I, class IndexTuple, class... Types>
struct make_indices_impl;
template<int I, int... Indices, class T, class... Types>
struct make_indices_impl<I,index_tuple<Indices...>,T,Types...> {
	typedef typename make_indices_impl<I+1,index_tuple<Indices...,I>,Types...>::type type;
};
template<int I, int... Indices>
struct make_indices_impl<I,index_tuple<Indices...>> {
	typedef index_tuple<Indices...> type;
};
template<class... Types>
struct make_indices : make_indices_impl<0, index_tuple<>, Types...> {};
// Unpacker/Forwarder
template<class R, class... Args, int... Indices>
R apply_helper(func_ptr<R,Args...> f, index_tuple<Indices...>, std::tuple<Args...>&& tup) {
	return f(std::forward<Args>(std::get<Indices>(tup))...);
}
template<class R, class... Args>
R apply(func_ptr<R,Args...> f, const std::tuple<Args...>& tup) {
    return apply_helper(f, typename make_indices<Args...>::type(), std::tuple<Args...>(tup));
}
template<class R, class... Args> 
R apply(func_ptr<R,Args...> f, std::tuple<Args...>&& tup) {
    return apply_helper(f, typename make_indices<Args...>::type(), 
		std::forward<std::tuple<Args...>>(tup));
}
// end of variadic helper

template<class R, class... Args> 
class callable{
	public:
		callable(func_ptr<R,Args...> f, Args... a) : func(f), args(a...) { }
		R call() { return apply(func, args); }
		R pass(Args... args) { return func(args...); }
	private:
		func_ptr<R,Args...> func;
		std::tuple<Args...> args;
};

#endif