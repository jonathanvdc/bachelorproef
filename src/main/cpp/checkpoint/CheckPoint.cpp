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

CheckPoint::CheckPoint(const std::string& filename) : m_filename(filename) {}

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
	hid_t attr = H5Acreate2(group, "bools", H5T_NATIVE_HBOOL, dataspace, H5P_DEFAULT, H5P_DEFAULT);
	H5Awrite(attr, H5T_NATIVE_HBOOL, bools);
	H5Sclose(dataspace);
	H5Aclose(attr);

	dims = 5;
	dataspace = H5Screate_simple(1, &dims, nullptr);
	attr = H5Acreate2(group, "uints", H5T_NATIVE_UINT, dataspace, H5P_DEFAULT, H5P_DEFAULT);
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
	attr = H5Acreate2(group, "doubles", H5T_NATIVE_DOUBLE, dataspace, H5P_DEFAULT, H5P_DEFAULT);
	double doubles[3];
	doubles[0] = common_config->r0;
	doubles[1] = common_config->seeding_rate;
	doubles[2] = common_config->immunity_rate;

	H5Awrite(attr, H5T_NATIVE_DOUBLE, doubles);
	H5Sclose(dataspace);
	H5Aclose(attr);

	dims = log_config->output_prefix.size();
	dataspace = H5Screate_simple(1, &dims, nullptr);
	attr = H5Acreate2(group, "prefix", H5T_NATIVE_CHAR, dataspace, H5P_DEFAULT, H5P_DEFAULT);
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

	hsize_t dims[1] = {pop.size()};
	hid_t dataspace = H5Screate_simple(1, dims, nullptr);
	std::string spot = datestr + "/Population";

	hid_t chunkP = H5Pcreate(H5P_DATASET_CREATE);
	hsize_t chunkDims[1] = {1};
	H5Pset_chunk(chunkP, 1, chunkDims);

	hid_t newType = H5Tcreate(H5T_COMPOUND, sizeof(h_personType));

	H5Tinsert(newType, "ID", HOFFSET(h_personType, ID), H5T_NATIVE_UINT);
	H5Tinsert(newType, "Age", HOFFSET(h_personType, Age), H5T_NATIVE_DOUBLE);
	H5Tinsert(newType, "Gender", HOFFSET(h_personType, Gender), H5T_NATIVE_CHAR);
	H5Tinsert(newType, "Participating", HOFFSET(h_personType, Participating), H5T_NATIVE_HBOOL);
	H5Tinsert(newType, "Immune", HOFFSET(h_personType, Immune), H5T_NATIVE_HBOOL);
	H5Tinsert(newType, "Infected", HOFFSET(h_personType, Infected), H5T_NATIVE_HBOOL);
	H5Tinsert(newType, "StartInf", HOFFSET(h_personType, StartInf), H5T_NATIVE_UINT);
	H5Tinsert(newType, "EndInf", HOFFSET(h_personType, EndInf), H5T_NATIVE_UINT);
	H5Tinsert(newType, "StartSympt", HOFFSET(h_personType, StartSympt), H5T_NATIVE_UINT);
	H5Tinsert(newType, "EndSympt", HOFFSET(h_personType, EndSympt), H5T_NATIVE_UINT);
	H5Tinsert(newType, "TimeInfected", HOFFSET(h_personType, TimeInfected), H5T_NATIVE_UINT);
	H5Tinsert(newType, "Household", HOFFSET(h_personType, Household), H5T_NATIVE_UINT);
	H5Tinsert(newType, "School", HOFFSET(h_personType, School), H5T_NATIVE_UINT);
	H5Tinsert(newType, "Work", HOFFSET(h_personType, Work), H5T_NATIVE_UINT);
	H5Tinsert(newType, "Primary", HOFFSET(h_personType, Primary), H5T_NATIVE_UINT);
	H5Tinsert(newType, "Secondary", HOFFSET(h_personType, Secondary), H5T_NATIVE_UINT);

	hid_t dataset = H5Dcreate2(m_file, spot.c_str(), newType, dataspace, H5P_DEFAULT, chunkP, H5P_DEFAULT);

	chunkP = H5Pcreate(H5P_DATASET_XFER);
	unsigned int i = 0;

	pop.serial_for([this, &i, &newType, &dataspace, &dataset, &chunkP](const Person& p, unsigned int) {
		h_personType data(p);

		hsize_t start[1] = {i};
		hsize_t count[1] = {1};
		hid_t chunkspace = H5Screate_simple(1, count, nullptr);
		H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, start, nullptr, count, nullptr);
		H5Dwrite(dataset, newType, chunkspace, dataspace, chunkP, &data);
		i++;
		H5Sclose(chunkspace);
	});
	H5Tclose(newType);
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

	std::vector<char> dataset{std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};

	hid_t group = H5Gopen2(m_file, "Config", H5P_DEFAULT);
	hsize_t dims[1] = {dataset.size()};
	hid_t dataspace = H5Screate_simple(1, dims, nullptr);
	hid_t dset =
	    H5Dcreate2(group, setname.c_str(), H5T_NATIVE_CHAR, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(dset, H5T_NATIVE_CHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, dataset.data());

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

void CheckPoint::SaveCheckPoint(const Simulator& sim)
{
	// TODO: add airport
	WritePopulation(*sim.GetPopulation(), sim.GetDate());
	WriteClusters(sim.GetClusters(), sim.GetDate());
	auto exp = sim.GetExpatriateJournal();
	WriteExpatriates(exp, sim.GetDate());
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
	hid_t newType = H5Dget_type(dset);

	hsize_t dims;
	H5Sget_simple_extent_dims(dspace, &dims, nullptr);

	Population result;
	for (hsize_t i = 0; i < dims; i++) {
		hsize_t start = i;

		hsize_t count = 1;

		hid_t subspace = H5Screate_simple(1, &count, nullptr);

		H5Sselect_hyperslab(dspace, H5S_SELECT_SET, &start, nullptr, &count, nullptr);

		h_personType data;

		H5Dread(dset, newType, subspace, dspace, H5P_DEFAULT, &data);

		disease::Fate disease;
		disease.start_infectiousness = data.StartInf;
		disease.start_symptomatic = data.StartSympt;
		disease.end_infectiousness = data.EndInf;
		disease.end_symptomatic = data.EndSympt;

		Person toAdd(
		    data.ID, data.Age, data.Household, data.School, data.Work, data.Primary, data.Secondary, disease);

		if (data.Participating) {
			toAdd.ParticipateInSurvey();
		}

		if (data.Immune) {
			toAdd.GetHealth().SetImmune();
		}
		if (data.Infected) {
			toAdd.GetHealth().StartInfection();
		}
		for (unsigned int i = 0; i < data.TimeInfected; i++) {
			toAdd.GetHealth().Update();
		}

		result.emplace(toAdd);
		H5Sclose(subspace);
	}

	H5Sclose(dspace);
	H5Tclose(newType);
	H5Dclose(dset);

	// loading clusters
	LoadCluster(clusters.m_households, ClusterType::Household, groupname, result);
	LoadCluster(clusters.m_school_clusters, ClusterType::School, groupname, result);
	LoadCluster(clusters.m_work_clusters, ClusterType::Work, groupname, result);
	LoadCluster(clusters.m_primary_community, ClusterType::PrimaryCommunity, groupname, result);
	LoadCluster(clusters.m_secondary_community, ClusterType::SecondaryCommunity, groupname, result);

	return result;
}

void CheckPoint::LoadCluster(
    std::vector<Cluster>& clusters, const ClusterType& i, const std::string& groupname, const Population& result)
{
	std::string type = ToString(i);
	std::string path = groupname + "/" + type;
	hid_t clusterID = H5Dopen2(m_file, path.c_str(), H5P_DEFAULT);
	hid_t dspace = H5Dget_space(clusterID);
	hid_t newType = H5Dget_type(clusterID);

	hsize_t dims;
	H5Sget_simple_extent_dims(dspace, &dims, nullptr);

	hsize_t start = 0;

	hsize_t count = 1;

	hid_t subspace = H5Dget_space(clusterID);
	H5Sselect_hyperslab(subspace, H5S_SELECT_SET, &start, nullptr, &count, nullptr);
	h_clusterType data;

	H5Dread(clusterID, newType, H5S_ALL, subspace, H5P_DEFAULT, &data);

	H5Sclose(subspace);

	std::unique_ptr<Cluster> CurrentCluster = std::make_unique<Cluster>(data.ID, i);

	for (hsize_t j = 1; j < dims; j++) {
		hsize_t start = j;
		hsize_t count = 1;

		hid_t subspace = H5Screate_simple(1, &count, nullptr);

		H5Sselect_hyperslab(dspace, H5S_SELECT_SET, &start, nullptr, &count, nullptr);

		h_clusterType data;

		H5Dread(clusterID, newType, subspace, dspace, H5P_DEFAULT, &data);

		H5Sclose(subspace);

		if (data.ID != CurrentCluster->GetId()) {
			clusters.emplace_back(*CurrentCluster);
			CurrentCluster = std::make_unique<Cluster>(data.ID, i);
			continue;
		}

		unsigned int idPersonToAdd = data.PersonID;

		result.serial_for([this, &idPersonToAdd, &CurrentCluster](const Person& p, unsigned int) {
			if (p.GetId() == idPersonToAdd) {
				CurrentCluster->AddPerson(p);
			}
		});
	}

	clusters.emplace_back(*CurrentCluster);
	H5Tclose(newType);
	H5Dclose(clusterID);
	H5Sclose(dspace);
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

	std::unique_ptr<H5A_info_t> info = std::make_unique<H5A_info_t>();

	H5Aget_info(attr, info.get());
	std::vector<char> prefix(info->data_size, '\0');
	H5Aread(attr, H5T_NATIVE_CHAR, prefix.data());
	H5Aclose(attr);
	result.log_config->output_prefix = std::string(prefix.begin(), prefix.end());

	if (info->data_size == 0) {
		result.log_config->output_prefix = "";
	}

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

	WriteCluster(clusters.m_households, group, ClusterType::Household);
	WriteCluster(clusters.m_school_clusters, group, ClusterType::School);
	WriteCluster(clusters.m_work_clusters, group, ClusterType::Work);
	WriteCluster(clusters.m_primary_community, group, ClusterType::PrimaryCommunity);
	WriteCluster(clusters.m_secondary_community, group, ClusterType::SecondaryCommunity);

	H5Gclose(group);
}

void CheckPoint::WriteCluster(const std::vector<Cluster>& clvector, hid_t& group, const ClusterType& t)
{
	std::string dsetname = ToString(t);

	unsigned int totalSize = 0;
	std::vector<unsigned int> sizes;

	for (auto& cluster : clvector) {
		totalSize += cluster.GetSize() + 1;
		sizes.push_back(cluster.GetSize());
	}
	hid_t newType = H5Tcreate(H5T_COMPOUND, sizeof(h_clusterType));

	H5Tinsert(newType, "ID", HOFFSET(h_clusterType, ID), H5T_NATIVE_UINT);
	H5Tinsert(newType, "PersonID", HOFFSET(h_clusterType, PersonID), H5T_NATIVE_UINT);

	hsize_t dims = totalSize;
	hid_t dataspace = H5Screate_simple(1, &dims, nullptr);

	hid_t dataset = H5Dcreate2(group, dsetname.c_str(), newType, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	unsigned int spot = 0;
	for (auto& cluster : clvector) {
		auto people = cluster.GetPeople();

		std::vector<h_clusterType> data{cluster.GetSize() + 1};

		// Start of new cluster
		data[0].ID = cluster.GetId();
		data[0].PersonID = 0;
		// Data in cluster
		for (unsigned int i = 1; i <= people.size(); i++) {
			data[i].ID = cluster.GetId();
			data[i].PersonID = people[i - 1].GetId();
		}

		hsize_t start = spot;
		hsize_t count = cluster.GetSize() + 1;
		hid_t chunkspace = H5Screate_simple(1, &count, nullptr);
		H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, &start, nullptr, &count, nullptr);

		hid_t plist = H5Pcreate(H5P_DATASET_XFER);

		H5Dwrite(dataset, newType, chunkspace, dataspace, plist, data.data());
		spot += cluster.GetSize() + 1;
		H5Sclose(chunkspace);
		H5Pclose(plist);
	}
	H5Tclose(newType);
	H5Sclose(dataspace);
	H5Dclose(dataset);
}

void CheckPoint::WriteExpatriates(multiregion::ExpatriateJournal& journal, boost::gregorian::date date)
{
	std::string datestr = to_iso_string(date);
	htri_t exist = H5Lexists(m_file, datestr.c_str(), H5P_DEFAULT);
	if (exist <= 0) {
		hid_t temp = H5Gcreate2(m_file, datestr.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		H5Gclose(temp);
	}
	hid_t group = H5Gopen2(m_file, datestr.c_str(), H5P_DEFAULT);

	std::string dsetname = "Expatriates";

	std::vector<unsigned int> data;

	journal.SerialForeach([&data](const Person& p, unsigned int) { data.push_back(p.GetId()); });

	hsize_t dims = data.size();
	hid_t dataspace = H5Screate_simple(1, &dims, nullptr);

	hid_t dataset =
	    H5Dcreate2(group, dsetname.c_str(), H5T_NATIVE_UINT, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(dataset, H5T_NATIVE_UINT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &data.front());

	H5Sclose(dataspace);
	H5Dclose(dataset);
	H5Gclose(group);
}

} /* namespace checkpoint */
} /* namespace stride */
