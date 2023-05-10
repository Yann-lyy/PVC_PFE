#ifndef EMP_GC_GEN_H__
#define EMP_GC_GEN_H__
#include "pfe/utils/utils.h"
#include "pfe/utils/hash.h"
#include "pfe/execution/circuit_execution.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
using namespace std;
namespace emp {

inline int fourCharToInt(char a, char b, char c, char d){
	int ret_val = 0;
	ret_val  =  a;
	ret_val <<= 8;
	ret_val |=  b;
	ret_val <<= 8;
	ret_val |=  c;
	ret_val <<= 8;
	ret_val |=  d;

	return ret_val;
}

inline void point_and_permute(unsigned char *ct, unsigned char * out,unsigned char *gg){
	vector<int> point;
	int ii=-1;
	int jj=-1;
	
	// nandct[0] = ct[0] ^ wb;
	// nandct[1] = ct[1] ^ wb;
	// nandct[2] = ct[2] ^ wb; 
	// nandct[3] = ct[3] ^ wa;
	int bct[4][264];
	unsigned char nandct[4*33];

	for(int j=0;j<4;j++){
		int h=0;
		if(j<3)
			h = 1;
		else
			h = 0;
		for(int i=0;i<33;i++){
			for(int k=7;k>=0;k--){
				bct[j][i*8+7-k] = (ct[j*33+i] >> k) & 1;
			}
			nandct[j*33+i]=ct[j*33+i]^out[h*33+i];
		}
		// std::cout<<nandct[j]<<std::endl;
	}
	

	//use input a and input b to encrypt input c := hash(input_a, input_b, gateid) xor output_c 
	for(int i=0;i<264;i++){
		// std::cout<<bct[0][i]<<bct[1][i]<<bct[2][i]<<bct[3][i]<<std::endl;
		if(bct[0][i]+bct[1][i]+bct[2][i]+bct[3][i]==2)
			point.push_back(i);
	}
	for(int i=0;i<point.size();i++){
		for(int j=i+1;j<point.size();j++){
			int al,bl,cl,ar,br,cr;
			al = bct[0][point[i]] ^ bct[1][point[i]];
			bl = bct[1][point[i]] ^ bct[2][point[i]];
			cl = bct[2][point[i]] ^ bct[3][point[i]];
			ar = bct[0][point[j]] ^ bct[1][point[j]];
			br = bct[1][point[j]] ^ bct[2][point[j]];
			cr = bct[2][point[j]] ^ bct[3][point[j]];
			if((al ^ ar)||(bl ^ br)||(cl ^ cr)==1){
				ii = point[i];
				jj = point[j];
				break;
			}
		}
		if(ii!=-1)
			break;
	}
	
	
	// std::cout<<"ii jj: "<<ii<<" "<<jj<<std::endl;
	// for(int i=0;i<4;i++){	
	// 	std::cout<<bct[i][ii]<<bct[i][jj]<<std::endl;
	// }
	for(int i=0;i<4;i++){
		int a = bct[i][ii];
		int b = bct[i][jj];
		int place = 2*a+b;
		memcpy(gg+place*33,nandct+i*33,33*sizeof(char));
	}
	
	// std::cout<<"p:"<<p[0]<<std::endl;
	gg[4*33]=ii/256;
	gg[4*33+1]=ii%256;
	gg[4*33+2]=jj/256;
	gg[4*33+3]=jj%256;

	/*point and permute

	Use a,b,c,d to donate the four rows of the truth table of the circuits without garbling. Find a pair(i,j) makes the set {(a[i],a[j]),(b[i],b[j]),(c[i],c[j]),(d[i],d[j])} = {(0,0),(0,1),(1,0),(1,1)}.
	Additionally, use the pair(i,j) to garble the circuits. 

	Use e,f,g,h to donate the four rows of the truth table of the garbled circuits.
	(e[i],e[j])=(0,0),(f[i],f[j])=(0,1),(g[i],g[j])=(1,0),(h[i],h[j])=(1,1)

	*/

}

class GarbledCircuitsGen:public CircuitExecution {
public:
	void nand_gate(int gateid, unsigned char* in, unsigned char* out, unsigned char* gg) override {
		//in := in_a_0, in_a_1, in_b_0, in_b_1 
		//out := out_c_0, out_c_1

		
		unsigned char *ct = new unsigned char[4*33]();
		unsigned char *pt= new unsigned char[4*3*33]();
		Hash hash;
		memcpy(pt, in, sizeof(char)*33);
		memcpy(pt+3*33, in, sizeof(char)*33);
		memcpy(pt+2*3*33, in+33, sizeof(char)*33);
		memcpy(pt+3*3*33, in+33, sizeof(char)*33);
		memcpy(pt+33, in+2*33, sizeof(char)*33);
		memcpy(pt+2*3*33+33, in+2*33, sizeof(char)*33);
		memcpy(pt+3*33+33, in+3*33, sizeof(char)*33);
		memcpy(pt+3*3*33+33, in+3*33, sizeof(char)*33);


		unsigned char cgateid[4];
		cgateid[0] = gateid%256;
		cgateid[1] = gateid/256%256;
		cgateid[2] = gateid/256/256%256;
		cgateid[3] = gateid/256/256/256%256;
		memcpy(pt+2*33,cgateid,sizeof(char)*4);
		memcpy(pt+3*33+2*33,cgateid,sizeof(char)*4);
		memcpy(pt+2*3*33+2*33,cgateid,sizeof(char)*4);
		memcpy(pt+3*3*33+2*33,cgateid,sizeof(char)*4);

		for(int i=0;i<4;i++){
			char tmp[64];
			hash.hash_once(tmp,pt+i*3*33,3*33*sizeof(char));
			hash.hash_once(tmp+32,tmp,32*sizeof(char));
			memcpy(ct+i*33, tmp, sizeof(char)*33);
			
		}
		
		// ct := hash(in_a,in_b,gate_id)
		point_and_permute(ct, out, gg);

		// gct := garbled(ct) 
	}
};
}
#endif// HALFGATE_GEN_H__
