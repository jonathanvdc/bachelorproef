/*
 * Disease.h
 *
 *  Created on: Dec 18, 2016
 *      Author: elise
 */

#ifndef SRC_MAIN_CPP_CORE_DISEASE_H_
#define SRC_MAIN_CPP_CORE_DISEASE_H_

namespace stride {

/*
 *
 */
enum class DiseaseStatus {Susceptible = 0U, Exposed = 1U, Infectious = 2U, Symptomatic = 3U, InfectiousAndSymptomatic = 4U, Recovered = 5U, Immune = 6U, Null};

/*
 *
 */
class Disease {
public:
	///
	Disease(unsigned int start_infectiousness, unsigned int start_symptomatic, unsigned int time_infectious, unsigned int time_symptomatic);

	///
	virtual ~Disease();

	///
	DiseaseStatus GetDiseaseStatus() const { return m_disease_status; }

	///
	unsigned int GetEndInfectiousness() const { return m_end_infectiousness; }

	///
	unsigned int GetEndSymptomatic() const { return m_end_symptomatic; }

	///
	unsigned int GetStartInfectiousness() const { return m_start_infectiousness; }

	///
	unsigned int GetStartSymptomatic() const { return m_start_symptomatic; }

	/// Set immune to true.
	void SetImmune();

	/// Start the infection.
	void StartInfection();

	/// Stop the infection.
	void StopInfection();

	/// Update progress of the disease.
	void Update();

private:
	/// Get the disease counter.
	unsigned int GetDiseaseCounter() const { return m_disease_counter; }

	/// Increment disease counter.
	void IncrementDiseaseCounter() { m_disease_counter++; }

	/// Reset the disease counter.
	void ResetDiseaseCounter() { m_disease_counter = 0U; }

private:
	DiseaseStatus		m_disease_status;			///< The current status of the person w.r.t. the disease.

	unsigned int		m_disease_counter;			///< The disease counter.

	unsigned int		m_start_infectiousness;		///< Days after infection to become infectious.
	unsigned int		m_start_symptomatic; 		///< Days after infection to become symptomatic.
	unsigned int		m_end_infectiousness;		///< Days after infection to end infectious state.
	unsigned int		m_end_symptomatic;			///< Days after infection to end symptomatic state.

};

} /* namespace stride */


#endif /* SRC_MAIN_CPP_CORE_DISEASE_H_ */
