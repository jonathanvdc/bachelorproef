#ifndef POPULATION_MODEL_H_INCLUDED
#define POPULATION_MODEL_H_INCLUDED

/**
 * @file
 * Header file for the population model class
 */

#include <vector>
#include <boost/property_tree/ptree.hpp>

namespace stride {

/**
 * Data from which populations may be generated.
 */
struct PopulationModel
{
    PopulationModel(const boost::property_tree::ptree& pt);

    /////////////////////////////////////////////////////////////////
    // Age parameters
    /////////////////////////////////////////////////////////////////

    // The age distribution is a piecewise linear curve connecting
    // (0, h), (elbow_age, h), and (maximum_age, 0), where h is computed
    // so that the area under the curve is 1.
    int maximum_age;
    int elbow_age;

    /////////////////////////////////////////////////////////////////
    // Family parameters
    /////////////////////////////////////////////////////////////////

    // Minimum age for members of families of size <= 2.
    int live_alone_minimum_age;

    // Maximum age for children in families of size >= 3.
    int child_maximum_age;

    // Minimum age for parents, in families of size >= 3.
    int parent_minimum_age;

    // Maximum age for parents, in families of size >= 3.
    int parent_maximum_age;

    // Minimum age gap between children and parents.
    int child_parent_minimum_age_gap;

    // Inter-parent age gap range.
    int parent_minimum_age_gap;
    int parent_maximum_age_gap;

    // Inter-child age gap range.
    int child_minimum_age_gap;
    int child_maximum_age_gap;

    // Family size probabilities (relative to sum of vector).
    // family_size_distribution[k] is the relative probability
    // of a family having (k + 1) members.
    std::vector<int> size_distribution;

    /////////////////////////////////////////////////////////////////
    // School parameters
    /////////////////////////////////////////////////////////////////

    // The age at which people start going to kindergarten.
    int kindergarten_age;

    // The age at which people start going to primary school.
    int primary_school_age;

    // The age at which people start going to secondary school.
    int secondary_school_age;

    // The age at which people start higher education (or leave school).
    int higher_eduation_age;

    // The age at which people graduate from higher education.
    int graduation_age;

    // The percentage of people who go into higher education.
    double p_higher_education;

    // The size range for schools (in # of students).
    int school_minimum_size;
    int school_maximum_size;

    /////////////////////////////////////////////////////////////////
    // Work parameters
    /////////////////////////////////////////////////////////////////

    // The age range for employees.
    int work_minimum_age;
    int work_maximum_age;

    // The percentage of people, within this age range, that is employed.
    double p_employed;

    // The size range for work sites (in # of employees). The distribution is linear.
    int work_minimum_size;
    int work_maximum_size;

    /////////////////////////////////////////////////////////////////
    // Community parameters
    /////////////////////////////////////////////////////////////////

    // The size range for communities (in # of members). The distribution is linear.
    int community_minimum_size;
    int community_maximum_size;
};

}

#endif
