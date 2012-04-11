#include "MlPartTmpl.hpp"
#include <FMParam.hpp>
#include <FMLimits.hpp>
#include <FMException.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <vector>
#include <cassert>  //added by Zhou
#include <climits>  //added by Zhou

using std::vector;

/** Constructor */
template <class PartMgrBase, class TopPartitioner, class RefinementMgr>
MlPartTmpl<PartMgrBase, TopPartitioner, RefinementMgr>::
MlPartTmpl(const FMParam& param) :
  PartMgrBase(param),
  _contractPtr(&Netlist::contractByMMC2),
  _isVCycle(false),
  _hasFixedCells(param._H.hasFixedCells()),
  _localityRatio(3),
  _relaxRatio(0)
{
}

/** Destructor */
template <class PartMgrBase, class TopPartitioner, class RefinementMgr>
MlPartTmpl<PartMgrBase, TopPartitioner, RefinementMgr>::~MlPartTmpl()
{
}

/** 
 * Coarsening Phase. Successively generate a sequence of coarse
 * hypergraphs. The process repeats until the number of cells
 * (modules) is less than or equal to the maximum pin count (degree),
 * or legal initial solution cannot be generated, or insufficient
 * memory to create further sub-netlists. Contraction methods
 * defined in Netlist such as Greedy and MMC could be used.   
 * 
 * In -noInit or VCycle mode, the clusters are restricted to be in the
 * same partition. To do that, all the interfaces nets are marked as
 * fixed so that the contraction algorithm will not select such nets.
 */ 
template <class PartMgrBase, class TopPartitioner, class RefinementMgr>
inline Netlist::SPN MlPartTmpl<PartMgrBase, TopPartitioner, RefinementMgr>::
coarseningPhase(Netlist& H, const vector<unsigned char>& part)
{
  return (H.*_contractPtr)(part); // allow parametric contraction methods
}


/** 
 * Initial Partition Phase. This phase is applied to the coarsest
 * (top level) hypergraph. In current implementation, the initial
 * partitioning solution is simply obtained from the best of three
 * runs of the FM algorithm. 
 */ 
template <class PartMgrBase, class TopPartitioner, class RefinementMgr>
void MlPartTmpl<PartMgrBase, TopPartitioner, RefinementMgr>::
topLevelInitPhase(Netlist& H, vector<unsigned char>& part,
                  unsigned int& cost, int starts, int level)
{
  const double relaxFactor = 1.0 + _relaxRatio * (level - 1);
  const FMParam 
    param(H, PartMgrBase::getNumPartitions(), PartMgrBase::getCostModel());
  TopPartitioner* tpMgr = new TopPartitioner(param);
  tpMgr->setVerbosity(PartMgrBase::getVerbosity());
  tpMgr->setBoundType(PartMgrBase::getBoundType());
  tpMgr->setAllowIllegal(PartMgrBase::allowIllegal());

  if (PartMgrBase::isNoInit()) { // only refine the solution
    tpMgr->setBalanceTol(PartMgrBase::getBalanceTol());
    tpMgr->setNoInit(H.cutCost(part, PartMgrBase::getNumPartitions()));
    tpMgr->doPartition(part, 1);
  } else {
    tpMgr->setBalanceTol(PartMgrBase::getBalanceTol()*relaxFactor);
    // tpMgr.setBalanceTol(getBalanceTol());
    if (!_hasFixedCells) part.resize(H.getNumCells());
    tpMgr->doPartition(part, starts);
  }
  cost = tpMgr->getBestCost();
  // std::cout << " Top level cost = " << cost << std::endl;

  tpMgr->initDiff(part);       // re-calc. the diff of the best sol'n
  setDiff(tpMgr->getDiff());   // set diff for the refinement phases
  delete tpMgr;
  assert(H.cutCost(part, PartMgrBase::getNumPartitions()) == cost);
}


