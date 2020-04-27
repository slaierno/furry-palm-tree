#pragma once

namespace cx {    
    template<class It, class UnaryPredicate>
    constexpr It find_if(It first, It last, UnaryPredicate p)
    {
        for (; first != last; ++first) if (p(*first)) return first;
        return last;
    }

    template <class It1, class It2>
    constexpr bool equal(It1 first1, It1 last1, It2 first2, It2 last2)
    {
        while (first1 != last1 && first2 != last2 && *first1 == *first2) ++first1, ++first2;
        return first1 == last1 && first2 == last2;
    }
}