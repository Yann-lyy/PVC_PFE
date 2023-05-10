#ifndef EMP_GC_EVA_H__
#define EMP_GC_EVA_H__
#include "pfe/utils/utils.h"
#include "pfe/utils/mitccrh.h"
#include "pfe/execution/circuit_execution.h"
#include <iostream>
#include "pfe/gc/GarbledCircuits_gen.h"
namespace emp {

class GarbledCircuitsEva:public CircuitExecution {
public:
	int* point;
	block gct[4];
	void nand_gate(int gateid, unsigned char* in, unsigned char* gg, unsigned char* out) override {
		//input a and input b 
		Hash hash;
		unsigned char *input= new unsigned char[3*33]();
		int bout[33*8];
		memcpy(input, in, 2*33*sizeof(char));
		unsigned char cgateid[4];
		cgateid[0] = gateid%256;
		cgateid[1] = gateid/256%256;
		cgateid[2] = gateid/256/256%256;
		cgateid[3] = gateid/256/256/256%256;
		memcpy(input+2*33,cgateid,sizeof(char)*4);
		cout<<"???"<<endl;
		char tmp[64];
		hash.hash_once(tmp,input,3*33*sizeof(char));
		hash.hash_once(tmp+32,tmp,32*sizeof(char));
		memcpy(out, tmp, sizeof(char)*33);
		for(int i=0;i<33;i++){
			for(int k=7;k>=0;k--){
				bout[i*8+7-k] = (out[i] >> k) & 1;
			}
		}
		int ii = gg[4*33]*256+gg[4*33+1];
		int jj = gg[4*33+2]*256+gg[4*33+3];
		int place = bout[ii]*2+bout[jj];
		cout<<"place"<<place<<endl;
		for(int i=0;i<33;i++){
			out[i]=out[i]^gg[place*33+i];
		}
	}
};	
}
#endif// HALFGATE_EVA_H__