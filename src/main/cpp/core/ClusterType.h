#ifndef CLUSTER_TYPE_H_INCLUDED
#define CLUSTER_TYPE_H_INCLUDED

#include <cstddef>
#include <string>

namespace stride {

/// Enumerates the cluster types.
enum class ClusterType
{
	Household,
	School,
	Work,
	PrimaryCommunity,
	SecondaryCommunity,
	Null
};

/// Number of Cluster types (not including Null type).
inline constexpr unsigned int NumOfClusterTypes() { return 5U; }

/// Cast for array access.
inline std::size_t ToSizeType(ClusterType c) { return static_cast<std::size_t>(c); }

/// Converts a ClusterType value to corresponding name.
std::string ToString(ClusterType w);

/// Check whether string is name of a ClusterType value.
bool IsClusterType(const std::string& s);

/// Converts a string with name to ClusterType value.
ClusterType ToClusterType(const std::string& s);

} // namespace

#endif // end-of-include-guard
