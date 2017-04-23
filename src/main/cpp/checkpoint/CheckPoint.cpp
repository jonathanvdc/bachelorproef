/*
 * CheckPoint.cpp
 *
 *  Created on: Mar 26, 2017
 *      Author: cedric
 */

#include "CheckPoint.h"
#include "hdf5.h"

#include <core/ClusterType.h>
#include <core/Health.h>
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
		WriteFileDSet(conf.GetReferenceHouseholdsPath(), "contact");
	}

	hid_t group = H5Gopen2(m_file, "Config", H5P_DEFAULT);
	hsize_t dims = 2;
	hid_t dataspace = H5Screate_simple(1, &dims, NULL);
	bool bools[2];
	bools[0] = common_config->track_index_case;
	bools[1] = log_config->generate_person_file;
	hid_t attr = H5Acreate2(group, "bools", H5T_STD_I32BE, dataspace, H5P_DEFAULT, H5P_DEFAULT);
	H5Awrite(attr, H5T_NATIVE_INT, bools);
	H5Sclose(dataspace);
	H5Aclose(attr);

	dims = 4;
	dataspace = H5Screate_simple(1, &dims, NULL);
	attr = H5Acreate2(group, "uints", H5T_STD_I32BE, dataspace, H5P_DEFAULT, H5P_DEFAULT);
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
	attr = H5Acreate2(group, "doubles", H5T_STD_I32BE, dataspace, H5P_DEFAULT, H5P_DEFAULT);
	double doubles[3];
	doubles[0] = common_config->r0;
	doubles[1] = common_config->seeding_rate;
	doubles[2] = common_config->immunity_rate;
	H5Awrite(attr, H5T_NATIVE_DOUBLE, doubles);
	H5Sclose(dataspace);
	H5Aclose(attr);

	dims = log_config->output_prefix.size();
	dataspace = H5Screate_simple(1, &dims, NULL);
	attr = H5Acreate2(group, "prefix", H5T_STD_I32BE, dataspace, H5P_DEFAULT, H5P_DEFAULT);
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

void CheckPoint::WriteDate(const Calendar& cal)
{
	std::size_t day = cal.GetDay();
	std::size_t month = cal.GetMonth();
	std::size_t year = cal.GetYear();

	std::stringstream ss;
	ss << year;
	if (month < 10) {
		ss << 0;
	}
	ss << month;
	if (day < 10) {
		ss << 0;
	}
	ss << day;
	unsigned int date = std::stoi(ss.str());
	// TODO: write as attribute to dataset
}

void CheckPoint::WriteHolidays(const std::string& filename, const std::string& groupname)
{
	htri_t exist = H5Lexists(m_file, "Config", H5P_DEFAULT);
	if (exist <= 0) {
		hid_t temp = H5Gcreate2(m_file, "Config", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		H5Gclose(temp);
	}
	hid_t temp = m_file;

	if (groupname.size() != 0) {
		m_file = H5Gopen2(m_file, groupname.c_str(), H5P_DEFAULT);
	}
	WriteFileDSet(filename, "holidays");
	if (groupname.size() != 0) {
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
	hsize_t entries = 5 + NumOfClusterTypes();

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
		data[0][4] = (unsigned int)p.GetHealth().GetHealthStatus();

		// Cluster data
		unsigned int k = 5;
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
	hsize_t dims[1] = {dataset.size() - 1};
	hid_t dataspace = H5Screate_simple(1, dims, NULL);
	hid_t dset =
	    H5Dcreate2(group, setname.c_str(), H5T_NATIVE_CHAR, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(dset, H5T_NATIVE_CHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(*dataset.begin()));
	H5Dclose(dset);
	H5Sclose(dataspace);
	H5Gclose(group);
}

void CheckPoint::SaveCheckPoint(const Population& pop, unsigned int time)
{
	// TODO: make it multiregion
	m_lastCh++;
	if (m_lastCh == m_limit) {
		m_lastCh = 0;
		WritePopulation(pop, time);
	}
}

} /* namespace checkpoint */
} /* namespace stride */
