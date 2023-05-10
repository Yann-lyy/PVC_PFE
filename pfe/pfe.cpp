#include "pfe/execution/circuit_execution.h"

#ifndef THREADING
emp::CircuitExecution* emp::CircuitExecution::circ_exec = nullptr;
#else
__thread emp::CircuitExecution* emp::CircuitExecution::circ_exec = nullptr;
#endif
