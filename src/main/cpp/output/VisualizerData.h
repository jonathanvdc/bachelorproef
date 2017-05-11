#ifndef VISUALIZERDATA_H_INCLUDED
#define VISUALIZERDATA_H_INCLUDED

/**
 * @file
 * Data structure for the visualiser.
 */

#include <memory>
#include <string>
#include <vector>

using namespace std;

class VisualizerData
{
public:
private:
	struct LocationTuple
	{
		string name;
		int inhabitants;
		int infected;
	};
	vector<vector<LocationTuple>> days;
};

#endif // end-of-include-guard