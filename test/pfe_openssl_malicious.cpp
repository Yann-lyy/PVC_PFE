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
void write_certificate(int n,char* p, int j, unsigned char *com_gcch, block *seed, block *decom_seed, unsigned char *sign){ 
    ofstream fout("./certificate.txt");
    fout<<n<<" ";
    for(int i=0;i<n*33;i++){
        int t = p[i];
        fout<<t<<" ";
    }
    fout<<j;
    for(int i=0;i<32;i++){
        int t = com_gcch[i];
        fout<<" "<<t;
    } 
    int* tmp = (int *)seed;
    for(int i=0;i<4;i++){
        fout<<" "<<tmp[i];
    }
    tmp = (int *)decom_seed;
    for(int i=0;i<4;i++){
        fout<<" "<<tmp[i];
    }
    for(int i=0;i<256;i++){
        int t = sign[i];
        fout<<" "<<t;
    }
}

void write_common_reference_string(int m, char* generators){
    ofstream fout("./crs.txt");
    fout<<m;
    for(int i=0;i<m*33;i++){
        int t = generators[i];
        fout<<" "<<t;
    }
}

void write_verify_key(Group *G, char *sigpubkey){
    ofstream fout("./pubkey.txt");
    for(int i=0;i<33;i++){
        int t = sigpubkey[i];
        fout<<t<<" ";
    }
}



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

// void simulate(){
//     unsigned char **choose_w = new unsigned char*[num_pvc];
//     unsigned char **choose_alpha = new unsigned char*[num_pvc];
//     unsigned char **garbledGates = new unsigned char*[num_pvc];
//     unsigned char **a_oct = new unsigned char*[num_pvc];
//     unsigned char **b_oct = new unsigned char*[num_pvc];
//     ECDSA_SIG *sig[num_pvc];
//     PRG prg;

//     cout<<"?????????????????????"<<endl;
//     for(int i=0;i<num_pvc;i++){
//         // printf("this is No.%d Thread ,i=%d\n", omp_get_thread_num(), i);
//         choose_w[i] = new unsigned char[33*2*num_output];
//         choose_alpha[i] = new unsigned char[64];
//         garbledGates[i] = new unsigned char[num_gate*5*33];
//         a_oct[i]= new unsigned char[2*(M+num_output)*33];
//         b_oct[i] = new unsigned char[2*N*33];
//         prg.reseed(&seed[i]);
//         prg.random_data(choose_w[i], 33*2*num_output);
//         prg.random_data(choose_alpha[i], 64);
//         BigInt alpha_a, alpha_b;
//         alpha_a.from_bin(choose_alpha[i], 32);
//         alpha_b.from_bin(choose_alpha[i]+32 , 32);
//         Point a_point[2*M],b_point[2*N]; 
//         size_t oct_len = 33;
//         G.get_rand_bn(alpha_a);
//         G.get_rand_bn(alpha_b);
//         for(int j=0;j<M;j++){
//             // cout<<"bbbbbbbbbbbbb"<<j<<endl;
//             a_point[2*j] = generators[j].mul(alpha_a);
//             a_point[2*j+1] = generators[j].mul(alpha_b);
//             // cout<<"bbbbbbbbbbbbb"<<j<<endl;
//             EC_POINT_point2oct(G.ec_group, a_point[2*j].point,POINT_CONVERSION_COMPRESSED,
//                                     a_oct[i]+2*j*33, oct_len, NULL);
//             EC_POINT_point2oct(G.ec_group, a_point[2*j+1].point,POINT_CONVERSION_COMPRESSED,
//                                     a_oct[i]+(2*j+1)*33, oct_len, NULL);
//             // cout<<"bbbbbbbbbbbbb"<<j<<endl;
//         }
//         for(int j=0;j<N;j++){
//             // cout<<"aaaaaaaaaaa"<<j<<endl;
//             b_point[2*j] = p[j].mul(alpha_a);
//             // cout<<"aaaaaaaaaaa"<<j<<endl;
//             b_point[2*j+1] = p[j].mul(alpha_b);
//             // cout<<"aaaaaaaaaaa"<<j<<endl;
//             EC_POINT_point2oct(G.ec_group, b_point[2*j].point,POINT_CONVERSION_COMPRESSED,
//                                     b_oct[i]+2*j*33, oct_len, NULL);
//             // cout<<"aaaaaaaaaaa"<<j<<endl;
//             EC_POINT_point2oct(G.ec_group, b_point[2*j+1].point,POINT_CONVERSION_COMPRESSED,
//                                     b_oct[i]+(2*j+1)*33, oct_len, NULL);
//             // cout<<"aaaaaaaaaaa"<<j<<endl;
//         }
//         for(int j=0;j<num_output;j++){
//             BigInt bn_w_a, bn_w_b;
//             bn_w_a.from_bin(choose_w[i]+j*64, 32); 
//             bn_w_b.from_bin(choose_w[i]+j*64+32, 32); 
//             Point p_w_a, p_w_b;
//             p_w_a = G.mul_gen(bn_w_a);
//             p_w_b = G.mul_gen(bn_w_b);
//             EC_POINT_point2oct(G.ec_group, p_w_a.point,POINT_CONVERSION_COMPRESSED,
//                                     a_oct[i]+2*(M+j)*33, oct_len, NULL);
//             EC_POINT_point2oct(G.ec_group, p_w_b.point,POINT_CONVERSION_COMPRESSED,
//                                     a_oct[i]+(2*(M+j)+1)*33, oct_len, NULL);
//         }
//         cout<<"start construct"<<endl;
//         CircuitExecution::circ_exec = new GarbledCircuitsGen(); 
//         construct_pfegarbledgates(a_oct[i]+2*num_input*33,b_oct[i],garbledGates[i], num_gate, num_input);
//         delete CircuitExecution::circ_exec;
//     }
//     cout<<"finish garbled circuits"<<endl;
//     for(int i=0;i<num_pvc;i++){
//         for(int j=0;j<num_input_alice;j++){
//             block* alice_block_a = new block[3];
//             char_to_block(alice_block_a,&a_oct[i][66*j]); 
//             block* alice_block_b = new block[3];
//             char_to_block(alice_block_b,&a_oct[i][66*j+33]); 

