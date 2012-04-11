#ifndef FM_EXCEPTION_HPP
#define FM_EXCEPTION_HPP

#include <exception>

/** @addtogroup partitioning_group
 *  @{
 */

/**
 * Exception Handling. To prevent the memory leak during an
 * exception, memory allocation
 * should be performed in constructors and memory deallocation should
 * be performed in destructors. 
 */
class FMException : public std::exception
{
public:
  /** Enum Type of Error */
  enum FMErrorType {
    FM_IMPOSSIBLE_INIT,   ///< Impossible to satisfy the balance constraints 
    FM_CANNOT_INIT,       ///< Cannot satisfy the balance constraints
    FM_INVALID_INIT,      ///< Input initial partition is invalid
    FM_EXCEED_MAX_PARTS,  ///< Exceed the maximum number of partitions
    FM_EXCEED_MAX_CELL_WEIGHT, ///< Exceed the maximum cell weight
    FM_EXCEED_MAX_NET_WEIGHT,  ///< Exceed the maximum net weight
    FM_EXCEED_MAX_DEGREE, ///< Exceed the allowed maximum degree of cells
    FM_UNKNOWN            ///< Unknown error
  };

  /** Constructor */
  FMException() throw() : _errType(FM_UNKNOWN) {}

  FMException(FMErrorType errType) throw() : _errType(errType) {}

  /** Destructor */
  virtual ~FMException() throw() {}

  /** @return the error message. */
  virtual const char* what() const throw();

private:
  FMErrorType _errType;    ///< error information 
};

/** @} */
#endif
