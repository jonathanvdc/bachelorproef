#include <hdf5.h>
#include "CheckPoint.h"

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

CheckPoint::CheckPoint(const std::string& filename, unsigned int interval)
    : m_filename(filename), m_limit(interval), m_lastCh(interval - 1)
{
}

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
	hid_t dataspace = H5Screate_simple(1, &dims, nullptr);
	hbool_t bools[2];
	bools[0] = common_config->track_index_case;
	bools[1] = log_config->generate_person_file;
	hid_t attr = H5Acreate2(group, "bools", H5T_IEEE_F64LE, dataspace, H5P_DEFAULT, H5P_DEFAULT);
	H5Awrite(attr, H5T_NATIVE_HBOOL, bools);
	H5Sclose(dataspace);
	H5Aclose(attr);

	dims = 5;
	dataspace = H5Screate_simple(1, &dims, nullptr);
	attr = H5Acreate2(group, "uints", H5T_IEEE_F64LE, dataspace, H5P_DEFAULT, H5P_DEFAULT);
	unsigned int uints[5];
	uints[0] = common_config->rng_seed;
	uints[1] = common_config->number_of_days;
	uints[2] = common_config->number_of_survey_participants;
	uints[3] = (unsigned int)log_config->log_level;
	uints[4] = conf.GetId();
	H5Awrite(attr, H5T_NATIVE_UINT, uints);
	H5Sclose(dataspace);
	H5Aclose(attr);

	dims = 3;
	dataspace = H5Screate_simple(1, &dims, nullptr);
	attr = H5Acreate2(group, "doubles", H5T_IEEE_F64LE, dataspace, H5P_DEFAULT, H5P_DEFAULT);
	double doubles[3];
	doubles[0] = common_config->r0;
	doubles[1] = common_config->seeding_rate;
	doubles[2] = common_config->immunity_rate;

	H5Awrite(attr, H5T_NATIVE_DOUBLE, doubles);
	H5Sclose(dataspace);
	H5Aclose(attr);

	dims = log_config->output_prefix.size();
	dataspace = H5Screate_simple(1, &dims, nullptr);
	attr = H5Acreate2(group, "prefix", H5T_IEEE_F64LE, dataspace, H5P_DEFAULT, H5P_DEFAULT);
	H5Awrite(attr, H5T_NATIVE_CHAR, log_config->output_prefix.c_str());
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
	if (group != nullptr) {
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
	if (group != nullptr) {
		H5Gclose(m_file);
		m_file = temp;
	}
}

