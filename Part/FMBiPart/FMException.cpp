#include "FMException.hpp"

const char* FMException::what() const throw()
{
  switch (_errType) {

  case FM_IMPOSSIBLE_INIT:
    // The feasible solution does not exist. Properly because the
    // input is too ridiculous, or the netlist is too coarse during
    // the coarsening process.
    return "Impossible to satisfy the balance constraints.";

  case FM_CANNOT_INIT:
    // The feasible solution may exist. Nevertheless, the algorithm
    // cannot find such solution.
    return "Cannot initalize in order to satisfy the constraints.";

  case FM_INVALID_INIT:
    // Initial solution is supplied by the caller. However, it does
    // not satisfy the (balance) constraints. 
    return "Input initial partition cannot satify the constraints.";

  case FM_EXCEED_MAX_PARTS:
    return "Exceed the maximum number of partitions.";

  case FM_EXCEED_MAX_CELL_WEIGHT:
    return "Exceed the maximum cell weight.";

  case FM_EXCEED_MAX_NET_WEIGHT:
    return "Exceed the maximum net weight.";

  case FM_EXCEED_MAX_DEGREE:
    return "Exceed the maximum weighted degree of cells.";

  case FM_UNKNOWN:
    return "Unknown error.";
  }

  return "";   // make compiler happy
}
