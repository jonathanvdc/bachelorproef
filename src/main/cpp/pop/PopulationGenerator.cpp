#include <memory>
#include <vector>
#include "Population.h"
#include "PopulationGenerator.h"

namespace stride {
namespace population_model {

std::unique_ptr<Population> Generator::generate() {
    return std::make_unique<Population>();
}

} // namespace population_model
} // namespace stride
