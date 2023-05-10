#include "pfe/pfe.h"
#include <iostream>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <fstream>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>
#include <openssl/buffer.h>
#include <openssl/pem.h>
#include<omp.h>
#include <time.h>
using namespace std;
using namespace emp;


void inline generate_gates(int *gate_arr,int lengthofInput, int numberofGates){
    for(int i=0;i<lengthofInput-1;i++){
        gate_arr[3*i] = i;
        gate_arr[3*i+1] = i+1;
        gate_arr[3*i+2] = lengthofInput+i;
    }
    int j=0;
	for(int i=lengthofInput-1;i<numberofGates;i++){
        gate_arr[3*i] = j++;
        gate_arr[3*i+1] = j;
        gate_arr[3*i+2] = i;
        if(j>lengthofInput-2){
            j=0;
        }
    }
}

void inline block_to_char(block* a, unsigned char* b){
    memcpy(b, a, sizeof(char)*11);
    memcpy(b+11, a+1, sizeof(char)*11);
    memcpy(b+22, a+2, sizeof(char)*11);

}
void inline char_to_block(block* a, unsigned char* b){
    memcpy(a, b, sizeof(char)*11);
    memcpy(a+1, b+11, sizeof(char)*11);
    memcpy(a+2, b+22, sizeof(char)*11);
}

int port, party;
const static int threads = 1;