//             iknp->send(alice_block_a, alice_block_b, 3);
//         }
//     }
//     cout<<"alice input send"<<endl;
//     block **decom_w_bob = new block*[num_pvc];
//     char **com_w_bob = new char*[num_pvc];
//     Hash hash;
//     char **ho = new char*[num_pvc];
//     for(int i=0;i<num_pvc;i++){
//         decom_w_bob[i] = new block[2*num_input-2*num_input_alice];
//         prg.random_block(decom_w_bob[i], 2*num_input-2*num_input_alice);
//         com_w_bob[i] = new char[(2*num_input-2*num_input_alice)*32];
//         for(int j=0;j<num_input-num_input_alice;j++){
//             hash.put_block(decom_w_bob[i]+2*j);
//             hash.put(a_oct[i]+num_input_alice*66+j*66, 33);
//             hash.digest(com_w_bob[i]+j*64);
//             hash.reset();
//             hash.put_block(decom_w_bob[i]+2*j+1);
//             hash.put(a_oct[i]+num_input_alice*66+j*66+33, 33);
//             hash.digest(com_w_bob[i]+j*64+32);
//             hash.reset();
//         }
//     }
//     for(int i=0;i<num_pvc;i++){
//         ho[i] = new char[32];
//         hash.put(choose_w[i], 33*2*num_output);
//         hash.digest(ho[i]);
//         hash.reset();
//     }

//     block *decom_gcch = new block[num_pvc];
//     char **com_gcch = new char*[num_pvc];
//     block *rand_order_wb = new block[1];
//     prg.random_block(rand_order_wb);
//     prg.random_block(decom_gcch, num_pvc);
//     bool* rand_order_wb_b = new bool[128];
//     memcpy(rand_order_wb_b, rand_order_wb, 16);
//     for(int i=0;i<num_pvc;i++){
//         com_gcch[i] = new char[32];
//         hash.put_block(decom_gcch+i, 1);
//         hash.put(garbledGates[i], num_gate*5*33*sizeof(char));
//         for(int j=0;j<num_input-num_input_alice;j++){
//             if(rand_order_wb_b[j%128]){
//                 hash.put(com_w_bob[i]+j*64,32);
//                 hash.put(com_w_bob[i]+j*64+32,32);
//             }else{
//                 hash.put(com_w_bob[i]+j*64+32,32);
//                 hash.put(com_w_bob[i]+j*64,32);
//             }
//         }
//         hash.put(ho[i], 32);
//         hash.digest(com_gcch[i]);
//         hash.reset();
//     }
// };

