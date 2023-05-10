#include <iostream>
#include "pfe/pfe.h"
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <fstream>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>
#include <openssl/buffer.h>
#include <openssl/pem.h>
using namespace std;
using namespace emp;

int main(){
    ifstream fin("./certificate.txt");
    int n;
    char* pchar = new char[33*n+1];
    int j;
    cout<<"start"<<endl;
    unsigned char *com_gcch = new unsigned char[33];
    com_gcch[32] = '\0';
    pchar[33*n] = '\0';
    block *seed = new block;
    block *decom_seed = new block;
    unsigned char * sign = new unsigned char[257];
    sign[256] = '\0';
    fin>>n;
    for(int i=0;i<n*33;i++){
        int t;
        fin>>t;
        pchar[i] = t;
    }
    fin>>j;
    for(int i=0;i<32;i++){
        int t;
        fin>>t;
        com_gcch[i] = t;
    } 
    int* tmp = new int[4];
    for(int i=0;i<4;i++){
        fin>>tmp[i];
    }
    memcpy(seed, tmp, sizeof(int)*4);
    for(int i=0;i<4;i++){
        fin>>tmp[i];
    }
    memcpy(decom_seed, tmp, sizeof(int)*4);

    for(int i=0;i<256;i++){
        int t;
        fin>>t;
        sign[i] = t;
    }
    cout<<"finish one"<<endl;
    ifstream fcrs("./crs.txt");
    int m;
    fcrs>>m;
    cout<<"n: "<<n<<endl;
    cout<<"m: "<<m<<endl;
    char* gchar = new char[33*m+1];
    gchar[33*m] = '\0';
    for(int i=0;i<m*33;i++){
        int t;
        fcrs>>t;
        gchar[i] = t;
    }
    cout<<"????"<<endl;
    // Group G;
    // EC_POINT* popo = EC_POINT_new(G.ec_group);
    // cout<<"adsafas"<<endl;
    // int rev = EC_POINT_oct2point(G.ec_group, popo, (unsigned char*)pchar, 33, NULL);
    // cout<<rev<<endl;
    // for(int i=0;i<n;i++){
    //     EC_POINT* popo = EC_POINT_new(G.ec_group);
    //     cout<<"adsafas"<<endl;
    //     int rv = EC_POINT_oct2point(G.ec_group, popo, (unsigned char*)pchar+i*33, 33, NULL);
    //     cout<<rv<<endl;
    // }
    // cout<<"?????"<<endl;
    // for(int i=0;i<m;i++){
    //     generators[i].point = EC_POINT_new(G.ec_group);
    //     EC_POINT_oct2point(G.ec_group, generators[i].point, (unsigned char*)gchar+i*33, 33, NULL);
    // }
    cout<<"finish two"<<endl;

    FILE *keypubfile = fopen("pubkey.pem", "r");
    RSA *rsa_pub = PEM_read_RSAPublicKey( keypubfile, NULL, NULL, NULL) ;
    EVP_PKEY *sigpubkey;
    sigpubkey = EVP_PKEY_new();
    cout<<"pubkey: "<<EVP_PKEY_set1_RSA(sigpubkey, rsa_pub);
    RSA_free(rsa_pub);

    cout<<"finish pri"<<endl;

    // FILE *keyfile = fopen("prikey.pem", "r");
    // RSA *rsa_pri = PEM_read_RSAPrivateKey( keyfile, NULL, NULL, NULL) ;
    // EVP_PKEY *sigprikey;
    // sigprikey = EVP_PKEY_new();
    // int re = EVP_PKEY_set1_RSA(sigprikey, rsa_pri);
    // cout<<"re "<<re<<endl;
    // RSA_free(rsa_pri);

    cout<<"finish three  "<<endl;
    Hash hash;
    char *com_seed_a = new char[33];
    com_seed_a[32] = '\0';
    hash.put_block(decom_seed);
    hash.put_block(seed);
    hash.digest(com_seed_a);
    hash.reset();

    char *message = new char[65];
    memcpy(message, com_seed_a, 32);
    memcpy(message+32, com_gcch, 32);
    message[64] = '\0';
    auto mctx = EVP_MD_CTX_new();
    // EVP_SignInit_ex(mctx, EVP_md5(), NULL);
    // EVP_SignUpdate(mctx, gchar, 33*m);
    // EVP_SignUpdate(mctx, pchar, 33*n);
    // EVP_SignUpdate(mctx, message, 64);
    // unsigned int size = 256;
    // unsigned char *sig = new unsigned char[257];
    // int res = EVP_SignFinal(mctx, sig, &size, sigprikey);
    // sig[256] = '\0';
    // cout<<"signres "<<res<<" "<<" "<<size <<endl;
    // mctx = EVP_MD_CTX_new();
    EVP_VerifyInit_ex(mctx, EVP_md5(), NULL);
    EVP_VerifyUpdate(mctx, gchar, 33*m);
    EVP_VerifyUpdate(mctx, pchar, 33*n);
    EVP_VerifyUpdate(mctx, message, 64);
    for(int i=0;i<256;i++){
        cout<<(int)sign[i]<<endl;
    }
    cout<<"????"<<endl;
    
    // int res = EVP_VerifyFinal(mctx, sign, 256, sigpubkey); 
    // cout<<"res: "<<res<<endl;

}