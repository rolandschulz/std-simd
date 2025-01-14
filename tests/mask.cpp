/*{{{
Copyright © 2009-2019 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
                      Matthias Kretz <m.kretz@gsi.de>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the names of contributing organizations nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

}}}*/

//#define UNITTEST_ONLY_XTEST 1
#include "unittest.h"
#include "metahelpers.h"

template <class... Ts> using base_template = std::experimental::simd_mask<Ts...>;
#include "testtypes.h"

// simd_mask generator functions {{{1
template <class M> M make_mask(const std::initializer_list<bool> &init)
{
    std::size_t i = 0;
    M r = {};
    for (;;) {
        for (bool x : init) {
            r[i] = x;
            if (++i == M::size()) {
                return r;
            }
        }
    }
}

template <class M> M make_alternating_mask()
{
    return make_mask<M>({false, true});
}

TEST_TYPES(M, broadcast, all_test_types)  //{{{1
{
    static_assert(std::is_convertible<typename M::reference, bool>::value,
                  "A smart_reference<simd_mask> must be convertible to bool.");
    static_assert(std::is_same<bool, decltype(std::declval<const typename M::reference &>() == true)>::value,
                  "A smart_reference<simd_mask> must be comparable against bool.");
    static_assert(vir::test::sfinae_is_callable<typename M::reference &&, bool>(
                      [](auto &&a, auto &&b) -> decltype(std::declval<decltype(a)>() ==
                                                         std::declval<decltype(b)>()) {
                          return {};
                      }),
                  "A smart_reference<simd_mask> must be comparable against bool.");
    VERIFY(std::experimental::is_simd_mask_v<M>);

    {
        M x;      // uninitialized
        x = M{};  // default broadcasts 0
        COMPARE(x, M(false));
        COMPARE(x, M());
        COMPARE(x, M{});
        x = M();  // default broadcasts 0
        COMPARE(x, M(false));
        COMPARE(x, M());
        COMPARE(x, M{});
        x = x;
        for (std::size_t i = 0; i < M::size(); ++i) {
            COMPARE(x[i], false);
        }
    }

    M x(true);
    M y(false);
    for (std::size_t i = 0; i < M::size(); ++i) {
        COMPARE(x[i], true);
        COMPARE(y[i], false);
    }
    y = M(true);
    COMPARE(x, y);
}

TEST_TYPES(M, operators, all_test_types)  //{{{1
{
    {  // compares{{{2
        M x(true), y(false);
        VERIFY(all_of(x == x));
        VERIFY(all_of(x != y));
        VERIFY(all_of(y != x));
        VERIFY(!all_of(x != x));
        VERIFY(!all_of(x == y));
        VERIFY(!all_of(y == x));
    }
    {  // subscripting{{{2
        M x(true);
        for (std::size_t i = 0; i < M::size(); ++i) {
            COMPARE(x[i], true) << "\nx: " << x << ", i: " << i;
            x[i] = !x[i];
        }
        COMPARE(x, M{false});
        for (std::size_t i = 0; i < M::size(); ++i) {
            COMPARE(x[i], false) << "\nx: " << x << ", i: " << i;
            x[i] = !x[i];
        }
        COMPARE(x, M{true});
    }
    {  // negation{{{2
        M x(false);
        M y = !x;
        COMPARE(y, M{true});
        COMPARE(!y, x);
    }
}

// implicit_conversions {{{1
template <class M, class M2>
constexpr bool assign_should_work =
    std::is_same<M, M2>::value ||
    (std::is_same<typename M::abi_type, std::experimental::simd_abi::fixed_size<M::size()>>::value &&
     std::is_same<typename M::abi_type, typename M2::abi_type>::value);
template <class M, class M2>
constexpr bool assign_should_not_work = !assign_should_work<M, M2>;

