/*
 * CheckPoint.h
 *
 *  Created on: Mar 26, 2017
 *      Author: cedric
 */

#ifndef CHECKPOINT_H_INCLUDED
#define CHECKPOINT_H_INCLUDED

#include <memory>
#include <vector>
#include "calendar/Calendar.h"
#include "core/Cluster.h"
#include <hdf5.h>
#include "pop/Population.h"
#include "sim/SimulationConfig.h"

namespace stride {
namespace checkpoint {

class CheckPoint
{
public:
	/// Creates the wanted file and immediately closes it. It will overwrite a file if one of the same name already
	/// exists.
	void CreateFile();

	/// Constructor The string is the file for the checkpoints. The unsigned int is the interval between
	/// checkpoints. An interval of 0 makes no checkpoints.
	CheckPoint(const std::string&, unsigned int interval = 5);

	/// Loads a checkpoint from the file in the constructor. The unsigned int tells which checkpoint to use.
	Population LoadCheckPoint(boost::gregorian::date, std::vector<std::vector<Cluster>>&);

	/// Saves the current simulation to a checkpoint with the date.
	void SaveCheckPoint(const Population&, const std::vector<std::vector<stride::Cluster>>&, boost::gregorian::date);

	/// Saves the population in a file to a checkpoint to group with the unsigned int as index
	void CombineCheckPoint(const std::string&, unsigned int);

	/// Puts the h5 for a single simulation into a seperate file
	void SplitCheckPoint(unsigned int, std::string);

	/// Writes the MultiSimulationConfig.
	void WriteConfig(const MultiSimulationConfig&);

	/// Writes the SingleSimulationConfig.
	void WriteConfig(const SingleSimulationConfig&);

	/// Loads the MultiSimulationConfig.
	MultiSimulationConfig LoadMultiConfig();

	/// Loads a SingleSimulationConfig with id the given id.
	SingleSimulationConfig LoadSingleConfig(unsigned int id = 0);

	/// Writes the holidays from a file. The int pointer represents the group.
	void WriteHolidays(const std::string&, unsigned int* group = NULL);

	/// Loads the Calendar starting with the given date.
	Calendar LoadCalendar(boost::gregorian::date);

	/// Opens the wanted file
	void OpenFile();

	/// Closes the wanted file
	void CloseFile();

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

#endif /* CHECKPOINT_H_ */
