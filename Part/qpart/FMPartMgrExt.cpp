#include <FMPartMgrBase.hpp>
#include <Netlist.hpp>
#include <qwidget.h>
#include <qprogressdialog.h>
#include <climits>  //added by Zhou

using std::vector;

int FMPartMgrBase::doPartitionQt(std::vector<unsigned char>& part, 
                                 unsigned int starts)
{
  QProgressDialog pdialog("Running partitioner...", "Stop", starts,
                          0, "pdialog", TRUE);
  pdialog.show();

  vector<unsigned char> ttpart(part);
  int bestDiff = INT_MAX;
  _bestCost = INT_MAX;
  _avgCost = 0.0;
  unsigned int i;

  if (_noInit && _initCost == INT_MAX) {
    _initCost = cutCost(part);
  }

  initBase();

  for (i=0; i<starts; i++) {
    pdialog.setProgress(i);

    unsigned int curCost = doPartitionOne(ttpart);
    if (curCost < _bestCost) {
      part = ttpart;
      _bestCost = curCost;
      bestDiff = _maxdiff;
    }
    if (pdialog.wasCancelled()) break;
    _avgCost += curCost;
  }

  _maxdiff = bestDiff;
  _avgCost = _avgCost / starts;

  return _bestCost;
}

