#ifndef CHECKPOINT_H_INCLUDED
#define CHECKPOINT_H_INCLUDED

#include <memory>
#include <vector>
#include <hdf5.h>
#include "calendar/Calendar.h"
#include "core/Cluster.h"
#include "pop/Population.h"
#include "sim/SimulationConfig.h"

namespace stride {
namespace checkpoint {

class CheckPoint
{
public:
	/// Constructor The string is the file for the checkpoints. The unsigned int is the interval between
	/// checkpoints. An interval of 0 makes no checkpoints.
	CheckPoint(const std::string& filename, unsigned int interval = 5);

	/// Creates the wanted file and immediately closes it. It will overwrite a file if one of the same name already
	/// exists.
	void CreateFile();

	/// Opens the wanted file. This is necessary for any of the following functions.
	void OpenFile();

	/// Closes the wanted file. This is necessary for any of the following functions.
	void CloseFile();

	/// Writes the MultiSimulationConfig in different simulation groups if necessary.
	void WriteConfig(const MultiSimulationConfig& conf);

	/// Writes the SingleSimulationConfig.
	void WriteConfig(const SingleSimulationConfig& conf);

	/// Loads a checkpoint from the file in the constructor. The unsigned date tells which checkpoint to use. The
	/// clusters will be made with each inner list of clusters all having the same clustertype.
	Population LoadCheckPoint(boost::gregorian::date date, std::vector<std::vector<Cluster>>& clusters);

	/// Saves the current simulation to a checkpoint with the date as Identifier.
	void SaveCheckPoint(
	    const Population& pop, const std::vector<std::vector<stride::Cluster>>& clusters,
	    boost::gregorian::date date);

	/// Copies the info in the filename under the data of the given simulation
	void CombineCheckPoint(unsigned int simulation, const std::string& filename);

	/// Copies the info of the asked simulation into the given file
	void SplitCheckPoint(unsigned int simulation, const std::string& filename);

	/// Loads a SingleSimulationConfig.
	SingleSimulationConfig LoadSingleConfig();

	/// Writes the holidays from a file. The int pointer represents the group.
	void WriteHolidays(const std::string& filename, unsigned int* simulation = NULL);

	/// Loads the Calendar starting with the given date.
	Calendar LoadCalendar(boost::gregorian::date date);

private:
	/// Writes the current population to a checkpoint.
	void WritePopulation(const Population&, boost::gregorian::date);

	/// Writes the clusters to a checkpoint.
	void WriteClusters(const std::vector<std::vector<Cluster>>&, boost::gregorian::date);

	/// Writes a file to a dataset. The first string is the filename in the data folder. The second string is the
	/// name of the dataset
	void WriteFileDSet(const std::string&, const std::string&);

	/// Writes a dataset to a file. The first string is the filename in the data folder. The second string is the
	/// name of the dataset
	void WriteDSetFile(const std::string&, const std::string&);

	hid_t m_file;		      //< current hdf5 workspace
	const std::string m_filename; //< filename
	unsigned int m_limit;	 //< the amount of steps before a checkpoint
	unsigned int m_lastCh = 0;    //< the amoint of steps since the last checkpoint
};

} /* namespace checkpoint */
} /* namespace stride */

#endif /* CHECKPOINT_H_INCLUDED */