/** 
 * Uncoarsening and Refinement Phase. The solutions are projected to
 * the finer hypergraphs and are then refined by the standard FM
 * algorithm.
 */
template <class PartMgrBase, class TopPartitioner, class RefinementMgr>
void MlPartTmpl<PartMgrBase, TopPartitioner, RefinementMgr>::
refinementPhase(Netlist& H, vector<unsigned char>& part, 
                unsigned int& cost, int level)
{
  const double relaxFactor = 1.0 + _relaxRatio * (level - 1);
  const FMParam param(H, PartMgrBase::getNumPartitions(), 
                      PartMgrBase::getCostModel());
  RefinementMgr* rfMgr = new RefinementMgr(param);
  rfMgr->setVerbosity(PartMgrBase::getVerbosity());
  rfMgr->setPValue(PartMgrBase::_pvalue);
  rfMgr->setQValue(PartMgrBase::_qvalue);
  rfMgr->setAllowIllegal(PartMgrBase::allowIllegal());

  if (PartMgrBase::isNoInit()) {
    rfMgr->setBalanceTol(PartMgrBase::getBalanceTol());
  } else {
    rfMgr->setBalanceTol(PartMgrBase::getBalanceTol()*relaxFactor);
  }
  rfMgr->setBoundType(PartMgrBase::getBoundType());
  rfMgr->setNoInit(cost); // only refine the solution

  // rfMgr.doPartition(part, 1);
  // cost = rfMgr.getBestCost();
  rfMgr->setDiff(PartMgrBase::getDiff());
  // rfMgr.printDiff();
  cost = rfMgr->doPartitionOne4(part);
  setDiff(rfMgr->getDiff()); // set for the next level
  delete rfMgr;
  assert(H.cutCost(part, PartMgrBase::getNumPartitions()) == cost); // modified by Zhou
}



/** 
 * Complete Multi-level run. Multilevel method consists of three
 * phases, namely Coarsening Phase, Initial Partition Phase (at top
 * level) and Refinement Phase. In practice, more sophisticated
 * strategies could be used here, such as V-cycle and W-cycle
 * (terminlogies borrowed from Multigrid Method in solving
 * differential equations (c.f. P. Wesseling, An Introduction to
 * Multigrid Methos, John Wiley & Sons, 1992) 
 */
template <class PartMgrBase, class TopPartitioner, class RefinementMgr>
unsigned int MlPartTmpl<PartMgrBase, TopPartitioner, RefinementMgr>::
doPartitionOne(vector<unsigned char>& part)
{
  unsigned int cost = UINT_MAX;
  std::cout << "been here" << std::endl;
  if (_isVCycle) {
    if (PartMgrBase::isNoInit()) PartMgrBase::initDiff(part);
    else {
      PartMgrBase::getNetlist().initVertexArray();
      part.resize(PartMgrBase::getNetlist().getNumCells()); // start memory allocation here
      PartMgrBase::initPart(part);
      cost = PartMgrBase::getNetlist().cutCost(part, PartMgrBase::getNumPartitions());
    }
  }

  PartMgrBase::setAllowIllegal(false);
  doRecur(PartMgrBase::getNetlist(), part, cost, 1);
  assert(PartMgrBase::getNetlist().cutCost(part, PartMgrBase::getNumPartitions()) == cost);

  // Real V-cycle
  if (!PartMgrBase::isNoInit()) {
    _isVCycle = true;
    PartMgrBase::setNoInit(cost);
    PartMgrBase::setAllowIllegal(false);
    doRecur(PartMgrBase::getNetlist(), part, cost, 1);
    _isVCycle = false;
    PartMgrBase::_noInit = false;
  }

  //xxx initDiff(part); // for diff information
  return cost;
}


