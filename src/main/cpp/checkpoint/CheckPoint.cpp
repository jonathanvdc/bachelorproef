/*
 * CheckPoint.cpp
 *
 *  Created on: Mar 26, 2017
 *      Author: cedric
 */

#include "CheckPoint.h"
#include "hdf5.h"

#include <calendar/Calendar.h>
#include <core/ClusterType.h>
#include <core/Health.h>
#include <multiregion/TravelModel.h>
#include <pop/Person.h>
#include <util/Errors.h>
#include <util/InstallDirs.h>

#include <cstring>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>

using namespace std;

namespace stride {
namespace checkpoint {

CheckPoint::CheckPoint(const std::string& filename, unsigned int interval) : m_filename(filename), m_limit(interval) {}

void CheckPoint::CreateFile()
{
	m_file = H5Fcreate(m_filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	H5Fclose(m_file);
}

void CheckPoint::CloseFile() { H5Fclose(m_file); }

void CheckPoint::WriteConfig(const SingleSimulationConfig& conf)
{
	htri_t exist = H5Lexists(m_file, "Config", H5P_DEFAULT);
	if (exist <= 0) {
		hid_t temp = H5Gcreate2(m_file, "Config", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		H5Gclose(temp);
	}

	std::shared_ptr<CommonSimulationConfig> common_config = conf.common_config;
	std::shared_ptr<LogConfig> log_config = conf.log_config;

	WriteFileDSet(common_config->disease_config_file_name, "disease");
	WriteFileDSet(common_config->contact_matrix_file_name, "contact");
	WriteFileDSet(conf.GetPopulationPath(), "popconfig");
	if (conf.GetGeodistributionProfilePath().size() != 0) {
		WriteFileDSet(conf.GetGeodistributionProfilePath(), "geoconfig");
	}
	if (conf.GetReferenceHouseholdsPath().size() != 0) {
		WriteFileDSet(conf.GetReferenceHouseholdsPath(), "household");
	}

	hid_t group = H5Gopen2(m_file, "Config", H5P_DEFAULT);
	hsize_t dims = 2;
	hid_t dataspace = H5Screate_simple(1, &dims, NULL);
	hbool_t bools[2];
	bools[0] = common_config->track_index_case;
	bools[1] = log_config->generate_person_file;
	hid_t attr = H5Acreate2(group, "bools", H5T_IEEE_F64LE, dataspace, H5P_DEFAULT, H5P_DEFAULT);
	H5Awrite(attr, H5T_NATIVE_HBOOL, bools);
	H5Sclose(dataspace);
	H5Aclose(attr);

	dims = 4;
	dataspace = H5Screate_simple(1, &dims, NULL);
	attr = H5Acreate2(group, "uints", H5T_IEEE_F64LE, dataspace, H5P_DEFAULT, H5P_DEFAULT);
	unsigned int uints[4];
	uints[0] = common_config->rng_seed;
	uints[1] = common_config->number_of_days;
	uints[2] = common_config->number_of_survey_participants;
	uints[3] = (unsigned int)log_config->log_level;
	H5Awrite(attr, H5T_NATIVE_UINT, uints);
	H5Sclose(dataspace);
	H5Aclose(attr);

	dims = 3;
	dataspace = H5Screate_simple(1, &dims, NULL);
	attr = H5Acreate2(group, "doubles", H5T_IEEE_F64LE, dataspace, H5P_DEFAULT, H5P_DEFAULT);
	double doubles[3];
	doubles[0] = common_config->r0;
	doubles[1] = common_config->seeding_rate;
	doubles[2] = common_config->immunity_rate;

	H5Awrite(attr, H5T_NATIVE_DOUBLE, doubles);
	H5Sclose(dataspace);
	H5Aclose(attr);

	dims = log_config->output_prefix.size();
	dataspace = H5Screate_simple(1, &dims, NULL);
	attr = H5Acreate2(group, "prefix", H5T_IEEE_F64LE, dataspace, H5P_DEFAULT, H5P_DEFAULT);
	char prefix[log_config->output_prefix.size()];
	strncpy(prefix, log_config->output_prefix.c_str(), sizeof(prefix));
	prefix[sizeof(prefix) - 1] = 0;
	H5Awrite(attr, H5T_NATIVE_CHAR, prefix);
	H5Sclose(dataspace);
	H5Aclose(attr);

	H5Gclose(group);
}

void CheckPoint::WriteConfig(const MultiSimulationConfig& conf)
{
	hid_t f = m_file;
	auto singles = conf.GetSingleConfigs();
	if (singles.size() == 1) {
		WriteConfig(singles[0]);
		return;
	}
	for (unsigned int i = 0; i < singles.size(); i++) {
		std::stringstream ss;
		ss << "Simulation " << i;

		m_file = H5Gcreate2(m_file, ss.str().c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		WriteConfig(singles[i]);
		H5Gclose(m_file);
	}
	m_file = f;
}

void CheckPoint::OpenFile() { m_file = H5Fopen(m_filename.c_str(), H5F_ACC_RDWR, H5P_DEFAULT); }

void CheckPoint::WriteHolidays(const std::string& filename, unsigned int* group)
{
	hid_t temp = m_file;
	if (group != NULL) {
		std::stringstream ss;
		ss << "Simulation " << *group;
		m_file = H5Gopen2(m_file, ss.str().c_str(), H5P_DEFAULT);
	}
	htri_t exist = H5Lexists(m_file, "Config", H5P_DEFAULT);
	if (exist <= 0) {
		hid_t tempCreate = H5Gcreate2(m_file, "Config", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		H5Gclose(tempCreate);
	}
	WriteFileDSet(filename, "holidays");
	if (group != NULL) {
		H5Gclose(m_file);
		m_file = temp;
	}
}

void CheckPoint::WritePopulation(const Population& pop, unsigned int date)
{
	htri_t exist = H5Lexists(m_file, "Population", H5P_DEFAULT);
	if (exist <= 0) {
		hid_t temp = H5Gcreate2(m_file, "Population", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		H5Gclose(temp);
	}
	hsize_t entries = 10 + NumOfClusterTypes();

	hsize_t dims[2] = {pop.size(), entries};
	hid_t dataspace = H5Screate_simple(2, dims, NULL);
	std::string spot = "Population/" + std::to_string(date);

	hid_t chunkP = H5Pcreate(H5P_DATASET_CREATE);
	hsize_t chunkDims[2] = {1, entries};
	H5Pset_chunk(chunkP, 2, chunkDims);

	hid_t dataset = H5Dcreate2(m_file, spot.c_str(), H5T_NATIVE_UINT, dataspace, H5P_DEFAULT, chunkP, H5P_DEFAULT);

	chunkP = H5Pcreate(H5P_DATASET_XFER);
	unsigned int i = 0;
	for (const auto& p : pop) {
		unsigned int data[1][entries];
		// Basic Data
		data[0][0] = p.GetId();
		data[0][1] = p.GetAge();
		data[0][2] = p.GetGender();
		data[0][3] = p.IsParticipatingInSurvey();

		// Health data
		data[0][4] = (unsigned int)p.GetHealth().IsImmune();
		data[0][5] = p.GetHealth().GetStartInfectiousness();
		data[0][6] = p.GetHealth().GetEndInfectiousness();
		data[0][7] = p.GetHealth().GetStartSymptomatic();
		data[0][8] = p.GetHealth().GetEndSymptomatic();
		data[0][9] = p.GetHealth().GetDaysInfected();

		// Cluster data
		unsigned int k = 10;
		for (unsigned int j = 0; j < NumOfClusterTypes(); j++) {
			ClusterType temp = (ClusterType)j;
			if (ToString(temp) == "Null") {
				continue;
			}
			data[0][k] = p.GetClusterId(temp);
			k++;
		}

		hsize_t start[2] = {i, 0};
		hsize_t count[2] = {1, entries};
		hid_t chunkspace = H5Screate_simple(2, count, NULL);
		H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, start, NULL, count, NULL);
		H5Dwrite(dataset, H5T_NATIVE_UINT, chunkspace, dataspace, chunkP, data);
		i++;
		H5Sclose(chunkspace);
	}

	H5Dclose(dataset);
	H5Sclose(dataspace);
}

void CheckPoint::WriteFileDSet(const std::string& filename, const std::string& setname)
{
	boost::filesystem::path path = util::InstallDirs::GetDataDir();
	boost::filesystem::path filep(filename);
	boost::filesystem::path fullpath = path / filep;
	if (!is_regular_file(fullpath)) {
		FATAL_ERROR("Unable to find file: " + fullpath.string());
	}
	std::ifstream f(fullpath.string());

	std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
	std::vector<char> dataset;
	std::copy(str.begin(), str.end(), std::back_inserter(dataset));

	hid_t group = H5Gopen2(m_file, "Config", H5P_DEFAULT);
	hsize_t dims[1] = {dataset.size()};
	hid_t dataspace = H5Screate_simple(1, dims, NULL);
	hid_t dset =
	    H5Dcreate2(group, setname.c_str(), H5T_NATIVE_CHAR, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(dset, H5T_NATIVE_CHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(*dataset.begin()));

	H5Dclose(dset);
	H5Sclose(dataspace);
	H5Gclose(group);
}

void CheckPoint::WriteDSetFile(const std::string& filestr, const std::string& setname){
	htri_t exist = H5Lexists(m_file, setname.c_str(), H5P_DEFAULT);
	if (exist <= 0) {
		FATAL_ERROR("Invalid file");
	}

	auto path = util::InstallDirs::GetDataDir();

	boost::filesystem::path filename(filestr);
	std::ofstream out((path / filename).string(), std::ios::out | std::ios::trunc);
	hid_t dset = H5Dopen(m_file, setname.c_str(), H5P_DEFAULT);

	hid_t dspace = H5Dget_space(dset);

	hsize_t dims[1];
	H5Sget_simple_extent_dims(dspace, dims, NULL);

	char data[dims[0]];
	H5Dread(dset, H5T_NATIVE_CHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
	out.write(data,dims[0]);

	out.close();
	H5Dclose(dset);
	H5Sclose(dspace);

}

void CheckPoint::SaveCheckPoint(const Population& pop, unsigned int time)
{
	// TODO: add airport
	m_lastCh++;
	if (m_lastCh == m_limit or time == 0) {
		m_lastCh = 0;
		WritePopulation(pop, time);
	}
}

void CheckPoint::SaveCheckPoint(const std::string& filename, unsigned int groupnum)
{
	std::stringstream ss;
	ss << "Simulation " << groupnum;
	std::string groupname = ss.str();

	htri_t exist = H5Lexists(m_file, groupname.c_str(), H5P_DEFAULT);
	if (exist <= 0) {
		hid_t temp = H5Gcreate2(m_file, groupname.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		H5Gclose(temp);
	}

	hid_t group = H5Gopen2(m_file, groupname.c_str(), H5P_DEFAULT);

	hid_t pop_group = H5Gopen2(group, "Population", H5P_DEFAULT);

	hid_t newFile = H5Fopen(filename.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);

	hid_t source_group = H5Gopen2(newFile, "Population", H5P_DEFAULT);

	H5G_info_t* to_move = new H5G_info_t();

	H5Gget_info(source_group, to_move);

	auto op_func = [&pop_group](hid_t loc_id, const char* name, const H5L_info_t* info, void* operator_data) {
		H5Ocopy(loc_id, name, pop_group, name, H5P_DEFAULT, H5P_DEFAULT);
		return 0;
	};
	auto temp = [](hid_t loc_id, const char* name, const H5L_info_t* info, void* operator_data) {
		(*static_cast<decltype(op_func)*>(operator_data))(loc_id, name, info, operator_data);
		return 0;
	};

	H5Literate(source_group, H5_INDEX_NAME, H5_ITER_NATIVE, NULL, temp, &op_func);

	H5Gclose(source_group);
	H5Fclose(newFile);
	H5Gclose(pop_group);
	H5Gclose(group);
}

Population CheckPoint::LoadCheckPoint(unsigned int date)
{

	std::cout<<"Loading CheckPoint"<<std::endl;
	htri_t exist = H5Lexists(m_file, "Population", H5P_DEFAULT);
	if (exist <= 0) {
		FATAL_ERROR("Tried to load incorrect file");
	}
	std::string name = "Population/" + to_string(date);
	exist = H5Lexists(m_file, name.c_str(), H5P_DEFAULT);
	if (exist <= 0) {
		FATAL_ERROR("Incorrect date loaded");
	}

	hid_t dset = H5Dopen(m_file, name.c_str(), H5P_DEFAULT);
	hid_t dspace = H5Dget_space(dset);

	hsize_t dims[2];
	H5Sget_simple_extent_dims(dspace, dims, NULL);

	Population result;

	for (hsize_t i = 0; i < dims[0]; i++) {
		hsize_t start[2];
		start[0] = i;
		start[1] = 0;

		hsize_t count[2];
		count[0] = 0;
		count[1] = dims[1];

		hid_t subspace = H5Dget_space(dset);

		H5Sselect_hyperslab(subspace, H5S_SELECT_SET, start, NULL, count, NULL);

		unsigned int data[1][dims[1]];

		H5Dread(dset, H5T_NATIVE_UINT, H5S_ALL, subspace, H5P_DEFAULT, data[0]);

		disease::Fate disease;
		disease.start_infectiousness = data[0][5];
		disease.start_symptomatic = data[0][7];
		disease.end_infectiousness = data[0][6];
		disease.end_symptomatic = data[0][8];

		Person toAdd(
		    data[0][0], data[0][1], data[0][10], data[0][11], data[0][12], data[0][13], data[0][14], disease);

		if ((bool)data[0][3]) {
			toAdd.ParticipateInSurvey();
		}

		if ((bool)data[0][4]) {
			toAdd.GetHealth().SetImmune();
		}

		for (unsigned int i = 0; i < data[0][9]; i++) {
			toAdd.GetHealth().Update();
		}

		result.emplace(toAdd);
	}
	std::cout<<"Loaded Population"<<std::endl;

	return result;
}

SingleSimulationConfig CheckPoint::LoadSingleConfig(unsigned int id)
{
	htri_t exist = H5Lexists(m_file, "Config", H5P_DEFAULT);
	if (exist <= 0) {
		FATAL_ERROR("Invalid file");
	}

	SingleSimulationConfig result;
	std::shared_ptr<CommonSimulationConfig> comCon(new CommonSimulationConfig());
	std::shared_ptr<LogConfig> logCon(new LogConfig());
	result.common_config = comCon;
	result.log_config = logCon;

	WriteDSetFile("tmp_matrix.xml","Config/disease");
	result.common_config->contact_matrix_file_name = "tmp_matrix.xml";

	WriteDSetFile("tmp_disease.xml","Config/contact");
	result.common_config->disease_config_file_name = "tmp_disease.xml";

	hid_t group = H5Gopen2(m_file, "Config", H5P_DEFAULT);

	hid_t attr = H5Aopen(group, "bools", H5P_DEFAULT);
	hbool_t bools[2];
	H5Aread(attr, H5T_NATIVE_HBOOL, bools);
	H5Aclose(attr);

	result.common_config->track_index_case = bools[0];
	result.log_config->generate_person_file = bools[1];

	attr = H5Aopen(group, "doubles", H5P_DEFAULT);
	double doubles[3];
	H5Aread(attr, H5T_NATIVE_DOUBLE, doubles);
	H5Aclose(attr);

	result.common_config->r0 = doubles[0];
	result.common_config->seeding_rate = doubles[1];
	result.common_config->immunity_rate = doubles[2];

	attr = H5Aopen(group, "uints", H5P_DEFAULT);
	unsigned int uints[3];
	H5Aread(attr, H5T_NATIVE_UINT, uints);
	H5Aclose(attr);

	result.common_config->rng_seed = uints[0];
	result.common_config->number_of_days = uints[1];
	result.common_config->number_of_survey_participants = uints[2];
	result.log_config->log_level = (LogMode)uints[3];
     
	attr = H5Aopen(group, "prefix", H5P_DEFAULT);

	H5A_info_t* info = new H5A_info_t();

	H5Aget_info(attr, info);

	char prefix[info->data_size];

	delete info;
	H5Aread(attr, H5T_NATIVE_CHAR, prefix);
	H5Aclose(attr);

	result.log_config->output_prefix = prefix;

	// travel model
	/*
	WriteDSetFile("tmp_pop.xml","Config/popconfig");
	std::string popconfig = "tmp_pop.xml";
	std::string geoconfig ="";

	exist = H5Lexists(m_file, "Config/geoconfig", H5P_DEFAULT);
	if (exist > 0) {
		WriteDSetFile("tmp_geoconfig.xml","Config/geoconfig");
		geoconfig = "tmp_geoconfig.xml";
	}
	std::string household = "";
	exist = H5Lexists(m_file, "Config/household", H5P_DEFAULT);
	if (exist > 0) {
		WriteDSetFile("tmp_household.xml","Config/household");
		household = "tmp_household.xml";
	}
	std::shared_ptr<const multiregion::RegionTravel> travelRef(new
	multiregion::RegionTravel(id,popconfig,geoconfig,household));
	result.travel_model=travelRef;
	*/
	return result;
}

void CheckPoint::ToSingleFile(unsigned int groupnum, std::string filename)
{
	std::stringstream ss;
	ss << "Simulation " << groupnum;
	std::string groupname = ss.str();

	htri_t exist = H5Lexists(m_file, groupname.c_str(), H5P_DEFAULT);
	if (exist <= 0) {
		FATAL_ERROR("Non-existing group");
	}

	hid_t group = H5Gopen2(m_file, groupname.c_str(), H5P_DEFAULT);

	hid_t f = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	exist = H5Lexists(group, "Config", H5P_DEFAULT);
	if (exist > 0) {
		H5Ocopy(group, "Config", f, "Config", H5P_DEFAULT, H5P_DEFAULT);
	}
	exist = H5Lexists(group, "Population", H5P_DEFAULT);
	if (exist > 0) {
		H5Ocopy(group, "Population", f, "Population", H5P_DEFAULT, H5P_DEFAULT);
	}

	H5Fclose(f);
	H5Gclose(group);
}

Calendar CheckPoint::LoadCalendar(unsigned int date)
{
	htri_t exist = H5Lexists(m_file, "Config", H5P_DEFAULT);
	if (exist <= 0) {
		FATAL_ERROR("Invalid file");
	}

	WriteDSetFile("tmp_holidays.json","Config/holidays");		


	Calendar result;
	boost::gregorian::date d;
	d = boost::gregorian::from_undelimited_string(std::to_string(date));
	result.Initialize(d, "tmp_holidays.json");
	boost::filesystem::remove("tmp_holidays.json");
	return result;
}

} /* namespace checkpoint */
} /* namespace stride */
