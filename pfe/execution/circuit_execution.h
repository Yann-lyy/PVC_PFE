#ifndef EMP_CIRCUIT_EXECUTION_H__
#define EMP_CIRCUIT_EXECUTION_H__
#include "pfe/utils/block.h"
#include "pfe/utils/constants.h"

namespace emp {

/* Circuit Pipelining 
 * [REF] Implementation of "Faster Secure Two-Party Computation Using Garbled Circuit"
 * https://www.usenix.org/legacy/event/sec11/tech/full_papers/Huang.pdf
 */
class CircuitExecution { public:
#ifndef THREADING
	static CircuitExecution * circ_exec;
#else
	static __thread CircuitExecution * circ_exec;
#endif
	virtual void nand_gate(int gateid, unsigned char* in1, unsigned char* in2, unsigned char* gg) = 0;
	virtual ~CircuitExecution (){ }
};
}
#endif