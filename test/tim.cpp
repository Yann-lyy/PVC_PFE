#include <iostream>
#include "pfe/pfe.h"
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <fstream>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>
#include <openssl/buffer.h>
#include <openssl/pem.h>
#include <time.h>
using namespace std;
using namespace emp;
int main(){
    
    int M=4000;
    Group G;
    // int *gate_arr = new int[30000000];
    // EC_POINT* generatorsq[3000000];
    Point generators[10000];
    // block * decom_gcch_j = new block[30000000];
    unsigned char *gp = new unsigned char[100000000];

    
    int N=1000;
    BigInt d;
    BigInt order;
    BN_CTX * bn_ctx = nullptr;
    EC_GROUP_get_order(G.ec_group, order.n, bn_ctx);
    clock_t start,end;
    G.get_rand_bn(d);
    for(int i=0;i<10000;i++){
        generators[i] = G.mul_gen(d);
    }
    cout<<"size of point "<<sizeof(generators[0].point)<<endl;
    cout<<"size of bigint "<<sizeof(d.n)<<endl;
    start = clock();
    while(N--){
        for(int k=0;k<M;k++){
            BN_mod_mul(d.n, d.n, d.n, order.n, G.bn_ctx);
            BN_mod_mul(d.n, d.n, d.n, order.n, G.bn_ctx);
            BN_mod_add(d.n, d.n, d.n, order.n, G.bn_ctx);
        }
    }
    // for(int j=0;j<10000;j++){
    //     generators[0].to_bin(gp+33*j, 33);
    //     generators[1].from_bin(&G, gp+33*j, 33);
    // }
    end = clock();
    cout<<"time to bin = "<<double(end-start)/CLOCKS_PER_SEC<<"s"<<endl; 
}