void test(int party, NetIO *io) {

    

    clock_t start,end,mid,setup,pvc_start,pvc_end;
    start = clock();
    uint8_t rand_g=102;
    uint8_t* rand_g_p = &rand_g;
    Group_relic G(rand_g_p);
    
    
    bn_t a, b;
    bn_null(a);
	bn_null(b);
    bn_new(a);
	bn_new(b);
    bn_rand(a, RLC_POS, RLC_BN_BITS);
    bn_rand(b, RLC_POS, RLC_BN_BITS);
    if (bn_cmp(a, b) != RLC_EQ) {
        if (bn_cmp(a, b) == RLC_GT) {
            cout<<"right";
        } else {
            cout<<"right";
        }
    }


    
    const int num_input = 50;
    const int num_input_alice = 10;
    const int num_gate = 1000;
    const int num_output = 9;
    const int N = 2*num_gate;
    const int M = num_gate+num_input-num_output;
    IKNP<NetIO> * iknp = new IKNP<NetIO>(io);
    Point_relic* generator = G.get_generator();
    cout<<"start party "<<party<<endl;
    if(party == ALICE){
        
        int *gate_arr = new int[3*num_gate];
        generate_gates(gate_arr, num_input, num_gate);
        PFE_circuits cf(num_gate, num_input, num_output, gate_arr);
        //cf
        Num_relic* s = G.get_rnd_num();
        Point_relic h = *generator^*s;
        io->send_pt(&h);
        //pick s and h
        Num_relic* rand_t= new Num_relic[N];
        for(int i=0;i<N;i++){
            rand_t[i] = *G.get_rnd_num();
            for(int j=0;j<i;j++){
                if(rand_t[j]==rand_t[i]){
                    i--;
                    break;
                }
            }
        } 
        //get ti 
        Point_relic generators[M];
        for(int i=0;i<M;i++){
            generators[i].set(&G);
        }
        io->recv_pt(generators, M);


        setup = clock();
        cout<<"setup time = "<<double(setup-start)/CLOCKS_PER_SEC<<"s"<<endl; 
        cf.topological_sorting();
        //receive generators
        char *mess = new char[33*M+1];
        mess[33*M]='\0';
        for(int j=0;j<M;j++){
            generators[j].to_bin((unsigned char*)(mess+33*j));
        }
        

        Point_relic ca[N];
        Point_relic cb[N];
        Point_relic phia[N];
        Point_relic phib[N]; 
        Point_relic d[N];
        Point_relic A[N];

        Num_relic r[N];
        for(int i=0;i<N;i++){
            ca[i].set(&G);
            cb[i].set(&G);
            phia[i].set(&G);
            phib[i].set(&G);
            d[i].set(&G);
            r[i].set(&G);
            A[i].set(&G);

            r[i] = *G.get_rnd_num();
            ca[i] = *generator ^ r[i];
            cb[i] = h ^ r[i];
            Point_relic tmpt = generators[cf.extended_permutation[i]];
            cb[i] = cb[i]+tmpt;
            phia[i] = ca[i] ^ rand_t[i];
            phib[i] = cb[i] ^ rand_t[i];
            unsigned char * tmp =new unsigned char[34];
			tmp[33] = '\0';
            ca[i].to_bin(tmp);
			A[i]= *generator ^ r[i];;
            cout<<"  "<<i<<"  "<<(bool)(A[i]==ca[i])<<endl;
        }
        // cout<<"BN bytes"<<BN_num_bytes(r[0].n)<<endl;
        cout<<"finish cf"<<endl;
        for(int i=0;i<N;i++){
            d[i] = phia[i]^*s;
        }
        //get phi, the encryption of g_{\Pif}，phi_prime,d
        io->send_pt(ca, N);
        io->send_pt(cb, N);
        io->send_pt(phia, N);
        io->send_pt(phib, N);
        io->send_pt(d, N);
        Point_relic p[N];
        io->recv_pt(p, N);

        
        cout<<"initial phase time = "<<double(mid-setup)/CLOCKS_PER_SEC<<"s"<<endl; 
        
        block *block_in = new block[num_input*3]; 
        bool *input_alice = new bool[3*num_input_alice];
        for(int i=0;i<3*num_input_alice;i++){
            input_alice[i] = false;
        }
        // cout<<"alice input"<<endl;
        for(int j=0;j<num_input_alice;j++){
            iknp->recv(block_in+3*j, input_alice+3*j, 3);
        }
        // cout<<"input alice finish"<<endl;
        unsigned char* in = new unsigned char[num_input*33];
        for(int i=0;i<num_input_alice;i++){
            block_to_char(block_in+i*3, in+i*33);
        }
        
        char *gp = new char[33*M+33*N+1];
        for(int j=0;j<M;j++){
            generators[j].to_bin((unsigned char*)(gp+33*j));
        }
        for(int j=0;j<N;j++){
            p[j].to_bin((unsigned char*)(gp+33*M+33*j));
        }
        gp[33*M+33*N] = '\0';
       
       cout<<"gp"<<endl;
         

        unsigned char * in_bob = new unsigned char[(num_input-num_input_alice)*33];
        io->recv_data(in_bob, (num_input-num_input_alice)*33);
        unsigned char * garbledgate = new unsigned char[num_gate*5*33];
        io->recv_data(garbledgate, num_gate*5*33);

       
        for(int i=0;i<num_input-num_input_alice;i++){
            memcpy(in+num_input_alice*33+i*33,in_bob+i*33,33);
        }
        // cf.compute(in, &G, rand_t, garbledgate);
        
    }else{
        Point_relic h(&G);
        io->recv_pt(&h);


        Point_relic generators[M];

        cout<<"start generators"<<endl;
        for(int i=0;i<M;i++){
            generators[i].set(&G);
            generators[i].set(G.get_rnd_point());
            for(int j=0;j<i;j++){
                if(generators[j]==generators[i]){
                    i--;
                    break;
                }
            }
        }
        cout<<"send generators"<<endl;
        io->send_pt(generators, M);

        setup = clock();
        cout<<"setup time = "<<double(setup-start)/CLOCKS_PER_SEC<<"s"<<endl; 

        char *mess = new char[33*M+1];
        mess[33*M]='\0';
        for(int j=0;j<M;j++){
            generators[j].to_bin((unsigned char*)(mess+33*j));
        }
        // pick generators
        Point_relic ca[N];
        Point_relic cb[N];
        Point_relic phia[N];
        Point_relic phib[N]; 
        Point_relic p[N];
        Point_relic d[N];

        for(int i=0;i<N;i++){
            ca[i].set(&G);
            cb[i].set(&G);
            phia[i].set(&G);
            phib[i].set(&G);
            p[i].set(&G);
            d[i].set(&G);
        }

        io->recv_pt(ca, N);
        io->recv_pt(cb, N);
        io->recv_pt(phia, N);
        io->recv_pt(phib, N);
        io->recv_pt(d, N);
        for(int i=0;i<N;i++){
            p[i] = phib[i]-d[i];
        }
        io->send_pt(p, N);
        mid = clock();
        cout<<"initial phase time = "<<double(mid-setup)/CLOCKS_PER_SEC<<"s"<<endl; 

    
        pvc_start = clock();
        unsigned char **choose_w = new unsigned char*[1];
        unsigned char **choose_alpha = new unsigned char*[1];
        unsigned char **garbledGates = new unsigned char*[1];
        unsigned char **a_oct = new unsigned char*[1];
        unsigned char **b_oct = new unsigned char*[1];

        // cout<<"for what"<<endl;
#pragma omp parallel for  num_threads(4) 
        for(int i=0;i<1;i++){
            // printf("this is No.%d Thread ,i=%d\n", omp_get_thread_num(), i);
            choose_w[i] = new unsigned char[33*2*num_output];
            choose_alpha[i] = new unsigned char[64];
            garbledGates[i] = new unsigned char[num_gate*5*33];
            a_oct[i]= new unsigned char[2*(M+num_output)*33];
            b_oct[i] = new unsigned char[2*N*33];

            Num_relic alpha_a, alpha_b;
            alpha_a.from_bin(choose_alpha[i], 32);
            alpha_b.from_bin(choose_alpha[i]+32 , 32);
            Point_relic a_point[2*M],b_point[2*N]; 
            size_t oct_len = 33;
            alpha_a = *G.get_rnd_num();
            alpha_b = *G.get_rnd_num();
            for(int j=0;j<M;j++){
                a_point[2*j] = generators[j] ^ alpha_a;
                a_point[2*j+1] = generators[j] ^ alpha_b;
                a_point[2*j].to_bin(a_oct[i]+2*j*33);
                a_point[2*j+1].to_bin(a_oct[i]+(2*j+1)*33);
            }
            for(int j=0;j<N;j++){
                b_point[2*j] = p[j] ^ alpha_a;
                b_point[2*j+1] = p[j] ^ alpha_b;
                b_point[2*j].to_bin(b_oct[i]+2*j*33);
                b_point[2*j+1].to_bin(b_oct[i]+(2*j+1)*33);
            }
            for(int j=0;j<num_output;j++){
                Num_relic bn_w_a, bn_w_b;
                bn_w_a.from_bin(choose_w[i]+j*64, 32); 
                bn_w_b.from_bin(choose_w[i]+j*64+32, 32); 
                Point_relic p_w_a, p_w_b;
                p_w_a = *generator ^ bn_w_a;
                p_w_b = *generator ^ bn_w_b;
                p_w_a.to_bin(a_oct[i]+2*(M+j)*33);
                p_w_b.to_bin(a_oct[i]+(2*(M+j)+1)*33);
            }
            CircuitExecution::circ_exec = new GarbledCircuitsGen(); 
            construct_pfegarbledgates(a_oct[i]+2*num_input*33,b_oct[i],garbledGates[i], num_gate, num_input);
            delete CircuitExecution::circ_exec;
        }
        // cout<<"finish garbled circuits"<<endl;
        for(int i=0;i<1;i++){
            for(int j=0;j<num_input_alice;j++){
                block* alice_block_a = new block[3];
                char_to_block(alice_block_a,&a_oct[i][66*j]); 
                block* alice_block_b = new block[3];
                char_to_block(alice_block_b,&a_oct[i][66*j+33]); 

                iknp->send(alice_block_a, alice_block_b, 3);
            }
        }
    


        
        char *gp = new char[33*M+33*N+1];
        for(int j=0;j<M;j++){
            generators[j].to_bin((unsigned char*)(gp+33*j));
        }
        for(int j=0;j<N;j++){
            p[j].to_bin((unsigned char*)(gp+33*M+33*j));
        } 
        gp[33*M+33*N] = '\0';
        
        cout<<"gp"<<endl;
            
        bool *input_bob = new bool[num_input-num_input_alice];
        for(int i=0;i<num_input-num_input_alice;i++){
            input_bob[i] = false;
        }
        unsigned char * in_bob = new unsigned char[(num_input-num_input_alice)*33];
        io->send_data(in_bob, (num_input-num_input_alice)*33);
        io->send_data(garbledGates[0], num_gate*5*33);
    }  
    end = clock();
    cout<<"evaluation time = "<<double(end-mid)/CLOCKS_PER_SEC<<"s"<<endl;
    cout<<"all time = " <<double(end-start)/CLOCKS_PER_SEC<<"s"<<endl;
}

int main(int argc, char** argv) {
	parse_party_and_port(argv, &party, &port);
	NetIO* io = new NetIO(party == ALICE? nullptr:"192.168.5.100", port);
    cout<<"in test"<<endl;
	test(party, io);
    delete io;
}