void CheckPoint::WritePopulation(const Population& pop, boost::gregorian::date date)
{
	std::string datestr = to_iso_string(date);
	htri_t exist = H5Lexists(m_file, datestr.c_str(), H5P_DEFAULT);
	if (exist <= 0) {
		hid_t temp = H5Gcreate2(m_file, datestr.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		H5Gclose(temp);
	}
	hsize_t entries = 10 + NumOfClusterTypes();

	hsize_t dims[2] = {pop.size(), entries};
	hid_t dataspace = H5Screate_simple(2, dims, nullptr);
	std::string spot = datestr + "/Population";

	hid_t chunkP = H5Pcreate(H5P_DATASET_CREATE);
	hsize_t chunkDims[2] = {1, entries};
	H5Pset_chunk(chunkP, 2, chunkDims);

	hid_t dataset = H5Dcreate2(m_file, spot.c_str(), H5T_NATIVE_UINT, dataspace, H5P_DEFAULT, chunkP, H5P_DEFAULT);

	chunkP = H5Pcreate(H5P_DATASET_XFER);
	unsigned int i = 0;
	pop.serial_for([this, &entries, &i, &dataspace, &dataset, &chunkP](const Person& p, unsigned int) {
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
		hid_t chunkspace = H5Screate_simple(2, count, nullptr);
		H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, start, nullptr, count, nullptr);
		H5Dwrite(dataset, H5T_NATIVE_UINT, chunkspace, dataspace, chunkP, data);
		i++;
		H5Sclose(chunkspace);
	});

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
	hid_t dataspace = H5Screate_simple(1, dims, nullptr);
	hid_t dset =
	    H5Dcreate2(group, setname.c_str(), H5T_NATIVE_CHAR, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(dset, H5T_NATIVE_CHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(*dataset.begin()));

	H5Dclose(dset);
	H5Sclose(dataspace);
	H5Gclose(group);
}

void CheckPoint::WriteDSetFile(const std::string& filestr, const std::string& setname)
{

	htri_t exist = H5Lexists(m_file, setname.c_str(), H5P_DEFAULT);
	if (exist <= 0) {
		FATAL_ERROR("Invalid file");
	}

	auto path = util::InstallDirs::GetDataDir();

	boost::filesystem::path filename(filestr);
	std::ofstream out((path / filename).string(), std::ios::out | std::ios::trunc);
	/*
	Piece of code based on code on
	https://lists.hdfgroup.org/pipermail/hdf-forum_lists.hdfgroup.org/2010-June/003208.html
	*/
	std::vector<char> data;
	hid_t did;
	herr_t err = 0;
	hid_t spaceId;
	hid_t dataType = H5T_NATIVE_CHAR;

	// std::cout << "HDF5 Data Type: " <<	H5Lite::HDFTypeForPrimitiveAsStr(test) << std::endl;
	/* Open the dataset. */
	// std::cout << "  Opening " << dsetName << " for data Retrieval.  "<< std::endl;
	did = H5Dopen(m_file, setname.c_str(), H5P_DEFAULT);
	if (did < 0) {
		std::cout << " Error opening Dataset: " << did << std::endl;
		return;
	}
	if (did >= 0) {
		spaceId = H5Dget_space(did);
		if (spaceId > 0) {
			int32_t rank = H5Sget_simple_extent_ndims(spaceId);
			if (rank > 0) {
				std::vector<hsize_t> dims;
				dims.resize(rank); // Allocate enough room for the dims
				rank = H5Sget_simple_extent_dims(spaceId, &(dims.front()), NULL);
				hsize_t numElements = 1;
				for (std::vector<hsize_t>::iterator iter = dims.begin(); iter < dims.end(); ++iter) {
					numElements = numElements * (*iter);
				}
				// std::cout << "NumElements: " << numElements << std::endl;
				// Resize the vector
				data.resize(static_cast<int>(numElements));
				// for (uint32_t i = 0; i<numElements; ++i) { data[i] = 55555555;}
				err = H5Dread(did, dataType, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(data.front()));
				if (err < 0) {
					std::cout << "Error Reading Data." << std::endl;
				}
			}
			err = H5Sclose(spaceId);
			if (err < 0) {
				std::cout << "Error Closing Data Space" << std::endl;
			}
		} else {
			std::cout << "Error Opening SpaceID" << std::endl;
		}
		err = H5Dclose(did);
		if (err < 0) {
			std::cout << "Error Closing Dataset" << std::endl;
		}
		for (auto& c : data) {
			out << c;
		}
	}
}

void CheckPoint::SaveCheckPoint(
    const Population& pop, const ClusterStruct& clusters, boost::gregorian::date time)
{
	// TODO: add airport
	m_lastCh++;
	if (m_lastCh == m_limit) {
		m_lastCh = 0;
		WritePopulation(pop, time);
		WriteClusters(clusters, time);
	}
}

void CheckPoint::CombineCheckPoint(unsigned int groupnum, const std::string& filename)
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

	hid_t newFile = H5Fopen(filename.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);

	auto op_func = [&group](hid_t loc_id, const char* name, const H5L_info_t* info, void* operator_data) {
		H5Ocopy(loc_id, name, group, name, H5P_DEFAULT, H5P_DEFAULT);
		return 0;
	};
	auto temp = [](hid_t loc_id, const char* name, const H5L_info_t* info, void* operator_data) {
		(*static_cast<decltype(op_func)*>(operator_data))(loc_id, name, info, operator_data);
		return 0;
	};

	H5Literate(newFile, H5_INDEX_NAME, H5_ITER_NATIVE, nullptr, temp, &op_func);

	H5Fclose(newFile);
	H5Gclose(group);
}

