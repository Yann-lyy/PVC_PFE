#ifndef EMP_BRISTOL_FORMAT_H__
#define EMP_BRISTOL_FORMAT_H__

#include "pfe/pfe.h"
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>
#include <stdio.h>
#include <algorithm> 
#include <fstream>

using std::vector;
using std::pair;
using namespace std;

namespace emp {

class PFE_circuits{ public:
	int num_gate, num_input, num_output, num_wire;
	int* gates;
	vector<int> extended_permutation;
	unsigned char* input_wires;
	vector<int> sorted_gates;
	PFE_circuits(int num_gate, int num_input, int num_output, int * gate_arr) {
		this->num_gate = num_gate;
		this->num_input = num_input;
		this->num_output = num_output;
		this->num_wire = num_gate + num_input ;
		gates = gate_arr;
		extended_permutation.resize(2*num_gate,-1);
		input_wires =new unsigned char[(num_input+num_gate)*33]();
	}
	void topological_sorting(){
		int wire_degree[num_wire]={0};
		int gate_degree[num_gate]={0};
		int gate_output[num_wire]={0};
		vector<int> oconnecti[num_wire];
		for(int i=0;i<num_gate;i++){
			wire_degree[gates[3*i+2]]=1;
		}
		for(int i=0;i<num_gate;i++){
			gate_degree[i]=wire_degree[gates[3*i]]+wire_degree[gates[3*i+1]];
		}
		for(int i=0;i<num_wire;i++){
			gate_output[i]=-1;
		}
		for(int i=0;i<num_input;i++){
			gate_output[i] = i;
		}
		for(int i=0;i<num_gate;i++){
			if(wire_degree[gates[3*i]]==1){
				oconnecti[gates[3*i]].push_back(i);
			}
			if(wire_degree[gates[3*i+1]]==1){
				oconnecti[gates[3*i+1]].push_back(i);
			}
		}
		for(int i=0;i<num_gate;i++){
			int *index;
			index=find(gate_degree,gate_degree+num_gate,0);
			int x = index-gate_degree;
			gate_degree[x]=-1;
			for(int j=0;j<3;j++)
				sorted_gates.push_back(gates[3*x+j]);
			for(int j=0;j<oconnecti[gates[3*x+2]].size();j++){
				// cout<<oconnecti[gates[3*x+2]][j]<<endl;
				gate_degree[oconnecti[gates[3*x+2]][j]]--;
			}
			gate_output[gates[3*x+2]]=num_input+i;
		}
		int index=0;
		for(int i=0;i<num_gate;i++){
			extended_permutation[2*i]=gate_output[gates[3*i]];
			extended_permutation[2*i+1]=gate_output[gates[3*i+1]];
		}
		// cout<<"gate: "<<endl;
		// for(int i=0;i<num_gate;i++){
		// 	cout<<sorted_gates[3*i]<<" "<<sorted_gates[3*i+1]<<" "<<sorted_gates[3*i+2]<<endl;
		// }
		// cout<<"EP: "<<endl;
		// for(int i=0;i<2*num_gate;i++){
		// 	cout<<extended_permutation[i]<<endl;
		// }
		// cout<<"finish"<<endl;
	}
	
	void compute(const unsigned char *a, Group *G, BigInt *rand_t, unsigned char* garbledGates){
		memcpy(input_wires, a, sizeof(char)*num_input*33);
		// for(int i=0;i<num_input;i++){
		// 	for(int j=0;j<33;j++)
        //     	printf("%d ",input_wires[i*33+j]);
		// 	cout<<endl;
		// 	cout<<endl;
		// }
		cout<<"??"<<endl;
		for(int i=0;i<num_gate;i++){
			cout<<"i=  "<<i<<endl;
			int input_left = extended_permutation[2*i];
			int input_right = extended_permutation[2*i+1];
			int out = i + num_input;
			unsigned char *gt = new unsigned char[5*33+1];
			gt[5*33]='\0';
			for(int j=0;j<5;j++){
				memcpy(gt+j*33, garbledGates+(5*i+j)*33, 33*sizeof(char));
			}
			unsigned char *in = new unsigned char[2*33]();
			memcpy(in, input_wires+input_left*33, 33*sizeof(char));
			memcpy(in+33, input_wires+input_right*33, 33*sizeof(char));
			cout<<"i=  "<<i<<endl;
			Point inl,inr;
			inl.group = G;
			inr.group = G;
			inl.point = EC_POINT_new(G->ec_group);
			inr.point = EC_POINT_new(G->ec_group);
			EC_POINT_oct2point(G->ec_group, inl.point, in, 33*sizeof(char), NULL);
			EC_POINT_oct2point(G->ec_group, inr.point, in+33, 33*sizeof(char), NULL);
			inl = inl.mul(rand_t[2*i]);
			inr = inr.mul(rand_t[2*i+1]);
			EC_POINT_point2oct(G->ec_group, inl.point,POINT_CONVERSION_COMPRESSED,
										in, 33, NULL);
			EC_POINT_point2oct(G->ec_group, inr.point,POINT_CONVERSION_COMPRESSED,
										in+33, 33, NULL);
			cout<<"i=  "<<i<<endl;
			unsigned char *result = new unsigned char[34];
			result[33] = '\0';
			CircuitExecution::circ_exec->nand_gate(i,in,gt,result);
			memcpy(input_wires+out*33, result, 33*sizeof(char));
		}
	}
};
}
#endif// CIRCUIT_FILE