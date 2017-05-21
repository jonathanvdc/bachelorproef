#ifndef AGE_H_INCLUDED
#define AGE_H_INCLUDED

#include <array>

namespace stride {

/// Maximum age for Person's.
inline constexpr unsigned int MaximumAge() { return 80U; }

/// Maximum age for Person's.
inline constexpr unsigned int MinAdultAge() { return 18U; }

/// Effective age (topping of at maximum).
inline unsigned int EffectiveAge(unsigned int age) { return (age <= MaximumAge()) ? age : MaximumAge(); }

} // namespace

#endif // end-of-include-guard