Population CheckPoint::LoadCheckPoint(boost::gregorian::date date, ClusterStruct& clusters)
{

	std::string groupname = to_iso_string(date);
	std::string name = groupname + "/Population";
	htri_t exist = H5Lexists(m_file, name.c_str(), H5P_DEFAULT);
	if (exist <= 0) {
		FATAL_ERROR("Incorrect date loaded");
	}
	// loading people
	hid_t dset = H5Dopen(m_file, name.c_str(), H5P_DEFAULT);
	hid_t dspace = H5Dget_space(dset);

	hsize_t dims[2];
	H5Sget_simple_extent_dims(dspace, dims, nullptr);

	Population result;

	for (hsize_t i = 0; i < dims[0]; i++) {
		hsize_t start[2];
		start[0] = i;
		start[1] = 0;

		hsize_t count[2];
		count[0] = 0;
		count[1] = dims[1];

		hid_t subspace = H5Dget_space(dset);

		H5Sselect_hyperslab(subspace, H5S_SELECT_SET, start, nullptr, count, nullptr);

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
	/*
	// loading clusters
	for (unsigned int i = 0; i < NumOfClusterTypes(); i++) {
		std::string type = ToString((ClusterType)i);
		std::string path = groupname + "/" + type;
		hid_t clusterID = H5Dopen2(m_file, path.c_str(), H5P_DEFAULT);
		hid_t dspace = H5Dget_space(dset);
		std::vector<Cluster> typeClusters;

		hsize_t dims[2];
		H5Sget_simple_extent_dims(dspace, dims, nullptr);

		hsize_t start[2];
		start[0] = 0;
		start[1] = 0;

		hsize_t count[2];
		count[0] = 0;
		count[1] = dims[1];

		hid_t subspace = H5Dget_space(clusterID);
		H5Sselect_hyperslab(subspace, H5S_SELECT_SET, start, nullptr, count, nullptr);
		unsigned int data[1][dims[1]];

		H5Dread(clusterID, H5T_NATIVE_UINT, H5S_ALL, subspace, H5P_DEFAULT, data[0]);

		H5Sclose(subspace);

		Cluster *CurrentCluster = new Cluster(*data[0], (ClusterType)i);

		for (hsize_t j = 1; j < dims[0]; j++) {
			hsize_t start[2];
			start[0] = j;
			start[1] = 0;

			hsize_t count[2];
			count[0] = 0;
			count[1] = dims[1];

			hid_t subspace = H5Dget_space(clusterID);

			H5Sselect_hyperslab(subspace, H5S_SELECT_SET, start, nullptr, count, nullptr);

			unsigned int data[1][dims[1]];

			H5Dread(clusterID, H5T_NATIVE_UINT, H5S_ALL, subspace, H5P_DEFAULT, data[0]);

			H5Sclose(subspace);

			if (*data[0] != CurrentCluster->GetId()) {
				typeClusters.push_back(*CurrentCluster);
				CurrentCluster = new Cluster(*data[0], (ClusterType)i);
				continue;
			}

			unsigned int idPersonToAdd = *data[1];

			result.serial_for(
			    [this, &idPersonToAdd, &CurrentCluster](const Person& p, unsigned int) {
				    if(p.GetId() == idPersonToAdd){
					CurrentCluster->AddPerson(p);
				    }
			    });
		}

		typeClusters.push_back(*CurrentCluster);
		clusters.push_back(typeClusters);
		H5Dclose(clusterID);
		H5Sclose(dspace);
	}
	*/
	return result;
}

SingleSimulationConfig CheckPoint::LoadSingleConfig()
{
	htri_t exist = H5Lexists(m_file, "Config", H5P_DEFAULT);
	if (exist <= 0) {
		FATAL_ERROR("Invalid file");
	}

	SingleSimulationConfig result;
	auto comCon = std::make_shared<CommonSimulationConfig>();
	auto logCon = std::make_shared<LogConfig>();
	result.common_config = comCon;
	result.log_config = logCon;

	WriteDSetFile("tmp_matrix.xml", "Config/disease");
	result.common_config->contact_matrix_file_name = "tmp_matrix.xml";

	WriteDSetFile("tmp_disease.xml", "Config/contact");
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
	unsigned int id = uints[4];

	attr = H5Aopen(group, "prefix", H5P_DEFAULT);

	H5A_info_t* info = new H5A_info_t();

	H5Aget_info(attr, info);

	char prefix[info->data_size];

	H5Aread(attr, H5T_NATIVE_CHAR, prefix);
	H5Aclose(attr);

	result.log_config->output_prefix = prefix;

	if(info->data_size == 0){
		result.log_config->output_prefix = "";
	}

	delete info;

	// travel model

	WriteDSetFile("tmp_pop.csv", "Config/popconfig");
	std::string popconfig = "tmp_pop.csv";
	std::string geoconfig = "";

	exist = H5Lexists(m_file, "Config/geoconfig", H5P_DEFAULT);
	if (exist > 0) {
		WriteDSetFile("tmp_geoconfig.xml", "Config/geoconfig");
		geoconfig = "tmp_geoconfig.xml";
	}
	std::string household = "";
	exist = H5Lexists(m_file, "Config/household", H5P_DEFAULT);
	if (exist > 0) {
		WriteDSetFile("tmp_household.xml", "Config/household");
		household = "tmp_household.xml";
	}
	auto travelRef = make_shared<multiregion::RegionTravel>(id, popconfig, geoconfig, household);
	result.travel_model = travelRef;

	return result;
}

void CheckPoint::SplitCheckPoint(unsigned int groupnum, const std::string& filename)
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

	auto op_func = [&f](hid_t loc_id, const char* name, const H5L_info_t* info, void* operator_data) {
		H5Ocopy(loc_id, name, f, name, H5P_DEFAULT, H5P_DEFAULT);
		return 0;
	};
	auto temp = [](hid_t loc_id, const char* name, const H5L_info_t* info, void* operator_data) {
		(*static_cast<decltype(op_func)*>(operator_data))(loc_id, name, info, operator_data);
		return 0;
	};

	H5Literate(group, H5_INDEX_NAME, H5_ITER_NATIVE, nullptr, temp, &op_func);

	H5Fclose(f);
	H5Gclose(group);
}