template <class L, class R>
std::enable_if_t<assign_should_work<L, R>> implicit_conversions_test()
{
    L x = R(true);
    COMPARE(x, L(true));
    x = R(false);
    COMPARE(x, L(false));
    R y(false);
    y[0] = true;
    x = y;
    L ref(false);
    ref[0] = true;
    COMPARE(x, ref);
}

template <class L, class R>
std::enable_if_t<assign_should_not_work<L, R>> implicit_conversions_test()
{
    VERIFY((is_substitution_failure<L &, R, assignment>));
}

TEST_TYPES(M, implicit_conversions, all_test_types)
{
    using std::experimental::simd_mask;
    using std::experimental::native_simd_mask;
    using std::experimental::fixed_size_simd_mask;

    implicit_conversions_test<M, simd_mask<ldouble>>();
    implicit_conversions_test<M, simd_mask<double>>();
    implicit_conversions_test<M, simd_mask<float>>();
    implicit_conversions_test<M, simd_mask<ullong>>();
    implicit_conversions_test<M, simd_mask<llong>>();
    implicit_conversions_test<M, simd_mask<ulong>>();
    implicit_conversions_test<M, simd_mask<long>>();
    implicit_conversions_test<M, simd_mask<uint>>();
    implicit_conversions_test<M, simd_mask<int>>();
    implicit_conversions_test<M, simd_mask<ushort>>();
    implicit_conversions_test<M, simd_mask<short>>();
    implicit_conversions_test<M, simd_mask<uchar>>();
    implicit_conversions_test<M, simd_mask<schar>>();
    implicit_conversions_test<M, native_simd_mask<ldouble>>();
    implicit_conversions_test<M, native_simd_mask<double>>();
    implicit_conversions_test<M, native_simd_mask<float>>();
    implicit_conversions_test<M, native_simd_mask<ullong>>();
    implicit_conversions_test<M, native_simd_mask<llong>>();
    implicit_conversions_test<M, native_simd_mask<ulong>>();
    implicit_conversions_test<M, native_simd_mask<long>>();
    implicit_conversions_test<M, native_simd_mask<uint>>();
    implicit_conversions_test<M, native_simd_mask<int>>();
    implicit_conversions_test<M, native_simd_mask<ushort>>();
    implicit_conversions_test<M, native_simd_mask<short>>();
    implicit_conversions_test<M, native_simd_mask<uchar>>();
    implicit_conversions_test<M, native_simd_mask<schar>>();
    implicit_conversions_test<M, fixed_size_simd_mask<ldouble, M::size()>>();
    implicit_conversions_test<M, fixed_size_simd_mask<double, M::size()>>();
    implicit_conversions_test<M, fixed_size_simd_mask<float, M::size()>>();
    implicit_conversions_test<M, fixed_size_simd_mask<ullong, M::size()>>();
    implicit_conversions_test<M, fixed_size_simd_mask<llong, M::size()>>();
    implicit_conversions_test<M, fixed_size_simd_mask<ulong, M::size()>>();
    implicit_conversions_test<M, fixed_size_simd_mask<long, M::size()>>();
    implicit_conversions_test<M, fixed_size_simd_mask<uint, M::size()>>();
    implicit_conversions_test<M, fixed_size_simd_mask<int, M::size()>>();
    implicit_conversions_test<M, fixed_size_simd_mask<ushort, M::size()>>();
    implicit_conversions_test<M, fixed_size_simd_mask<short, M::size()>>();
    implicit_conversions_test<M, fixed_size_simd_mask<uchar, M::size()>>();
    implicit_conversions_test<M, fixed_size_simd_mask<schar, M::size()>>();
}

