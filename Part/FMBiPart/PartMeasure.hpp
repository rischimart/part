#ifndef PART_MEASURE_HPP
#define PART_MEASURE_HPP
#include <climits>  //added by Zhou
#include <climits>  //added by Zhou

class PartMeasure
{
private:
  unsigned int _cost;
  int _diff;

public:
  PartMeasure() : _cost(UINT_MAX), _diff(INT_MAX) {}
  PartMeasure(unsigned int cost, int diff) : _cost(cost), _diff(diff) {}

  unsigned int getCost() const { return _cost; }
  int getDiff() const { return _diff; }
  void setCost(unsigned int cost) { _cost = cost; }
  void setDiff(int diff) { _diff = diff; }
};

inline bool operator<(const PartMeasure& lhs, const PartMeasure& rhs)
{
  return lhs.getCost() < rhs.getCost() ||
    lhs.getCost() == rhs.getCost() && lhs.getDiff() < rhs.getDiff() ;
}

#endif