Calendar CheckPoint::LoadCalendar(boost::gregorian::date date)
{
	htri_t exist = H5Lexists(m_file, "Config", H5P_DEFAULT);
	if (exist <= 0) {
		FATAL_ERROR("Invalid file");
	}

	WriteDSetFile("tmp_holidays.json", "Config/holidays");

	Calendar result;
	result.Initialize(date, "tmp_holidays.json");
	boost::filesystem::remove("tmp_holidays.json");
	return result;
}

void CheckPoint::WriteClusters(const ClusterStruct& clusters, boost::gregorian::date date)
{
	
	std::string datestr = to_iso_string(date);
	htri_t exist = H5Lexists(m_file, datestr.c_str(), H5P_DEFAULT);
	if (exist <= 0) {
		hid_t temp = H5Gcreate2(m_file, datestr.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		H5Gclose(temp);
	}
	hid_t group = H5Gopen2(m_file, datestr.c_str(), H5P_DEFAULT);

	std::vector<int> clusterVector = {0,1,2,3,4};
	for (auto& type : clusterVector) {

		const std::vector<Cluster> *clvector;
		switch(type){
			case 0:
				clvector = &clusters.m_households;
				break;
			case 1:
				clvector = &clusters.m_school_clusters;
				break;
			case 2:
				clvector = &clusters.m_work_clusters;
				break;
			case 3:
				clvector = &clusters.m_primary_community;
				break;
			case 4:
				clvector = &clusters.m_secondary_community;
				break;
		}

		std::string dsetname = ToString(clvector->front().GetClusterType());

		unsigned int totalSize = 0;
		std::vector<unsigned int> sizes;

		for (auto& cluster : *(clvector)) {
			totalSize += cluster.GetSize() + 1;
			sizes.push_back(cluster.GetSize());
		}

		hsize_t dims[2] = {totalSize, 2};
		hid_t dataspace = H5Screate_simple(2, dims, nullptr);

		hid_t dataset = H5Dcreate2(
		    group, dsetname.c_str(), H5T_NATIVE_UINT, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		unsigned int spot = 0;
		for (auto cluster = clvector->begin(); cluster < clvector->end(); cluster++) {
			auto people = cluster->GetPeople();

			unsigned int data[cluster->GetSize() + 1][2];

			// Start of new cluster
			data[0][0] = cluster->GetId();
			data[0][1] = 0;
			// Data in cluster
			for (unsigned int i = 1; i <= people.size(); i++) {
				data[i][0] = cluster->GetId();
				data[i][1] = people[i - 1].GetId();
			}

			hsize_t start[2] = {spot, 0};
			hsize_t count[2] = {cluster->GetSize() + 1, 2};
			hid_t chunkspace = H5Screate_simple(2, count, nullptr);
			H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, start, nullptr, count, nullptr);

			hid_t plist = H5Pcreate(H5P_DATASET_XFER);

			H5Dwrite(dataset, H5T_NATIVE_UINT, chunkspace, dataspace, plist, data);
			spot += cluster->GetSize() + 1;
			H5Sclose(chunkspace);
			H5Pclose(plist);
		}
		H5Sclose(dataspace);
		H5Dclose(dataset);
	}

}

} /* namespace checkpoint */
} /* namespace stride */
