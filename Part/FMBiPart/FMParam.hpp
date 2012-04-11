#ifndef FM_PARAM_HPP
#define FM_PARAM_HPP

class Netlist;

/** Class for parameters */
class FMParam
{
public:
  Netlist&        _H;    /**< the netlist */
  unsigned int    _K;    /**< number of partitions */
  int             _cm;   /**< cost model */

public:
  /** Constructor */
  FMParam(Netlist& H, unsigned int K=2, int cm=1) :
    _H(H), _K(K), _cm(cm)
  {
  }
};

#endif
