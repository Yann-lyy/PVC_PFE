#ifndef EMP_PFE_GarG_H__
#define EMP_PFE_GarG_H__

#include "pfe/execution/circuit_execution.h"
#include <stdio.h>
#include <algorithm> 
#include <fstream>
using std::vector;
using std::pair;
using namespace std;
using namespace emp;

void construct_pfegarbledgates(const unsigned char *a, const unsigned char *b, unsigned char* garbledGates, int num_gate, int num_input){
	unsigned char* incoming_wires = new unsigned char [2*num_gate*33];
	unsigned char* outgoing_wires = new unsigned char [4*num_gate*33];
    memcpy(incoming_wires, a, 2*num_gate*33);
    memcpy(outgoing_wires, b, 4*num_gate*33);
    for(int i=0;i<num_gate;i++){
        unsigned char in[4*33],out[2*33];
        for(int j=0;j<4;j++){
            memcpy(in+j*33, outgoing_wires+(4*i+j)*33, 33);
        }
        for(int j=0;j<2;j++){
            memcpy(out+j*33, incoming_wires+(2*i+j)*33, 33);
        }
        unsigned char *gct= new unsigned char[5*33];
        CircuitExecution::circ_exec->nand_gate(i,in,out,gct);
        memcpy(garbledGates+5*i*33, gct, 5*33);
    }
}

#endif// CIRCUIT_FILE