/** 
 * In normal mode, refinements are done only in top-down fashion. 
 * If -noInit mode is on, initialize partition is given by callers so
 * that it will be propagated to the top level, and hence during the
 * coarsening, only cells in the same partition will be clustered. To
 * do that, nets spread in different partitions will be set to be
 * fixed, so that they will not be selected in the clustering
 * process. 
 *
 * In V-cycle mode, we calculate the bottom-up information also. It
 * may be a better choice than the normal mode when 
 *    (1) some cells are constrainted to be fixed, or 
 *    (2) initial partition has been given by users
 *
 * Questions:
 * (1) How many levels?
 * In flat FM algorithm (one level), the solutions are easy to fall
 * into the local minima. Experiments showed that vertices moved in
 * clusters could get better results. Using LIFO gain bucket is an
 * example. 
 * Some partitioners take the two-levels approach. For example
 * FLARE. Unlike the clustering algorithm in here, vertices are 
 * merged *until all the edges are in-contractable*. Unfortunately,
 * the concept of contractility applies to graph only. The extension
 * to hypergraph is not exist.
 * The algorithm in here usually takes more than ten levels for large
 * circuits. However, it is not necessarily more levels
 * better. Sometimes it is even worse. The number of levels depend on
 * the locality of the individual circuit, which unluckily hard to
 * quantified. In this implementation, we stop generating next level
 * whenever the weighted maximum degree of netlist is greater than two
 * times the total number of cells of netlist (we use the ratio of the
 * two numbers to estimate the locality). The process will also be
 * stoped if the resulting netlist exceeds our implementation limits
 * (@see FMLimits) or the program exceeds the memory limit.
 */
template <class PartMgrBase, class TopPartitioner, class RefinementMgr>
void MlPartTmpl<PartMgrBase, TopPartitioner, RefinementMgr>::
doRecur(Netlist& H, vector<unsigned char>& part, unsigned int& cost, int level)
{
  try { 
    // if (_isVCycle) refinementPhase(H, part, cost, level);
    bool ok = false;
    { // let the scope limit here
      Netlist::SPN H2; // shared point to prevent memory leak from exceptions
      if (PartMgrBase::isNoInit() || _isVCycle) H.setFixedNets(part); // mark interface nets
      else H.clrFixedAllNets(); // unmark all the nets
      H2 = coarseningPhase(H, part); // only phase 1
      vector<unsigned char> partLevel1;

      if (H2 != 0) {
        // Projection up
        if (_hasFixedCells || PartMgrBase::isNoInit() || _isVCycle) {
          partLevel1.resize(H2->getNumCells()); // start memory allocation
          H2->projectUp(H, partLevel1, part); // propagate information up
        }
        H.contractPhase2(H2);
      }

      if (H2 != 0
          // && H2->getNumCells() > 100 * getNumPartitions()
          && H2->getMaxDegree() <= _localityRatio * H2->getNumCells()
          && H2->getMaxDegree() <= FMLimits::maxDegreeOfCells()) { 
        doRecur(*H2, partLevel1, cost, level+1);
        if (!(_hasFixedCells || PartMgrBase::isNoInit() || _isVCycle)) {
          part.resize(H.getNumCells()); // start memory allocation here
        }
        H.projectDown(*H2, partLevel1, part); // propagate information down
        ok = true;
      } 
    } // let H2 out of scope (so that its memory can be released)
    H.clearMasterList(); // release memory from masterList
    if (ok) {
      refinementPhase(H, part, cost, level); // refinement phase
      return;
    }
  }
  catch (FMException& e) {
    // Discard one level in case of any errors
    if (PartMgrBase::getVerbosity() > 0) {
      std::cerr << "Warning: " << e.what() 
                << " Discard one level." << '\n';
    }
  }
  catch (std::bad_alloc) {
    if (PartMgrBase::getVerbosity() >= 0) {
      std::cerr << "Warning: Insufficient memory."
  	        << " Discard one level." << '\n';
    }
  }

  H.clearMasterList(); // release memory from masterList
  if (!_isVCycle) {
    //$$$ topLevelInitPhase(H, part, cost, 3); // top level initialization
    topLevelInitPhase(H, part, cost, level, level); // top level initialization
  }
}