TEST_TYPES(M, load_store, concat<all_test_types, many_fixed_size_types>)  //{{{1
{
    // loads {{{2
    constexpr size_t alignment = 2 * std::experimental::memory_alignment_v<M>;
    alignas(alignment) bool mem[3 * M::size()];
    std::memset(mem, 0, sizeof(mem));
    for (std::size_t i = 1; i < sizeof(mem) / sizeof(*mem); i += 2) {
        COMPARE(mem[i - 1], false);
        mem[i] = true;
    }
    using std::experimental::element_aligned;
    using std::experimental::vector_aligned;
    constexpr size_t stride_alignment =
        M::size() & 1 ? 1 : M::size() & 2
                                ? 2
                                : M::size() & 4
                                      ? 4
                                      : M::size() & 8
                                            ? 8
                                            : M::size() & 16
                                                  ? 16
                                                  : M::size() & 32
                                                        ? 32
                                                        : M::size() & 64
                                                              ? 64
                                                              : M::size() & 128
                                                                    ? 128
                                                                    : M::size() & 256
                                                                          ? 256
                                                                          : 512;
    using stride_aligned_t =
        std::conditional_t<M::size() == stride_alignment, decltype(vector_aligned),
                           std::experimental::overaligned_tag<stride_alignment * sizeof(bool)>>;
    constexpr stride_aligned_t stride_aligned = {};
    constexpr auto overaligned = std::experimental::overaligned<alignment>;

    const M alternating_mask = make_alternating_mask<M>();

    M x(&mem[M::size()], stride_aligned);
    COMPARE(x, M::size() % 2 == 1 ? !alternating_mask : alternating_mask)
        << x.__to_bitset() << ", alternating_mask: " << alternating_mask.__to_bitset();
    x = {&mem[1], element_aligned};
    COMPARE(x, !alternating_mask);
    x = M{mem, overaligned};
    COMPARE(x, alternating_mask);

    x.copy_from(&mem[M::size()], stride_aligned);
    COMPARE(x, M::size() % 2 == 1 ? !alternating_mask : alternating_mask);
    x.copy_from(&mem[1], element_aligned);
    COMPARE(x, !alternating_mask);
    x.copy_from(mem, vector_aligned);
    COMPARE(x, alternating_mask);

    x = !alternating_mask;
    where(alternating_mask, x).copy_from(&mem[M::size()], stride_aligned);
    COMPARE(x, M::size() % 2 == 1 ? !alternating_mask : M{true});
    x = M(true);                                                   // 1111
    where(alternating_mask, x).copy_from(&mem[1], element_aligned);  // load .0.0
    COMPARE(x, !alternating_mask);                                 // 1010
    where(alternating_mask, x).copy_from(mem, overaligned);          // load .1.1
    COMPARE(x, M{true});                                           // 1111

    // stores {{{2
    memset(mem, 0, sizeof(mem));
    x = M(true);
    x.copy_to(&mem[M::size()], stride_aligned);
    std::size_t i = 0;
    for (; i < M::size(); ++i) {
        COMPARE(mem[i], false);
    }
    for (; i < 2 * M::size(); ++i) {
        COMPARE(mem[i], true) << "i: " << i << ", x: " << x;
    }
    for (; i < 3 * M::size(); ++i) {
        COMPARE(mem[i], false);
    }
    memset(mem, 0, sizeof(mem));
    x.copy_to(&mem[1], element_aligned);
    COMPARE(mem[0], false);
    for (i = 1; i <= M::size(); ++i) {
        COMPARE(mem[i], true);
    }
    for (; i < 3 * M::size(); ++i) {
        COMPARE(mem[i], false);
    }
    memset(mem, 0, sizeof(mem));
    alternating_mask.copy_to(mem, overaligned);
    for (i = 0; i < M::size(); ++i) {
        COMPARE(mem[i], (i & 1) == 1);
    }
    for (; i < 3 * M::size(); ++i) {
        COMPARE(mem[i], false);
    }
    x.copy_to(mem, vector_aligned);
    where(alternating_mask, !x).copy_to(mem, overaligned);
    for (i = 0; i < M::size(); ++i) {
        COMPARE(mem[i], i % 2 == 0);
    }
    for (; i < 3 * M::size(); ++i) {
        COMPARE(mem[i], false);
    }
}

