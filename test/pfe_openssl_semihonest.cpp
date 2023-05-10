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

void test(int party, NetIO *io, int len) {
    clock_t start,end,mid,setup,pvc_start,pvc_end;
    start = clock();
    Group G;
    const int num_input = 50;
    const int num_input_alice = 10;
    const int num_gate = len;
    const int num_output = 9;
    const int N = 2*num_gate;
    const int M = num_gate+num_input-num_output;
    // cout<<"start"<<endl;
    IKNP<NetIO> * iknp = new IKNP<NetIO>(io);
    if(party == ALICE){
        
        int *gate_arr = new int[3*num_gate];
        generate_gates(gate_arr, num_input, num_gate);
        PFE_circuits cf(num_gate, num_input, num_output, gate_arr);
        //cf
        BigInt s;
        G.get_rand_bn(s);
        Point h = G.mul_gen(s);
        io->send_pt(&h);
        //pick s and h
        BigInt *rand_t= new BigInt[N];
        for(int i=0;i<N;i++){
            G.get_rand_bn(rand_t[i]);
            for(int j=0;j<i;j++){
                if(BN_cmp(rand_t[j].n,rand_t[i].n)==0){
                    i--;
                    break;
                }
            }
        } 
        //get ti 
        Point generators[M];
        io->recv_pt(&G, generators, M);


        setup = clock();
        cout<<"setup time = "<<double(setup-start)/CLOCKS_PER_SEC<<"s"<<endl; 
        cf.topological_sorting();
        //receive generators
        char *mess = new char[33*M+1];
        mess[33*M]='\0';
        for(int j=0;j<M;j++){
            EC_POINT_point2oct(G.ec_group, generators[j].point,POINT_CONVERSION_COMPRESSED,
            (unsigned char*)(mess+33*j), 33, NULL);
        }
        EC_POINT* popo = EC_POINT_new(G.ec_group);
        int rv = EC_POINT_oct2point(G.ec_group, popo, (unsigned char*)mess, 33, NULL);
        // cout<<"  rv         "<<rv<<endl;
        

        Point ca[N];
        Point cb[N];
        Point phia[N];
        Point phib[N]; 
        Point d[N];

        BigInt *r = new BigInt[N];
        for(int i=0;i<N;i++){
            G.get_rand_bn(r[i]);
            ca[i] = G.mul_gen(r[i]);
            cb[i] = h.mul(r[i]);
            Point tmpt = generators[cf.extended_permutation[i]];
            cb[i] = cb[i].add(tmpt);
            phia[i] = ca[i].mul(rand_t[i]);
            phib[i] = cb[i].mul(rand_t[i]);
        }
        // cout<<"BN bytes"<<BN_num_bytes(r[0].n)<<endl;
        // cout<<"finish cf"<<endl;
        for(int i=0;i<N;i++){
            d[i] = phia[i].mul(s);
        }
        //get phi, the encryption of g_{\Pif}，phi_prime,d
        io->send_pt(ca, N);
        io->send_pt(cb, N);
        io->send_pt(phia, N);
        io->send_pt(phib, N);
        io->send_pt(d, N);
        Point p[N];
        io->recv_pt(&G, p, N);

        mid = clock();
        cout<<"initial phase time = "<<double(mid-setup)/CLOCKS_PER_SEC<<"s"<<endl; 
        cout<<"io              "<<io->counter<<endl;
        
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
            EC_POINT_point2oct(G.ec_group, generators[j].point,POINT_CONVERSION_COMPRESSED,
            (unsigned char*)(gp+33*j), 33, NULL);
        }
        for(int j=0;j<N;j++){
            EC_POINT_point2oct(G.ec_group, p[j].point,POINT_CONVERSION_COMPRESSED,
            (unsigned char*)(gp+33*M+33*j), 33, NULL);
        }
        gp[33*M+33*N] = '\0';
       
    //    cout<<"gp"<<endl;
         

        unsigned char * in_bob = new unsigned char[(num_input-num_input_alice)*33];
        io->recv_data(in_bob, (num_input-num_input_alice)*33);
        unsigned char * garbledgate = new unsigned char[num_gate*5*33];
        io->recv_data(garbledgate, num_gate*5*33);

       
        for(int i=0;i<num_input-num_input_alice;i++){
            memcpy(in+num_input_alice*33+i*33,in_bob+i*33,33);
        }
        // cf.compute(in, &G, rand_t, garbledgate);
        
    }else{
        // start = clock();
        Point h_recv[1];
        io->recv_pt(&G, h_recv);
        Point h = h_recv[0];

        Point generators[M];
        BigInt *rand_power = new BigInt[M];
        BigInt order, cofactor;
        BN_CTX * bn_ctx = nullptr;
        EC_GROUP_get_order(G.ec_group, order.n, bn_ctx);
        EC_GROUP_get_cofactor(G.ec_group, cofactor.n, bn_ctx);

        // cout<<"start generators"<<endl;
        for(int i=0;i<M;i++){
            G.get_rand_bn(rand_power[i]);
            generators[i] = G.mul_gen(rand_power[i]);
            for(int j=0;j<i;j++){
                if(BN_cmp(rand_power[j].n,rand_power[i].n)==0){
                    i--;
                    break;
                }
            }
        }

        io->send_pt(generators, M);

        setup = clock();
        cout<<"setup time = "<<double(setup-start)/CLOCKS_PER_SEC<<"s"<<endl; 

        char *mess = new char[33*M+1];
        mess[33*M]='\0';
        for(int j=0;j<M;j++){
            EC_POINT_point2oct(G.ec_group, generators[j].point,POINT_CONVERSION_COMPRESSED,
            (unsigned char*)(mess+33*j), 33, NULL);
        }
        // io->send_data(mess, 33*M+1);
        // pick generators
        Point ca[N];
        Point cb[N];
        Point phia[N];
        Point phib[N]; 
        Point p[N];
        Point d[N];
        io->recv_pt(&G, ca, N);
        io->recv_pt(&G, cb, N);
        io->recv_pt(&G, phia, N);
        io->recv_pt(&G, phib, N);
        io->recv_pt(&G, d, N);
        for(int i=0;i<N;i++){
            Point tmp = d[i].inv();
            p[i] = phib[i].add(tmp);
        }
        io->send_pt(p, N);
        mid = clock();
        cout<<"initial phase time = "<<double(mid-setup)/CLOCKS_PER_SEC<<"s"<<endl; 
        cout<<"io              "<<io->counter<<endl;

    
        pvc_start = clock();
        unsigned char **choose_w = new unsigned char*[1];
        unsigned char **choose_alpha = new unsigned char*[1];
        unsigned char **garbledGates = new unsigned char*[1];
        unsigned char **a_oct = new unsigned char*[1];
        unsigned char **b_oct = new unsigned char*[1];

        // cout<<"for what"<<endl;
// #pragma omp parallel for  num_threads(4) 
        for(int i=0;i<1;i++){
            // printf("this is No.%d Thread ,i=%d\n", omp_get_thread_num(), i);
            choose_w[i] = new unsigned char[33*2*num_output];
            choose_alpha[i] = new unsigned char[64];
            garbledGates[i] = new unsigned char[num_gate*5*33];
            a_oct[i]= new unsigned char[2*(M+num_output)*33];
            b_oct[i] = new unsigned char[2*N*33];

            BigInt alpha_a, alpha_b;
            alpha_a.from_bin(choose_alpha[i], 32);
            alpha_b.from_bin(choose_alpha[i]+32 , 32);
            Point a_point[2*M],b_point[2*N]; 
            size_t oct_len = 33;
            G.get_rand_bn(alpha_a);
            G.get_rand_bn(alpha_b);
            for(int j=0;j<M;j++){
                a_point[2*j] = generators[j].mul(alpha_a);
                a_point[2*j+1] = generators[j].mul(alpha_b);
                EC_POINT_point2oct(G.ec_group, a_point[2*j].point,POINT_CONVERSION_COMPRESSED,
                                        a_oct[i]+2*j*33, oct_len, NULL);
                EC_POINT_point2oct(G.ec_group, a_point[2*j+1].point,POINT_CONVERSION_COMPRESSED,
                                        a_oct[i]+(2*j+1)*33, oct_len, NULL);
            }
            for(int j=0;j<N;j++){
                b_point[2*j] = p[j].mul(alpha_a);
                b_point[2*j+1] = p[j].mul(alpha_b);
                EC_POINT_point2oct(G.ec_group, b_point[2*j].point,POINT_CONVERSION_COMPRESSED,
                                        b_oct[i]+2*j*33, oct_len, NULL);
                EC_POINT_point2oct(G.ec_group, b_point[2*j+1].point,POINT_CONVERSION_COMPRESSED,
                                        b_oct[i]+(2*j+1)*33, oct_len, NULL);
            }
            for(int j=0;j<num_output;j++){
                BigInt bn_w_a, bn_w_b;
                bn_w_a.from_bin(choose_w[i]+j*64, 32); 
                bn_w_b.from_bin(choose_w[i]+j*64+32, 32); 
                Point p_w_a, p_w_b;
                p_w_a = G.mul_gen(bn_w_a);
                p_w_b = G.mul_gen(bn_w_b);
                EC_POINT_point2oct(G.ec_group, p_w_a.point,POINT_CONVERSION_COMPRESSED,
                                        a_oct[i]+2*(M+j)*33, oct_len, NULL);
                EC_POINT_point2oct(G.ec_group, p_w_b.point,POINT_CONVERSION_COMPRESSED,
                                        a_oct[i]+(2*(M+j)+1)*33, oct_len, NULL);
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
            EC_POINT_point2oct(G.ec_group, generators[j].point,POINT_CONVERSION_COMPRESSED,
            (unsigned char*)(gp+33*j), 33, NULL);
        }
        for(int j=0;j<N;j++){
            EC_POINT_point2oct(G.ec_group, p[j].point,POINT_CONVERSION_COMPRESSED,
            (unsigned char*)(gp+33*M+33*j), 33, NULL);
        } 
        gp[33*M+33*N] = '\0';
        
        // cout<<"gp"<<endl;
            
        bool *input_bob = new bool[num_input-num_input_alice];
        for(int i=0;i<num_input-num_input_alice;i++){
            input_bob[i] = false;
        }
        unsigned char * in_bob = new unsigned char[(num_input-num_input_alice)*33];
        io->send_data(in_bob, (num_input-num_input_alice)*33);
        io->send_data(garbledGates[0], num_gate*5*33);
    }  
    end = clock();
    cout<<"io              "<<io->counter<<endl;
    cout<<"evaluation time = "<<double(end-mid)/CLOCKS_PER_SEC<<"s"<<endl;
    cout<<"all time = " <<double(end-setup)/CLOCKS_PER_SEC<<"s"<<endl;
}

int main(int argc, char** argv) {
	parse_party_and_port(argv, &party, &port);
    int len = 0;
    len = atoi(argv[3]);
    // cout<<"start"<<endl;
	NetIO* io = new NetIO(party == ALICE? nullptr:"192.168.5.100", port);
    // cout<<"in test"<<endl;
	test(party, io, len);
    delete io;
}