/** Here, I use the template technique to generate both
    bi-partitioning and mult-way partitioning ML code. It may reflects
    the fact that "multi-level" is such a general concept (like
    simulated annealing) that can be applied to different problems.

    One myth is that template codes will make a program runs
    slower. In my experience, template code itself runs exactly as fast as you
    makes several copies manually. What make a program really slow is the
    *generality*. Template codes tends to handle more general
    problems using more general concepts. As all know, one may take
    advantage of some speciality of a particular problem and makes a
    program runs faster. In C++, you are allowed to write template
    specialization code that hopefully solve this issue mostly.

    One major problem of template technique, in my opinion, is that
    only a few C++ compilers fully support the standard template
    features. In GNU C++, the keyword "export" is not supported and I
    have to explicitly instantiate all non-inline template functions.
*/


/** Explicit Instantiation */
#include <FMPartTmpl.hpp>
#include <FMPartTmpl4.hpp>

#include <FMBiPartCore.hpp>
#include <FMBiPartCore4.hpp>
#include <FMBiGainMgr2.hpp>
#include <FMBiGainMgr3.hpp>

template class 
MlPartTmpl
< FMBiPartMgrBase, 
  FMPartTmpl<FMBiPartCore, FMBiGainMgr2>,
  FMPartTmpl<FMBiPartCore, FMBiGainMgr2> 
>;

template class 
MlPartTmpl
< FMBiPartMgrBase, 
  FMPartTmpl<FMBiPartCore, FMBiGainMgr2>,
  FMPartTmpl4<FMBiPartCore4, FMBiGainMgr2> 
>;

template class 
MlPartTmpl
< FMBiPartMgrBase, 
  FMPartTmpl<FMBiPartCore, FMBiGainMgr3>,
  FMPartTmpl4<FMBiPartCore4, FMBiGainMgr3> 
>;

template class 
MlPartTmpl
< FMBiPartMgrBase, 
  FMPartTmpl4<FMBiPartCore4, FMBiGainMgr2>,
  FMPartTmpl4<FMBiPartCore4, FMBiGainMgr2> 
>;

#include <FMBiGainMgr.hpp>

template class 
MlPartTmpl
< FMBiPartMgrBase, 
  FMPartTmpl<FMBiPartCore, FMBiGainMgr>,
  FMPartTmpl4<FMBiPartCore4, FMBiGainMgr> 
>;

#include <FMKWayPartCore.hpp>
#include <FMKWayPartCore4.hpp>
#include <FMKWayGainMgr2.hpp>
#include <FMKWayGainMgr3.hpp>

template class 
MlPartTmpl
< FMKWayPartMgrBase, 
  FMPartTmpl<FMKWayPartCore, FMKWayGainMgr2>,
  FMPartTmpl4<FMKWayPartCore4, FMKWayGainMgr2> 
>;

template class 
MlPartTmpl
< FMKWayPartMgrBase, 
  FMPartTmpl<FMKWayPartCore, FMKWayGainMgr3>,
  FMPartTmpl4<FMKWayPartCore4, FMKWayGainMgr3> 
>;

template class 
MlPartTmpl
< FMKWayPartMgrBase, 
  FMPartTmpl<FMKWayPartCore, FMKWayGainMgr2>,
  FMPartTmpl<FMKWayPartCore, FMKWayGainMgr2> 
>;


#include <FMPWPartTmpl.hpp>
// #include <FMBiGainMgr2.hpp>

template class 
MlPartTmpl
< FMKWayPartMgrBase, 
  FMPWPartTmpl<FMBiGainMgr2>,
  FMPWPartTmpl<FMBiGainMgr2> 
>;

template class 
MlPartTmpl
< FMKWayPartMgrBase, 
  FMPWPartTmpl<FMBiGainMgr2>,
  FMPartTmpl4<FMKWayPartCore4, FMKWayGainMgr2> 
>;