TEST_TYPES(M, operator_conversions, current_native_mask_test_types)  //{{{1
{
    // binary ops without conversions work
    COMPARE(typeid(M() & M()), typeid(M));

    // nothing else works: no implicit conv. or ambiguous
    using std::experimental::simd_mask;
    using std::experimental::native_simd_mask;
    using std::experimental::fixed_size_simd_mask;
    auto &&sfinae_test = [](auto x) {
        return is_substitution_failure<M, decltype(x), std::bit_and<>>;
    };
    VERIFY(sfinae_test(bool()));

    {
        auto &&is = [](auto x) { return std::is_same<M, simd_mask<decltype(x)>>::value; };
        COMPARE(!is(ldouble()), sfinae_test(simd_mask<ldouble>()));
        COMPARE(!is(double ()), sfinae_test(simd_mask<double >()));
        COMPARE(!is(float  ()), sfinae_test(simd_mask<float  >()));
        COMPARE(!is(ullong ()), sfinae_test(simd_mask<ullong >()));
        COMPARE(!is(llong  ()), sfinae_test(simd_mask<llong  >()));
        COMPARE(!is(ulong  ()), sfinae_test(simd_mask<ulong  >()));
        COMPARE(!is(long   ()), sfinae_test(simd_mask<long   >()));
        COMPARE(!is(uint   ()), sfinae_test(simd_mask<uint   >()));
        COMPARE(!is(int    ()), sfinae_test(simd_mask<int    >()));
        COMPARE(!is(ushort ()), sfinae_test(simd_mask<ushort >()));
        COMPARE(!is(short  ()), sfinae_test(simd_mask<short  >()));
        COMPARE(!is(uchar  ()), sfinae_test(simd_mask<uchar  >()));
        COMPARE(!is(schar  ()), sfinae_test(simd_mask<schar  >()));
    }

    VERIFY(sfinae_test(fixed_size_simd_mask<ldouble, 2>()));
    VERIFY(sfinae_test(fixed_size_simd_mask<double , 2>()));
    VERIFY(sfinae_test(fixed_size_simd_mask<float  , 2>()));
    VERIFY(sfinae_test(fixed_size_simd_mask<ullong , 2>()));
    VERIFY(sfinae_test(fixed_size_simd_mask<llong  , 2>()));
    VERIFY(sfinae_test(fixed_size_simd_mask<ulong  , 2>()));
    VERIFY(sfinae_test(fixed_size_simd_mask<long   , 2>()));
    VERIFY(sfinae_test(fixed_size_simd_mask<uint   , 2>()));
    VERIFY(sfinae_test(fixed_size_simd_mask<int    , 2>()));
    VERIFY(sfinae_test(fixed_size_simd_mask<ushort , 2>()));
    VERIFY(sfinae_test(fixed_size_simd_mask<short  , 2>()));
    VERIFY(sfinae_test(fixed_size_simd_mask<uchar  , 2>()));
    VERIFY(sfinae_test(fixed_size_simd_mask<schar  , 2>()));

    {
        auto &&is = [](auto x) { return std::is_same<M, native_simd_mask<decltype(x)>>::value; };
        if (!is(ldouble())) VERIFY(sfinae_test(native_simd_mask<ldouble>()));
        if (!is(double ())) VERIFY(sfinae_test(native_simd_mask<double >()));
        if (!is(float  ())) VERIFY(sfinae_test(native_simd_mask<float  >()));
        if (!is(ullong ())) VERIFY(sfinae_test(native_simd_mask<ullong >()));
        if (!is(llong  ())) VERIFY(sfinae_test(native_simd_mask<llong  >()));
        if (!is(ulong  ())) VERIFY(sfinae_test(native_simd_mask<ulong  >()));
        if (!is(long   ())) VERIFY(sfinae_test(native_simd_mask<long   >()));
        if (!is(uint   ())) VERIFY(sfinae_test(native_simd_mask<uint   >()));
        if (!is(int    ())) VERIFY(sfinae_test(native_simd_mask<int    >()));
        if (!is(ushort ())) VERIFY(sfinae_test(native_simd_mask<ushort >()));
        if (!is(short  ())) VERIFY(sfinae_test(native_simd_mask<short  >()));
        if (!is(uchar  ())) VERIFY(sfinae_test(native_simd_mask<uchar  >()));
        if (!is(schar  ())) VERIFY(sfinae_test(native_simd_mask<schar  >()));
    }
}

