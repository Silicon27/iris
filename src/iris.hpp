// iris - instantiation, resolution and (i)xpansion system
/*
 * copyright (c) 2026 Silicon27
 *
 * A C++20 compile time support library with each element designed to aid compile time resolution
 */
#ifndef IRIS_HPP
#define IRIS_HPP

#include <type_traits>
#include <tuple>

namespace iris {
  // get pack at some index
  template <std::size_t I, auto... Vs>
  consteval decltype(auto) pack_at() {
    constexpr auto tup = std::tuple{Vs...};
    return std::get<I>(tup);
  }
}

namespace iris::support {
  // check for homogeneity in template packs
  template <typename...> struct all_same : std::true_type {};
  template <typename T, typename... Ts> struct all_same<T, Ts...>
  : std::bool_constant<(... && std::is_same_v<std::remove_cvref_t <T>, std::remove_cvref_t<Ts>>)> {};

  // create applicable alias for homogenous packs
  template <typename... Ts> concept homogeneous_pack_v = all_same<Ts...>::value;

  template <auto... Vs>
  struct homogenous_value_type_pack_helper {
    static constexpr bool value = (all_same<decltype(Vs)...>::value);
  };

  template <auto... Vs> concept homogenous_value_type_pack = homogenous_value_type_pack_helper<Vs...>::value;

  // helper to get the type of a homogeneous value type pack
  template <auto... Vs>
  requires homogenous_value_type_pack<Vs...>
  using homogenous_value_type_pack_t = decltype(pack_at<0, Vs...>);
}

namespace iris {
  template <std::size_t I, typename T>
  struct bundle_leaf {
    T value;
  };


  template <std::size_t I, typename... Ts>
  struct bundle_impl;

  template <std::size_t I>
  struct bundle_impl<I> {};

  template <std::size_t I, typename T, typename... Ts>
  struct bundle_impl<I, T, Ts...>
                    : bundle_leaf<I, T>,
                      bundle_impl<I+1, Ts...>
  {};


  // a bundle denotes a non-type template parameter pack. It is obligatory that a pack is converted into
  // a bundle before iris may apply its operations on said pack
  template <typename... Ts>
  struct bundle : bundle_leaf<0, Ts...> { };

  template <std::size_t I, typename T>
  T& get_leaf(bundle_leaf<I, T>& leaf) {
    return leaf.value;
  }

  template <std::size_t I, typename... Ts>
  auto& get(bundle<Ts...>& b) {
    return get_leaf<I>(b);
  }
}

namespace iris::pgen {
  // iris::pgen - iris parser generator 

  template <char... Cs>
    consteval auto has_trailing_null() {
        if constexpr (sizeof...(Cs) == 0) {
            return false;
        } else {
            return pack_at<sizeof...(Cs) - 1, Cs...>() == '\0';
        }
    }

    template <char... Cs>
    struct string {
        static constexpr char value[sizeof...(Cs)] = {Cs...};
        static consteval size_t max_idx() {return sizeof...(Cs) - 1;}
        static consteval size_t length() {return sizeof...(Cs);}
        static consteval const char* begin() {return value;}
        static consteval const char* end() {return value + sizeof...(Cs);}
    };

    template <char...>
    struct remove_trailing_null_recursive;

    template <>
    struct remove_trailing_null_recursive<> {
        using type = string<>;
    };

    template <char Last>
    struct remove_trailing_null_recursive<Last> {
        using type = std::conditional_t<Last == '\0', string<>, string<Last>>;
    };

    template <char First, char... Rest>
    struct remove_trailing_null_recursive<First, Rest...> {
    private:
        using tail = typename remove_trailing_null_recursive<Rest...>::type;
    public:
        using type = decltype([]<char... Cs2>(string<Cs2...>) -> string<First, Cs2...> { return {}; }(tail{}));
    };

    template <char... Cs>
    consteval auto remove_trailing_null() {
        if constexpr (sizeof...(Cs) == 0) {
            return string<>{};
        } else {
            constexpr char arr[] = {Cs...};
            if constexpr (arr[sizeof...(Cs) - 1] == '\0') {
                return []<std::size_t... Is>(std::index_sequence<Is...>) {
                    return string<arr[Is]...>{};
                }(std::make_index_sequence<sizeof...(Cs) - 1>{});
            } else {
                return string<Cs...>{};
            }
        }
    }

    template <char... Cs>
    using remove_trailing_null_t = decltype(remove_trailing_null<Cs...>());

    template <typename Str1, typename Str2>
    struct append;

    template <char... Cs1, char... Cs2>
    struct append<string<Cs1...>, string<Cs2...>> {
        using str1 = remove_trailing_null_t<Cs1...>;
        using str2 = remove_trailing_null_t<Cs2...>;

        using type = decltype(
            []<char... C1, char... C2>(string<C1...>, string<C2...>) -> string<C1..., C2...>
            {return {};} (str1{}, str2{})
        );
    };

    template <typename Str1, typename Str2>
    using append_t = typename append<Str1, Str2>::type;

    template <typename Str1, typename Str2>
    struct append_as_literal;

    template <char... Cs1, char... Cs2>
    struct append_as_literal<string<Cs1...>, string<Cs2...>> {
        using str1 = remove_trailing_null_t<Cs1...>;
        using str2 = remove_trailing_null_t<Cs2...>;

        using type = decltype([]<char... C1, char... C2>(string<C1...>, string<C2...>) -> append_t<string<C1..., C2...>, string<'\0'>>
            {return {};} (str1{}, str2{}));
    };

    template <auto& Str, std::size_t N, std::size_t... Is>
    consteval auto make_string_literal_helper(std::index_sequence<Is...>) {
        if constexpr (has_trailing_null<Str[Is]...>()) {
            return string<Str[Is]...>{};
        } else  {
            return string<Str[Is]..., '\0'>{};
        }
    }

    template <auto& Str, std::size_t N>
    struct make_string_literal {
        using type = decltype(make_string_literal_helper<Str, N>(std::make_index_sequence<N>{}));
    };

    template <auto& Str, std::size_t N>
    using make_string_literal_t = typename make_string_literal<Str, N>::type;

}


#endif // IRIS_HPP