int port, party;
const static int threads = 1;

void test(int party, NetIO *io,int len) {
    clock_t start,end,mid,setup,pvc_start,pvc_end;
    start = clock();
    Group G;
    const int num_input = 50;
    const int num_input_alice = 10;
    const int num_gate = len;
    const int num_output = 9;
    const int N = 2*num_gate;
    const int M = num_gate+num_input-num_output;
    const int num_pvc = 2;
    // cout<<"??"<<endl;
    cout<<"start"<<endl;
    IKNP<NetIO> * iknp = new IKNP<NetIO>(io);
    if(party == ALICE){
        // cout<<"write key"<<endl;

        // FILE *keyfile = fopen("prikey.pem", "r");
        // RSA *rsa_pri = PEM_read_RSAPrivateKey( keyfile, NULL, NULL, NULL) ;
        // EVP_PKEY *sigprikey;
        // sigprikey = EVP_PKEY_new();
        // EVP_PKEY_set1_RSA(sigprikey, rsa_pri);
        // RSA_free(rsa_pri);

        FILE *pubkeyfile = fopen("pubkey.pem", "r");
        RSA *rsa_pub = PEM_read_RSAPublicKey(pubkeyfile, NULL, NULL, NULL) ;
        EVP_PKEY *sigkey;
        sigkey = EVP_PKEY_new();
        EVP_PKEY_set1_RSA(sigkey, rsa_pub);
        RSA_free(rsa_pub);
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
        // cout<<"mess"<<endl;
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
        write_common_reference_string(M, mess);
        // cout<<"write crs"<<endl;

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

        // cout<<"start zk"<<endl;
        bool zk_bool =zk_EncEP_p(&G, h, generators, ca, cb, cf.extended_permutation, r, M, N, io);
        mid = clock();
        BigInt zk_dh_t,zk_dh_c;
        G.get_rand_bn(zk_dh_t);
        Point com_dh_t;
        com_dh_t = G.mul_gen(zk_dh_t);
        io->send_pt(&com_dh_t);
        unsigned char *redata_dh_c = new unsigned char[32];
        io->recv_data(redata_dh_c, 32);
        zk_dh_c.from_bin(redata_dh_c,32);
        BigInt respon_s;
        // cout<<"zk dh one"<<endl;
        BN_mul(respon_s.n, zk_dh_c.n, s.n, G.bn_ctx);
        BN_add(respon_s.n, respon_s.n, zk_dh_t.n);
        // cout<<"zk dh two"<<endl;
        unsigned char *srespon_s = new unsigned char[32];
        // cout<<"zk dh three"<<endl;
        respon_s.to_bin(srespon_s);
        io->send_data(srespon_s,32);

        cout<<"initial phase time = "<<double(mid-setup)/CLOCKS_PER_SEC<<"s"<<endl; 
        cout<<"io              "<<io->counter<<endl;
        // cout<<"finish zk"<<endl;
        block *seed = new block[num_pvc];
        block *decom_seed = new block[num_pvc];
        char *com_seed_a = new char[32*num_pvc];
        Hash hash;
        PRG prg;
        prg.random_block(decom_seed, num_pvc);
        prg.random_block(seed, num_pvc);
        for(int i=0;i<num_pvc;i++){
            hash.put_block(decom_seed+i);
            hash.put_block(seed+i);
            hash.digest(com_seed_a+i*32);
            hash.reset();
        }
        io->send_data(com_seed_a, 32*num_pvc);

        // cout<<"com seed alice"<<endl;
        block *seed_witness = new block[num_pvc];
        bool *choose_r = new bool[num_pvc];
        for(int i=0;i<num_pvc;i++){
            choose_r[i] = false;
        }
        int rand_j;
        prg.random_data(&rand_j, 4);
        if(rand_j<0){
            rand_j = - rand_j;
        }
        rand_j = rand_j % num_pvc;
        // cout<<"rand j"<<rand_j<<endl;
        choose_r[rand_j] = true;  
        io->save = true;
        iknp->recv(seed_witness, choose_r, num_pvc);
        io->save = false;

        // cout<<"seed witness"<<endl;
        block *block_in = new block[num_input*3]; 
        bool *input_alice = new bool[3*num_input_alice];
        for(int i=0;i<3*num_input_alice;i++){
            input_alice[i] = false;
        }
        // cout<<"alice input"<<endl;
        for(int i=0;i<num_pvc;i++){
            for(int j=0;j<num_input_alice;j++){
                if(i==rand_j){
                    iknp->recv(block_in+3*j, input_alice+3*j, 3);
                }else{
                    block *blank_in = new block[3];
                    iknp->recv(blank_in, input_alice+3*j, 3);
                }
            }
        }
        // cout<<"input alice finish"<<endl;
        unsigned char* in = new unsigned char[num_input*33];
        for(int i=0;i<num_input_alice;i++){
            block_to_char(block_in+i*3, in+i*33);
        }
        unsigned char **com_gcch = new unsigned char*[num_pvc];

        BigInt *pr = new BigInt[num_pvc];
        BigInt *ps = new BigInt[num_pvc];
        for(int i=0;i<num_pvc;i++){
            com_gcch[i] = new unsigned char[33];
            com_gcch[i][32] = '\0';
            io->recv_data(com_gcch[i], 32);
        }

        char **message = new char*[num_pvc]; 
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
        int sn = 0;
        int days = 1;
        for(int i=0;i<num_pvc;i++){
            message[i] = new char[65];
            message[i][64] = '\0';
            memcpy(message[i], com_seed_a+32*i, 32);
            memcpy(message[i]+32, com_gcch[i], 32);
            auto mctx = EVP_MD_CTX_new();
            EVP_VerifyInit_ex(mctx, EVP_md5(), NULL);
            EVP_VerifyUpdate(mctx, gp, 33*M+33*N);
            EVP_VerifyUpdate(mctx, message[i], 64);
            unsigned char *sign = new unsigned char[257];
            sign[256] = '\0';
            io->recv_data(sign, 256);
            int res = EVP_VerifyFinal(mctx, sign, 256, sigkey);   
            cout<<"res: "<<res<<endl;
            

            // char *new_message = new char[65]; 
            // new_message[64] = '\0';
            // char *new_gp = new char[33*M+33*N+1];
            // new_gp[33*M+33*N] = '\0';
            // io->recv_data(new_gp, 33*M+33*N);  
            // io->recv_data(new_message, 64);

            // cout<<"gp           "<<strcmp(new_gp, gp)<<endl;
            // cout<<"message          "<<strcmp(new_message, message[i])<<endl;

            // mctx = EVP_MD_CTX_new();
            // EVP_SignInit_ex(mctx, EVP_md5(), NULL);
            // EVP_SignUpdate(mctx, gp, 33*M+33*N);
            // EVP_SignUpdate(mctx, message[i], 64);
            // unsigned int size = 256;
            // unsigned char *sig = new unsigned char[257];
            // res = EVP_SignFinal(mctx, sig, &size, sigprikey);
            // sig[256] = '\0';
            // cout<<"signres "<<res<<" "<<i<<" "<<size<<endl;
            // mctx = EVP_MD_CTX_new();
            // EVP_VerifyInit_ex(mctx, EVP_md5(), NULL);
            // EVP_VerifyUpdate(mctx, gp, 33*M+33*N);
            // EVP_VerifyUpdate(mctx,message[i], 64);
            // res = EVP_VerifyFinal(mctx, sig, 256, sigkey);  
            // cout<<"verify res "<<res<<" "<<i<<endl;

            // cout<<"sign               "<<strcmp((char*)sign, (char*)sig)<<endl;


            if(i == 0){
                char *pchar = new char[33*N+1];
                for(int j=0;j<N;j++){
                    EC_POINT_point2oct(G.ec_group, p[j].point,POINT_CONVERSION_COMPRESSED,
                    (unsigned char*)(pchar+33*j), 33, NULL);
                }
                pchar[33*N] = '\0';
                // write_certificate(N, pchar, i, com_gcch[i], &seed[i], &decom_seed[i], sign);
            }
        }
        // need to verify here and send seed witness; step 4
         

        io->send_data(&rand_j, 4);
        // cout<<"rand_j "<<rand_j<<endl;
        io->send_block(seed_witness, num_pvc);
        unsigned char * in_bob = new unsigned char[(num_input-num_input_alice)*33];
        io->recv_data(in_bob, (num_input-num_input_alice)*33);
        unsigned char * garbledgate = new unsigned char[num_gate*5*33];
        io->recv_data(garbledgate, num_gate*5*33);

        char **com_w_bob = new char*[num_pvc];
        for(int i=0;i<num_pvc;i++){
            com_w_bob[i] = new char[(2*num_input-2*num_input_alice)*32];
            for(int j = 0;j<2*num_input-2*num_input_alice;j++){
                io->recv_data(com_w_bob[i]+32*j,32);
            }
        }
        char **ho = new char*[num_pvc];
        for(int i=0;i<num_pvc;i++){
            ho[i] = new char[32];
            io->recv_data(ho[i], 32);
        }
        block * decom_gcch_j = new block[1];
        io->recv_block(decom_gcch_j,1);
        block ** decom_w_bob = new block* [num_pvc];
        for(int i=0;i<num_pvc;i++){
            decom_w_bob[i] = new block[2*num_input-2*num_input_alice];
            io->recv_block(decom_w_bob[i], 2*num_input-2*num_input_alice);
        }
        for(int i=0;i<num_input-num_input_alice;i++){
            char *hash_one = new char[33];
            char *hash_two = new char[33];
            hash_one[32] = '\0';
            hash_two[32] = '\0';
            hash.put_block(decom_w_bob[rand_j]+2*i);
            hash.put(in_bob+i*33, 33);
            hash.digest(hash_one);
            hash.reset();
            hash.put_block(decom_w_bob[rand_j]+2*i+1);
            hash.put(in_bob+i*33, 33);
            hash.digest(hash_two);
            hash.reset();
            char *hash_recv_one = new char[33];
            char *hash_recv_two = new char[33];
            hash_recv_one[32] = '\0';
            hash_recv_two[32] = '\0';
            memcpy(hash_recv_one, com_w_bob[rand_j]+i*64, 32);
            memcpy(hash_recv_two, com_w_bob[rand_j]+i*64+32, 32);
            int pt = 0;
            if(strcmp(hash_one, hash_recv_one)==0){
                pt++;
            }
            if(strcmp(hash_one, hash_recv_two)==0){
                pt++;
            }
            if(strcmp(hash_two, hash_recv_one)==0){
                pt++;
            }
            if(strcmp(hash_two, hash_recv_two)==0){
                pt++;
            }
            if(pt==0){
                cout<<"wrong bob"<<endl;
            }
        }
        char *com_gcch_j = new char[33];
        com_gcch_j[32] = '\0';
        hash.put_block(decom_gcch_j, 1);
        hash.put(garbledgate, num_gate*5*33);
        for(int j=0;j<num_input-num_input_alice;j++){
            hash.put(com_w_bob[rand_j]+j*64,32);
            hash.put(com_w_bob[rand_j]+j*64+32,32);
        }
        hash.put(ho[rand_j], 32);
        hash.digest(com_gcch_j);
        char *hash_three = new char[33];
        hash_three[32] = '\0';
        memcpy(hash_three, com_gcch[rand_j], 32);
        if(strcmp(com_gcch_j, hash_three)!=0){
            cout<<"wrong gcch"<<endl;
        }
        for(int i=0;i<num_input-num_input_alice;i++){
            memcpy(in+num_input_alice*33+i*33,in_bob+i*33,33);
        }
        // cf.compute(in, &G, rand_t, garbledgate);
        
    }else{
        FILE *keyfile = fopen("prikey.pem", "r");
        RSA *rsa_pri = PEM_read_RSAPrivateKey( keyfile, NULL, NULL, NULL) ;
        EVP_PKEY *sigprikey;
        sigprikey = EVP_PKEY_new();
        EVP_PKEY_set1_RSA(sigprikey, rsa_pri);
        RSA_free(rsa_pri);

        FILE *keypubfile = fopen("pubkey.pem", "r");
        RSA *rsa_pub = PEM_read_RSAPublicKey( keypubfile, NULL, NULL, NULL) ;
        EVP_PKEY *sigpubkey;
        sigpubkey = EVP_PKEY_new();
        EVP_PKEY_set1_RSA(sigpubkey, rsa_pub);
        RSA_free(rsa_pub);

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

        // cout<<"start zk"<<endl;
        bool zk_bool =zk_EncEP_v(&G, h, generators, ca, cb, M, N, io);
        
        Point recommit;
        io->recv_pt(&G,&recommit);
        BigInt zk_dh_c;
        G.get_rand_bn(zk_dh_c);
        unsigned char* szk_dh_c = new unsigned char[32];
        zk_dh_c.to_bin(szk_dh_c);
        io->send_data(szk_dh_c, 32);
        unsigned char* respon_s = new unsigned char[32];
        io->recv_data(respon_s, 32);
        BigInt respon_s_bn;
        respon_s_bn.from_bin(respon_s, 32);
        Point test_s_a, test_s_b;
        test_s_a = G.mul_gen(respon_s_bn);
        test_s_b = h.mul(zk_dh_c);
        test_s_b = test_s_b.add(recommit);
        if(test_s_a == test_s_b){
            cout<<"okok"<<endl;
        }

        mid = clock();
        cout<<"initial phase time = "<<double(mid-setup)/CLOCKS_PER_SEC<<"s"<<endl; 
        cout<<"io              "<<io->counter<<endl;

        // cout<<"finish zk"<<endl;
    
        char * com_seed_a = new char[32*num_pvc];
        io->recv_data(com_seed_a, 32*num_pvc);
        pvc_start = clock();
        PRG prg;
        block *seed = new block[num_pvc];
        block *witness = new block[num_pvc];
        prg.random_block(seed, num_pvc);
        prg.random_block(witness, num_pvc);
        iknp->send(seed, witness, num_pvc);

        unsigned char **choose_w = new unsigned char*[num_pvc];
        unsigned char **choose_alpha = new unsigned char*[num_pvc];
        unsigned char **garbledGates = new unsigned char*[num_pvc];
        unsigned char **a_oct = new unsigned char*[num_pvc];
        unsigned char **b_oct = new unsigned char*[num_pvc];
        ECDSA_SIG *sig[num_pvc];

        // cout<<"?????????????????????"<<endl;
        // cout<<"for what"<<endl;
// #pragma omp parallel for  num_threads(4) 
        for(int i=0;i<num_pvc;i++){
            // printf("this is No.%d Thread ,i=%d\n", omp_get_thread_num(), i);
            choose_w[i] = new unsigned char[33*2*num_output];
            choose_alpha[i] = new unsigned char[64];
            garbledGates[i] = new unsigned char[num_gate*5*33];
            a_oct[i]= new unsigned char[2*(M+num_output)*33];
            b_oct[i] = new unsigned char[2*N*33];
            prg.reseed(&seed[i]);
            prg.random_data(choose_w[i], 33*2*num_output);
            prg.random_data(choose_alpha[i], 64);
            BigInt alpha_a, alpha_b;
            alpha_a.from_bin(choose_alpha[i], 32);
            alpha_b.from_bin(choose_alpha[i]+32 , 32);
            Point a_point[2*M],b_point[2*N]; 
            size_t oct_len = 33;
            G.get_rand_bn(alpha_a);
            G.get_rand_bn(alpha_b);
            for(int j=0;j<M;j++){
                // cout<<"bbbbbbbbbbbbb"<<j<<endl;
                a_point[2*j] = generators[j].mul(alpha_a);
                a_point[2*j+1] = generators[j].mul(alpha_b);
                // cout<<"bbbbbbbbbbbbb"<<j<<endl;
                EC_POINT_point2oct(G.ec_group, a_point[2*j].point,POINT_CONVERSION_COMPRESSED,
                                        a_oct[i]+2*j*33, oct_len, NULL);
                EC_POINT_point2oct(G.ec_group, a_point[2*j+1].point,POINT_CONVERSION_COMPRESSED,
                                        a_oct[i]+(2*j+1)*33, oct_len, NULL);
                // cout<<"bbbbbbbbbbbbb"<<j<<endl;
            }
            for(int j=0;j<N;j++){
                // cout<<"aaaaaaaaaaa"<<j<<endl;
                b_point[2*j] = p[j].mul(alpha_a);
                // cout<<"aaaaaaaaaaa"<<j<<endl;
                b_point[2*j+1] = p[j].mul(alpha_b);
                // cout<<"aaaaaaaaaaa"<<j<<endl;
                EC_POINT_point2oct(G.ec_group, b_point[2*j].point,POINT_CONVERSION_COMPRESSED,
                                        b_oct[i]+2*j*33, oct_len, NULL);
                // cout<<"aaaaaaaaaaa"<<j<<endl;
                EC_POINT_point2oct(G.ec_group, b_point[2*j+1].point,POINT_CONVERSION_COMPRESSED,
                                        b_oct[i]+(2*j+1)*33, oct_len, NULL);
                // cout<<"aaaaaaaaaaa"<<j<<endl;
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
            cout<<"start construct"<<endl;
            CircuitExecution::circ_exec = new GarbledCircuitsGen(); 
            construct_pfegarbledgates(a_oct[i]+2*num_input*33,b_oct[i],garbledGates[i], num_gate, num_input);
            delete CircuitExecution::circ_exec;
        }
        cout<<"finish garbled circuits"<<endl;
        for(int i=0;i<num_pvc;i++){
            for(int j=0;j<num_input_alice;j++){
                block* alice_block_a = new block[3];
                char_to_block(alice_block_a,&a_oct[i][66*j]); 
                block* alice_block_b = new block[3];
                char_to_block(alice_block_b,&a_oct[i][66*j+33]); 

                iknp->send(alice_block_a, alice_block_b, 3);
            }
        }
        // cout<<"alice input send"<<endl;
        block **decom_w_bob = new block*[num_pvc];
        char **com_w_bob = new char*[num_pvc];
        Hash hash;
        char **ho = new char*[num_pvc];
        for(int i=0;i<num_pvc;i++){
            decom_w_bob[i] = new block[2*num_input-2*num_input_alice];
            prg.random_block(decom_w_bob[i], 2*num_input-2*num_input_alice);
            com_w_bob[i] = new char[(2*num_input-2*num_input_alice)*32];
            for(int j=0;j<num_input-num_input_alice;j++){
                hash.put_block(decom_w_bob[i]+2*j);
                hash.put(a_oct[i]+num_input_alice*66+j*66, 33);
                hash.digest(com_w_bob[i]+j*64);
                hash.reset();
                hash.put_block(decom_w_bob[i]+2*j+1);
                hash.put(a_oct[i]+num_input_alice*66+j*66+33, 33);
                hash.digest(com_w_bob[i]+j*64+32);
                hash.reset();
            }
        }
        for(int i=0;i<num_pvc;i++){
            ho[i] = new char[32];
            hash.put(choose_w[i], 33*2*num_output);
            hash.digest(ho[i]);
            hash.reset();
        }

        block *decom_gcch = new block[num_pvc];
        char **com_gcch = new char*[num_pvc];
        block *rand_order_wb = new block[1];
        prg.random_block(rand_order_wb);
        prg.random_block(decom_gcch, num_pvc);
        bool* rand_order_wb_b = new bool[128];
        memcpy(rand_order_wb_b, rand_order_wb, 16);
        for(int i=0;i<num_pvc;i++){
            com_gcch[i] = new char[32];
            hash.put_block(decom_gcch+i, 1);
            hash.put(garbledGates[i], num_gate*5*33*sizeof(char));
            for(int j=0;j<num_input-num_input_alice;j++){
                if(rand_order_wb_b[j%128]){
                    hash.put(com_w_bob[i]+j*64,32);
                    hash.put(com_w_bob[i]+j*64+32,32);
                }else{
                    hash.put(com_w_bob[i]+j*64+32,32);
                    hash.put(com_w_bob[i]+j*64,32);
                }
            }
            hash.put(ho[i], 32);
            hash.digest(com_gcch[i]);
            hash.reset();
        }
        // cout<<"com gcch"<<endl;

        
        char **message = new char*[num_pvc]; 
        char *gp = new char[33*M+33*N+1];
        for(int j=0;j<M;j++){
            EC_POINT_point2oct(G.ec_group, generators[j].point,POINT_CONVERSION_COMPRESSED,
            (unsigned char*)(gp+33*j), 33, NULL);
        }
        for(int j=0;j<N;j++){
            EC_POINT_point2oct(G.ec_group, p[j].point,POINT_CONVERSION_COMPRESSED,
            (unsigned char*)(gp+33*M+33*j), 33, NULL);
        }
        for(int i=0;i<num_pvc;i++){\
            io->send_data(com_gcch[i], 32);
        }

        pvc_end = clock();
        cout<<"pvc time = "<<double(pvc_end-pvc_start)/CLOCKS_PER_SEC<<"s"<<endl; 
        gp[33*M+33*N] = '\0';
        for(int i=0;i<num_pvc;i++){
            message[i] = new char[64+1];
            message[i][64] = '\0';
            memcpy(message[i], com_seed_a+32*i, 32);
            memcpy(message[i]+32, com_gcch[i], 32);
            auto mctx = EVP_MD_CTX_new();
            EVP_SignInit_ex(mctx, EVP_md5(), NULL);
            EVP_SignUpdate(mctx, gp, 33*M+33*N);
            EVP_SignUpdate(mctx, message[i], 64);
            unsigned int size = 256;
            unsigned char *sig = new unsigned char[257];
            int res = EVP_SignFinal(mctx, sig, &size, sigprikey);
            sig[256] = '\0';
            // cout<<"signres "<<res<<" "<<i<<" "<<size<<endl;
            io->send_data(sig, 256);

            // io->send_data(gp, 33*M+33*N);  
            // io->send_data(message[i], 64);

            // mctx = EVP_MD_CTX_new();
            // EVP_VerifyInit_ex(mctx, EVP_md5(), NULL);
            // EVP_VerifyUpdate(mctx, gp, 33*M+33*N);
            // EVP_VerifyUpdate(mctx,message[i], 64);
            // res = EVP_VerifyFinal(mctx, sig, 256, sigpubkey);  
            // cout<<"verify res "<<res<<" "<<i<<endl;
        }
        
        
            
        int rand_j;
        block *seed_witness = new block[num_pvc];
        io->recv_data(&rand_j, 4);
        // cout<<"rand_j "<<rand_j<<endl;
        io->recv_block(seed_witness, num_pvc);
        bool *input_bob = new bool[num_input-num_input_alice];
        for(int i=0;i<num_input-num_input_alice;i++){
            input_bob[i] = false;
        }
        unsigned char * in_bob = new unsigned char[(num_input-num_input_alice)*33];
        for(int i=0;i<num_input-num_input_alice;i++){
            memcpy(in_bob+ 33*i, a_oct[rand_j]+num_input_alice*66+i*66, 33);
        }
        io->send_data(in_bob, (num_input-num_input_alice)*33);
        io->send_data(garbledGates[rand_j], num_gate*5*33);

        for(int i=0;i<num_pvc;i++){
            for(int j=0;j<num_input-num_input_alice;j++){
                if(rand_order_wb_b[j%128]){
                    io->send_data(com_w_bob[i]+j*64,32);
                    io->send_data(com_w_bob[i]+j*64+32,32);
                }else{
                    io->send_data(com_w_bob[i]+j*64+32,32);
                    io->send_data(com_w_bob[i]+j*64,32);
                }
            }
        }
        for(int i=0;i<num_pvc;i++){
            io->send_data(ho[i], 32);
        }
        io->send_block(decom_gcch+rand_j,1);
        for(int i=0;i<num_pvc;i++){
            io->send_block(decom_w_bob[i], 2*num_input-2*num_input_alice);
        }
    }  
    end = clock();
    cout<<"io              "<<io->counter<<endl;
    cout<<"evaluation time = "<<double(end-mid)/CLOCKS_PER_SEC<<"s"<<endl; 
    cout<<"all time = "<<double(end-setup)/CLOCKS_PER_SEC<<"s"<<endl; 
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