TEST_TYPES(M, reductions, all_test_types)  //{{{1
{
    const M alternating_mask = make_alternating_mask<M>();
    COMPARE(alternating_mask[0], false);  // assumption below
    auto &&gen = make_mask<M>;

    // all_of
    VERIFY( all_of(M{true}));
    VERIFY(!all_of(alternating_mask));
    VERIFY(!all_of(M{false}));
    using std::experimental::all_of;
    VERIFY( all_of(true));
    VERIFY(!all_of(false));
    VERIFY( sfinae_is_callable< bool>([](auto x) -> decltype(std::experimental::all_of(x)) { return {}; }));
    VERIFY(!sfinae_is_callable<  int>([](auto x) -> decltype(std::experimental::all_of(x)) { return {}; }));
    VERIFY(!sfinae_is_callable<float>([](auto x) -> decltype(std::experimental::all_of(x)) { return {}; }));
    VERIFY(!sfinae_is_callable< char>([](auto x) -> decltype(std::experimental::all_of(x)) { return {}; }));

    // any_of
    VERIFY( any_of(M{true}));
    COMPARE(any_of(alternating_mask), M::size() > 1);
    VERIFY(!any_of(M{false}));
    using std::experimental::any_of;
    VERIFY( any_of(true));
    VERIFY(!any_of(false));
    VERIFY( sfinae_is_callable< bool>([](auto x) -> decltype(std::experimental::any_of(x)) { return {}; }));
    VERIFY(!sfinae_is_callable<  int>([](auto x) -> decltype(std::experimental::any_of(x)) { return {}; }));
    VERIFY(!sfinae_is_callable<float>([](auto x) -> decltype(std::experimental::any_of(x)) { return {}; }));
    VERIFY(!sfinae_is_callable< char>([](auto x) -> decltype(std::experimental::any_of(x)) { return {}; }));

    // none_of
    VERIFY(!none_of(M{true}));
    COMPARE(none_of(alternating_mask), M::size() == 1);
    VERIFY( none_of(M{false}));
    using std::experimental::none_of;
    VERIFY(!none_of(true));
    VERIFY( none_of(false));
    VERIFY( sfinae_is_callable< bool>([](auto x) -> decltype(std::experimental::none_of(x)) { return {}; }));
    VERIFY(!sfinae_is_callable<  int>([](auto x) -> decltype(std::experimental::none_of(x)) { return {}; }));
    VERIFY(!sfinae_is_callable<float>([](auto x) -> decltype(std::experimental::none_of(x)) { return {}; }));
    VERIFY(!sfinae_is_callable< char>([](auto x) -> decltype(std::experimental::none_of(x)) { return {}; }));

    // some_of
    VERIFY(!some_of(M{true}));
    VERIFY(!some_of(M{false}));
    if (M::size() > 1) {
        VERIFY(some_of(gen({true, false})));
        VERIFY(some_of(gen({false, true})));
        if (M::size() > 3) {
            VERIFY(some_of(gen({0, 0, 0, 1})));
        }
    }
    using std::experimental::some_of;
    VERIFY(!some_of(true));
    VERIFY(!some_of(false));
    VERIFY( sfinae_is_callable< bool>([](auto x) -> decltype(std::experimental::some_of(x)) { return {}; }));
    VERIFY(!sfinae_is_callable<  int>([](auto x) -> decltype(std::experimental::some_of(x)) { return {}; }));
    VERIFY(!sfinae_is_callable<float>([](auto x) -> decltype(std::experimental::some_of(x)) { return {}; }));
    VERIFY(!sfinae_is_callable< char>([](auto x) -> decltype(std::experimental::some_of(x)) { return {}; }));

    // popcount
    COMPARE(popcount(M{true}), int(M::size()));
    COMPARE(popcount(alternating_mask), int(M::size()) / 2);
    COMPARE(popcount(M{false}), 0);
    COMPARE(popcount(gen({0, 0, 1})), int(M::size()) / 3);
    COMPARE(popcount(gen({0, 0, 0, 1})), int(M::size()) / 4);
    COMPARE(popcount(gen({0, 0, 0, 0, 1})), int(M::size()) / 5);
    COMPARE(std::experimental::popcount(true), 1);
    COMPARE(std::experimental::popcount(false), 0);
    VERIFY( sfinae_is_callable< bool>([](auto x) -> decltype(std::experimental::popcount(x)) { return {}; }));
    VERIFY(!sfinae_is_callable<  int>([](auto x) -> decltype(std::experimental::popcount(x)) { return {}; }));
    VERIFY(!sfinae_is_callable<float>([](auto x) -> decltype(std::experimental::popcount(x)) { return {}; }));
    VERIFY(!sfinae_is_callable< char>([](auto x) -> decltype(std::experimental::popcount(x)) { return {}; }));

    // find_first_set
    {
        M x(false);
        for (int i = int(M::size() / 2 - 1); i >= 0; --i) {
            x[i] = true;
            COMPARE(find_first_set(x), i) << x;
        }
        x = M(false);
        for (int i = int(M::size() - 1); i >= 0; --i) {
            x[i] = true;
            COMPARE(find_first_set(x), i) << x;
        }
    }
    COMPARE(find_first_set(M{true}), 0);
    if (M::size() > 1) {
        COMPARE(find_first_set(gen({0, 1})), 1);
    }
    if (M::size() > 2) {
        COMPARE(find_first_set(gen({0, 0, 1})), 2);
    }
    COMPARE(std::experimental::find_first_set(true), 0);
    VERIFY( sfinae_is_callable< bool>([](auto x) -> decltype(std::experimental::find_first_set(x)) { return {}; }));
    VERIFY(!sfinae_is_callable<  int>([](auto x) -> decltype(std::experimental::find_first_set(x)) { return {}; }));
    VERIFY(!sfinae_is_callable<float>([](auto x) -> decltype(std::experimental::find_first_set(x)) { return {}; }));
    VERIFY(!sfinae_is_callable< char>([](auto x) -> decltype(std::experimental::find_first_set(x)) { return {}; }));

    // find_last_set
    {
        M x(false);
        for (int i = 0; i < int(M::size()); ++i) {
            x[i] = true;
            COMPARE(find_last_set(x), i) << x;
        }
    }
    COMPARE(find_last_set(M{true}), int(M::size()) - 1);
    if (M::size() > 1) {
        COMPARE(find_last_set(gen({1, 0})), int(M::size()) - 2 + int(M::size() & 1));
    }
    if (M::size() > 3 && (M::size() & 3) == 0) {
        COMPARE(find_last_set(gen({1, 0, 0, 0})), int(M::size()) - 4 - int(M::size() & 3));
    }
    COMPARE(std::experimental::find_last_set(true), 0);
    VERIFY( sfinae_is_callable< bool>([](auto x) -> decltype(std::experimental::find_last_set(x)) { return {}; }));
    VERIFY(!sfinae_is_callable<  int>([](auto x) -> decltype(std::experimental::find_last_set(x)) { return {}; }));
    VERIFY(!sfinae_is_callable<float>([](auto x) -> decltype(std::experimental::find_last_set(x)) { return {}; }));
    VERIFY(!sfinae_is_callable< char>([](auto x) -> decltype(std::experimental::find_last_set(x)) { return {}; }));
}


// vim: foldmethod=marker
