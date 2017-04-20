/*
 * CheckPoint.h
 *
 *  Created on: Mar 26, 2017
 *      Author: cedric
 */

#ifndef CHECKPOINT_H_INCLUDED
#define CHECKPOINT_H_INCLUDED

#include <memory>
#include "calendar/Calendar.h"
#include "hdf5.h"
#include "pop/Population.h"
#include "sim/SimulationConfig.h"

namespace stride {
namespace checkpoint {

class CheckPoint
{
public:
	/// Constructor The string is the file for the checkpoints.
	CheckPoint(const std::string&);

	/// Loads a checkpoint from the file in the constructor. The unsigned int tells which checkpoint to use.
	void LoadCheckPoint(unsigned int);

	/// Saves the current simulation to a checkpoint.
	void SaveCheckPoint(const Population&, unsigned int);

	/// Writes the MultiSimulationConfig.
	void WriteConfig(const MultiSimulationConfig&);

	/// Loads the MultiSimulationConfig.
	MultiSimulationConfig LoadConfig();

	/// Writes the SingleSimulationConfig.
	void WriteConfig(const SingleSimulationConfig&);

	/// Writes the holidays from a file. The second string represents the groupname.
	void WriteHolidays(const std::string&, const std::string& groupname = "");

	/// Loads the holidays into a std::string.
	std::string LoadHolidays();

	/// Creates the wanted file and immediately closes it. It will overwrite a file if one of the same name already
	/// exists.
	void CreateFile();

	/// Opens the wanted file
	void OpenFile();

	/// Closes the wanted file
	void CloseFile();

private:
	/// Loads the disease into a boost::property_tree::ptree.
	std::string LoadDisease();

	/// Writes the current date to the checkpoint.
	void WriteDate(const Calendar&);

	/// Loads the current date from a checkpoint.
	boost::gregorian::date LoadDate();

	/// Writes the current population to a checkpoint.
	void WritePopulation(const Population&, unsigned int);

	/// Loads the asked population from the given checkpoint.
	Population LoadPopulation(unsigned int);

	/// Writes the PopulationMatrix
	void WritePopulationConfig(const std::string&);

	/// Writes a file to a dataset. The first string is the filename in the data folder. The second string is the
	/// name of the dataset
	void WriteFileDSet(const std::string&, const std::string&);

	/// Returns a file in the form of a string. The argument is the name of dataset
	std::string LoadFileDSet(const std::string&);

	hid_t m_file;		      //< current hdf5 workspace
	const std::string m_filename; //< filename
};

} /* namespace checkpoint */
} /* namespace stride */

#endif /* CHECKPOINT_H_ */
