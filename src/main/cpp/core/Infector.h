#ifndef INFECTOR_H_INCLUDED
#define INFECTOR_H_INCLUDED
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
 *  Copyright 2017, Willem L, Kuylen E, Stijven S & Broeckhove J
 */

/**
 * @file
 * Header for the core Cluster class.
 */

#include "LogMode.h"
#include <memory>

namespace indismo {

class Cluster;
class ContactHandler;
class WorldEnvironment;

/**
 * Represents a location for social contacts, an group of people.
 */
template<LogMode log_level, bool track_index_case = false>
class Infector
{
public:
	///
	static void Execute(Cluster& cluster,
	        std::shared_ptr<ContactHandler> contact_handler,
	        std::shared_ptr<const WorldEnvironment> sim_state);
};


extern template class Infector<LogMode::None, false>;

extern template class Infector<LogMode::None, true>;

extern template class Infector<LogMode::Transmissions, false>;

extern template class Infector<LogMode::Transmissions, true>;

extern template class Infector<LogMode::Contacts, false>;

extern template class Infector<LogMode::Contacts, true>;

} // end_of_namespace

#endif // include-guard
