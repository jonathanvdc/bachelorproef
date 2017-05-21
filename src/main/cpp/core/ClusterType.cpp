#include "ClusterType.h"

#include <map>
#include <string>
#include <boost/algorithm/string.hpp>

namespace {

using stride::ClusterType;
using boost::to_upper;
using namespace std;

map<ClusterType, string> g_cluster_type_name{make_pair(ClusterType::Household, "household"),
					     make_pair(ClusterType::School, "school"),
					     make_pair(ClusterType::Work, "work"),
					     make_pair(ClusterType::PrimaryCommunity, "primary_community"),
					     make_pair(ClusterType::SecondaryCommunity, "secondary_community"),
					     make_pair(ClusterType::Null, "null")};

map<string, ClusterType> g_name_cluster_type{make_pair("HOUSEHOLD", ClusterType::Household),
					     make_pair("SCHOOL", ClusterType::School),
					     make_pair("WORK", ClusterType::Work),
					     make_pair("PRIMARY_COMMUNITY", ClusterType::PrimaryCommunity),
					     make_pair("SECONDARY_COMMUNITY", ClusterType::SecondaryCommunity),
					     make_pair("NULL", ClusterType::Null)};
}

namespace stride {

string ToString(ClusterType c) { return (g_cluster_type_name.count(c) == 1) ? g_cluster_type_name[c] : "Null"; }

bool IsClusterType(const string& s)
{
	std::string t{s};
	to_upper(t);
	return (g_name_cluster_type.count(t) == 1);
}

ClusterType ToClusterType(const string& s)
{
	std::string t{s};
	to_upper(t);
	return (g_name_cluster_type.count(s) == 1) ? g_name_cluster_type[s] : ClusterType::Null;
}

} // namespace
