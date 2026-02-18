// iris - instantiation, resolution and (i)xpansion system
/*
 * copyright (c) 2026 Silicon27
 *
 * A C++20 compile time support library with each element designed to aid compile time resolution
 */

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

namespace iris {
  // section made explicitly for template packs
  // pack recursion, pack conversion, pack op, etc

}
