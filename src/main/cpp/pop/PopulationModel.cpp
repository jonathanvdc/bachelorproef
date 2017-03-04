#include "PopulationModel.h"

namespace stride {

PopulationModel::PopulationModel(const boost::property_tree::ptree& pt) {
	maximum_age = pt.get<int>("popgen.age.maximum");
}

}