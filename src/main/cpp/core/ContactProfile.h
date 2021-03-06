#ifndef CONTACT_PROFILE_H_INCLUDED
#define CONTACT_PROFILE_H_INCLUDED
/*
 *  This is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *  The software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with the software. If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2015, Willem L, Kuylen E, Stijven S & Broeckhove J
 */

/**
 * @file
 * Contact profile.
 */

#include "core/ClusterType.h"
#include "pop/Age.h"

#include <boost/property_tree/ptree.hpp>
#include <array>

namespace stride {

class ContactProfile : public std::array<double, MaximumAge() + 1>
{
public:
        /// Need to keep the default constructor available.
        ContactProfile() {};

        /// Explicitly initialize
        ContactProfile(ClusterType cluster_type,  const boost::property_tree::ptree& pt_contacts);
};

} // namespace

#endif // end-of-include-guard
