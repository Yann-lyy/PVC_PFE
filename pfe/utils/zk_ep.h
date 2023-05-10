#ifndef ZK_EP_H__
#define ZK_EP_H__

#include <iostream>
#include <time.h>
#include <map>
#include <cstring>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>
using namespace std;
using namespace emp;
/*
Protocol zk EncEP
*/

class Place
{
public:
    int a,b;
    bool operator < (const Place &p) const
    {   
        if(a == p.a){
            return  b < p.b;
        }
        return a < p.a;
    }
    bool operator == (const Place &p) const
    {   
        return a == p.a && b == p.b;
    }
};

bool zk_EncEP_p(Group * G, Point h, Point * generators, Point * ca, Point * cb, vector<int> ep, BigInt * r, int M, int N, NetIO *io){
    
    // cout<<"in"<<endl;

    
    clock_t start,mid,end,one,two,three,four,five,six,seven,eight,nine,ten,eleven,twive,thirteen;
    start = clock();
    // cout<<"step one"<<M<<" "<<N<<endl;
    Point tmp_p;
    BigInt tmp_power;
    BigInt tmp_bi;
    //step one
    BigInt order;
    EC_GROUP_get_order(G->ec_group, order.n, G->bn_ctx);
    
    // for(int i=0;i<32;i++){
    //     cout<<(int)ococ[i]<<" ";
    // }
    // cout<<endl;
    map<Place, BigInt> e_matrix;
    BigInt bn_one;
    BN_one(bn_one.n);

    for(int i=0;i<N;i++){
        Place place0;
        place0.a = i+1;
        place0.b = ep[i];
        e_matrix.insert(pair<Place, BigInt>(place0, bn_one));
    }
    // BigInt **old_e_matrix = new BigInt*[N+2];
    // old_e_matrix[0] = new BigInt[M];
    // for(int i=0;i<N;i++){
    //     old_e_matrix[i+1] = new BigInt[M];
    //     BN_one(old_e_matrix[i+1][ep[i]].n);
    // }
    // old_e_matrix[N+1] = new BigInt[M];
    //get ei(from 1 to N)

    //step two
    // cout<<"step two"<<endl;
    
    BigInt W;
    BigInt *w = new BigInt[N];
    unsigned char * w_char = new unsigned char[32];
    io->recv_data(w_char, 32);
    w[0].from_bin(w_char,32);
    BN_one(tmp_power.n);
    for(int i=1;i<N;i++){
        BN_add(tmp_power.n, tmp_power.n, BN_value_one());
        BN_mod_exp(w[i].n, w[0].n, tmp_power.n, order.n, G->bn_ctx);
    }
    //rand w and compute w^i

    // cout<<"step three"<<endl;

    Point Ca, Cb;
    Ca = ca[0].mul(w[0]);
    Cb = cb[0].mul(w[0]);
    for(int i=1;i<N;i++){
        tmp_p = ca[i].mul(w[i]);
        Ca = Ca.add(tmp_p);
        tmp_p = cb[i].mul(w[i]);
        Cb = Cb.add(tmp_p);
    }

    // cout<<"step four"<<endl;

    //compute C

    BigInt *e_array = new BigInt[M];
    BigInt re;
    for(int i=0;i<N;i++){
        BN_mod_add(W.n, W.n, w[i].n, order.n, G->bn_ctx);
        BN_mod_mul(tmp_bi.n, r[i].n, w[i].n, order.n, G->bn_ctx);
        BN_mod_add(re.n, re.n, tmp_bi.n, order.n, G->bn_ctx);
        BN_mod_add(e_array[ep[i]].n, e_array[ep[i]].n, w[i].n, order.n, G->bn_ctx);
    }

    // cout<<"?123"<<endl; 
    //compute W, re = sum(r[i]*w^i), e = ei*w^i

    Point ce = Cb;
    Point ce_p = G->mul_gen(W);
    BigInt rho_e, rho_e_p;
    BN_zero(rho_e.n);
    BN_zero(rho_e_p.n);
    int l_p = M;
    BigInt y_p, e_p, rand_u;
    Point u_receive[1], u;
    io->recv_pt(G, u_receive);
    u = u_receive[0];

    Point g_p_result[l_p+l_p%2];
    BigInt *y_p_result = new BigInt[l_p+l_p%2];
    BigInt *e_p_result = new BigInt[l_p+l_p%2];
    for(int i=0;i<l_p;i++){
        g_p_result[i] = generators[i];
        BN_one(y_p_result[i].n);
        e_p_result[i] = e_array[i];
    }

    //loop
    while(l_p!=1){    
        // cout<<"l_p:          "<<l_p<<endl;                                                                                                                                                        
        BigInt rho_l, rho_r, rho_l_p, rho_r_p;
        int pad = l_p%2;
        Point g_p[pad+l_p];
        BigInt *y_p = new BigInt[pad+l_p];
        BigInt *e_p = new BigInt[pad+l_p];
        for(int i=0;i<l_p;i++){
            g_p[i] = g_p_result[i];
            y_p[i] = y_p_result[i];
            e_p[i] = e_p_result[i];
        }
        if(pad == 1){
            G->get_rand_bn(tmp_bi);
            g_p[l_p]=G->mul_gen(tmp_bi);
            io->send_pt(g_p+l_p, 1);
            BN_zero(y_p[l_p].n);
            BN_zero(e_p[l_p].n);
        }

        // pad 

        int mid = (l_p + pad)/2;
        G->get_rand_bn(rho_l);
        G->get_rand_bn(rho_r);
        G->get_rand_bn(rho_l_p);
        G->get_rand_bn(rho_r_p);

        Point v_l, v_r, v_l_p, v_r_p;
        v_l = u.mul(rho_l);
        v_r = u.mul(rho_r);
        v_l_p = u.mul(rho_l_p);
        v_r_p = u.mul(rho_r_p);
        for(int i=0;i<mid;i++){
            tmp_p = g_p[mid+i].mul(e_p[i]);
            v_l = v_l.add(tmp_p);
            tmp_p = g_p[i].mul(e_p[mid+i]);
            v_r = v_r.add(tmp_p);
            BigInt tmp_bn_l_p, tmp_bn_r_p;
            BN_mod_mul(tmp_bn_l_p.n, y_p[mid+i].n, e_p[i].n, order.n, G->bn_ctx);
            BN_mod_mul(tmp_bn_r_p.n, y_p[i].n, e_p[mid+i].n, order.n, G->bn_ctx);
            tmp_p = G->mul_gen(tmp_bn_l_p);
            v_l_p = v_l_p.add(tmp_p);
            tmp_p = G->mul_gen(tmp_bn_r_p);
            v_r_p = v_r_p.add(tmp_p);
        }
        Point v_send[4] = {v_l, v_r, v_l_p, v_r_p};
        io->send_pt(v_send, 4);
        //compute vl, vr, vlp, vrp

        BigInt alpha, alpha_inverse;
        unsigned char * alpha_char = new unsigned char[32];
        io->recv_data(alpha_char, 32);
        alpha.from_bin(alpha_char,32);
        BN_mod_inverse(alpha_inverse.n, alpha.n, order.n, G->bn_ctx); 
        for(int i=0;i<mid;i++){
            BigInt tmp_l, tmp_r;
            BN_mod_mul(tmp_l.n, alpha.n, e_p[i].n, order.n, G->bn_ctx);
            BN_mod_mul(tmp_r.n, alpha_inverse.n, e_p[mid+i].n, order.n, G->bn_ctx);
            BN_mod_add(e_p_result[i].n, tmp_l.n, tmp_r.n, order.n, G->bn_ctx);
        }
        //e_p

        BigInt alpha_square, alpha_square_inverse;
        BN_mod_mul(alpha_square.n, alpha.n, alpha.n, order.n, G->bn_ctx);
        BN_mod_inverse(alpha_square_inverse.n, alpha_square.n, order.n, G->bn_ctx);

        BN_mod_mul(tmp_bi.n, rho_l.n, alpha_square.n,order.n, G->bn_ctx);
        BN_mod_add(rho_e.n, rho_e.n, tmp_bi.n, order.n, G->bn_ctx);
        BN_mod_mul(tmp_bi.n, rho_r.n, alpha_square_inverse.n,order.n, G->bn_ctx);
        BN_mod_add(rho_e.n, rho_e.n, tmp_bi.n, order.n, G->bn_ctx);

        BN_mod_mul(tmp_bi.n, rho_l_p.n, alpha_square.n,order.n, G->bn_ctx);
        BN_mod_add(rho_e_p.n, rho_e_p.n, tmp_bi.n, order.n, G->bn_ctx);
        BN_mod_mul(tmp_bi.n, rho_r_p.n, alpha_square_inverse.n,order.n, G->bn_ctx);
        BN_mod_add(rho_e_p.n, rho_e_p.n, tmp_bi.n, order.n, G->bn_ctx);
        //rho e, rho e p

        tmp_p = v_l.mul(alpha_square);
        ce = ce.add(tmp_p);
        tmp_p = v_r.mul(alpha_square_inverse);
        ce = ce.add(tmp_p);

        tmp_p = v_l_p.mul(alpha_square);
        ce_p = ce_p.add(tmp_p);
        tmp_p = v_r_p.mul(alpha_square_inverse);
        ce_p = ce_p.add(tmp_p);
        //ce, cep

        for(int i=0;i<mid;i++){
            tmp_p = g_p[mid+i].mul(alpha);
            g_p_result[i] = g_p[i].mul(alpha_inverse);
            g_p_result[i] = g_p_result[i].add(tmp_p);
            BN_mod_mul(y_p_result[i].n, alpha_inverse.n, y_p[i].n, order.n, G->bn_ctx);
            BN_mod_mul(tmp_bi.n, alpha.n, y_p[mid+i].n, order.n, G->bn_ctx);
            BN_mod_add(y_p_result[i].n, y_p_result[i].n, tmp_bi.n, order.n, G->bn_ctx);
        }
        io->send_pt(g_p_result, 1);



        
        l_p = mid;   
    }
    // cout<<"finish p "<<endl;  
    BigInt e_bar, y_bar;
    Point g_bar;
    g_bar = g_p_result[0];
    e_bar = e_p_result[0];
    y_bar = y_p_result[0];
    io->send_pt(g_p_result, 1);
    Point gamma = G->mul_gen(y_bar);
    BigInt x_1, x_2, x_3, x_4;
    G->get_rand_bn(x_1);
    G->get_rand_bn(x_2);
    G->get_rand_bn(x_3);
    G->get_rand_bn(x_4);
    Point a[3];
    a[0] = g_bar.mul(x_1);
    tmp_p = u.mul(x_2);
    a[0] = a[0].add(tmp_p);
    tmp_p = h.mul(x_3);
    a[0] = a[0].add(tmp_p);
    a[1] = gamma.mul(x_1);
    tmp_p = u.mul(x_4);
    a[1] = a[1].add(tmp_p);
    a[2] = G->mul_gen(x_3);
    io->send_pt(a, 3);

    // cout<<"p one"<<endl; 

    BigInt z_1, z_2, z_3, z_4, alpha;
    unsigned char * alpha_char = new unsigned char[32];
    io->recv_data(alpha_char, 32);
    alpha.from_bin(alpha_char,32);
    BN_mod_mul(z_1.n, alpha.n, e_bar.n, order.n, G->bn_ctx);
    BN_mod_add(z_1.n, z_1.n, x_1.n, order.n, G->bn_ctx);
    BN_mod_mul(z_2.n, alpha.n, rho_e.n, order.n, G->bn_ctx); 
    BN_mod_add(z_2.n, z_2.n, x_2.n, order.n, G->bn_ctx);
    BN_mod_mul(z_3.n, alpha.n, re.n, order.n, G->bn_ctx); 
    BN_mod_add(z_3.n, z_3.n, x_3.n, order.n, G->bn_ctx);
    BN_mod_mul(z_4.n, alpha.n, rho_e_p.n, order.n, G->bn_ctx);  
    BN_mod_add(z_4.n, z_4.n, x_4.n, order.n, G->bn_ctx);


    unsigned char* z_1_char = new unsigned char[32];
    z_1.to_bin(z_1_char);
    io->send_data(z_1_char,32);
    unsigned char* z_2_char = new unsigned char[32];
    z_2.to_bin(z_2_char);
    io->send_data(z_2_char,32);
    unsigned char* z_3_char = new unsigned char[32];
    z_3.to_bin(z_3_char);
    io->send_data(z_3_char,32);
    unsigned char* z_4_char = new unsigned char[32];
    z_4.to_bin(z_4_char);
    io->send_data(z_4_char,32);

    //zk sum


    // cout<<"zk zero p "<<endl;

    mid = clock();
    cout<<"zk sum time = "<<double(mid-start)/CLOCKS_PER_SEC<<"s"<<endl; 
    //zk zero
    one = clock();

    BigInt *x = new BigInt[N];
    BigInt *y = new BigInt[M];
    unsigned char *xy_char = new unsigned char[32];
    io->recv_data(xy_char, 32);
    x[0].from_bin(xy_char, 32);
    io->recv_data(xy_char, 32);
    y[0].from_bin(xy_char, 32);
    BN_one(tmp_power.n);
    for(int i=1;i<N;i++){
        BN_add(tmp_power.n, tmp_power.n, BN_value_one());
        BN_mod_exp(x[i].n, x[0].n, tmp_power.n, order.n, G->bn_ctx);
    }
    BN_one(tmp_power.n);
    for(int i=1;i<M;i++){
        BN_add(tmp_power.n, tmp_power.n, BN_value_one());
        BN_mod_exp(y[i].n, y[0].n, tmp_power.n, order.n, G->bn_ctx);
    }
    two = clock();
    // cout<<"zk zero step one"<<double(two-one)/CLOCKS_PER_SEC<<"s"<<endl;
    // randomly choose x,y
    Point cd_a[N+2], cd_b[N+2];
    cd_a[0] = ca[0].mul(x[0]);
    cd_b[0] = cb[0].mul(x[0]);
    cd_a[N] = cd_a[0];
    cd_b[N] = cd_b[0];
    for (int i = 1; i < N; i++)
    {
        cd_a[i] = ca[i].mul(x[i]);
        cd_b[i] = cb[i].mul(x[i]);
        cd_a[N] = cd_a[N].add(cd_a[i]);
        cd_b[N] = cd_b[N].add(cd_b[i]);
    }
    BigInt minus_one, zero;
    BN_zero(zero.n);
    BN_mod_sub(minus_one.n, zero.n, BN_value_one(), order.n, G->bn_ctx);

    
    Point ce_a[N+2], ce_b[N+2];
    for(int i = 0;i<N;i++){
        ce_a[i+1] = ca[i];
        ce_b[i+1] = cb[i];
    }
    three = clock();
    // cout<<"zk zero step two "<<double(three-two)/CLOCKS_PER_SEC<<"s"<<endl;

    map<Place, BigInt> d_i;
    // BigInt **old_d_i = new BigInt*[N+2];
    // for(int i=0;i<N+2;i++){
    //     old_d_i[i] = new BigInt[M];
    // }

    BigInt *rd_i = new BigInt[N+2];
    BigInt *re_i = new BigInt[N+2];
    // cout<<"later"<<endl;
    // for(int i=0;i<N;i++){
    //     for(int j=0;j<M;j++){
    //         BN_mod_mul(old_d_i[i][j].n, old_e_matrix[i+1][j].n, x[i].n, order.n, G->bn_ctx);
    //         BN_mod_add(old_d_i[N][j].n, old_d_i[N][j].n, old_d_i[i][j].n, order.n, G->bn_ctx);
    //     }
    // }

    for(auto iter = e_matrix.begin(); iter != e_matrix.end(); ++ iter){
        Place p;
        p.a = iter->first.a-1; 
        p.b = iter->first.b;
        d_i.insert(pair<Place, BigInt>(p, x[p.a]));
        Place pn;
        pn.a = N; 
        pn.b = p.b;
        auto iterfind = d_i.find(pn);
        if(iterfind == d_i.end()){
            d_i.insert(pair<Place, BigInt>(pn, x[p.a]));
        }else{
            BN_mod_add(iterfind->second.n, iterfind->second.n, x[p.a].n, order.n, G->bn_ctx);
        }
    }

    // for(int i=0;i<M;i++){
    //     Place pn;
    //     pn.a = N; 
    //     pn.b = i;
    //     auto iterfind = d_i.find(pn);
    //     if(iterfind != d_i.end()){
    //         if(BN_cmp(iterfind->second.n, old_d_i[N][i].n)==0){
    //             cout<<"     "<<i<<endl;
    //         }
    //     }
    // }


    for(int i=0;i<N;i++){
        BN_mod_mul(rd_i[i].n, r[i].n, x[i].n, order.n, G->bn_ctx);
        BN_mod_add(rd_i[N].n, rd_i[i].n, rd_i[N].n, order.n, G->bn_ctx);
        re_i[i+1]=r[i];
    }
    for(int i=0;i<M;i++){
        Place p;
        p.a = N+1;
        p.b = i;
        e_matrix.insert(pair<Place, BigInt>(p,minus_one));
    }
    // for(int i=0;i<M;i++){
    //     old_e_matrix[N+1][i] = minus_one;
    // }
    four = clock();
    // cout<<"step 2.3"<<double(four-three)/CLOCKS_PER_SEC<<"s"<<endl;
    ce_a[N+1] = G->mul_gen(re_i[N+1]);
    ce_b[N+1] = generators[0].mul(minus_one);
    for(int i=1;i<M;i++){
        tmp_p = generators[i].mul(minus_one);
        ce_b[N+1] = ce_b[N+1].add(tmp_p);
    }
 
    cout<<"step 2.5"<<endl;
    int l = N+1;

    for(int i=0;i<M;i++){
        Place p;
        p.a = 0;
        p.b = i;
        BigInt rbia, rbib;
        G->get_rand_bn(rbia);
        G->get_rand_bn(rbib);
        e_matrix.insert(pair<Place, BigInt>(p,rbia));
        p.a = N+1;
        d_i.insert(pair<Place, BigInt>(p,rbib));
        // old_e_matrix[0][i]=rbia;
        // old_d_i[N+1][i]=rbib;
    }
    G->get_rand_bn(re_i[0]);
    G->get_rand_bn(rd_i[N+1]);
    
    ce_a[0]=G->mul_gen(re_i[0]);
    ce_b[0]=h.mul(re_i[0]); 
    for(int i=0;i<M;i++){
        Place p;
        p.a = 0;
        p.b = i;
        BigInt tmp_p_bi = e_matrix[p];
        tmp_p = generators[i].mul(tmp_p_bi);
        ce_b[0]=ce_b[0].add(tmp_p);
    }

    cd_a[N+1]=G->mul_gen(rd_i[N+1]);
    cd_b[N+1]=h.mul(rd_i[N+1]);
    for(int i=0;i<M;i++){
        Place p;
        p.a = N+1;
        p.b = i;
        BigInt tmp_p_bi = d_i[p];
        tmp_p = generators[i].mul(tmp_p_bi);
        cd_b[N+1]=cd_b[N+1].add(tmp_p);
    }   
    
    for(int i=0;i<l;i++){
        tmp_p = G->mul_gen(rd_i[N]);
        if(!(cd_a[N] == tmp_p)){
            cout<<"ttttttttttttttttttrrrrrrrrrrrue:"<<endl;
        }
    }
    five = clock();
    cout<<"zk zero step three"<<double(five-four)/CLOCKS_PER_SEC<<"s"<<endl;
    

    BigInt *d_phi = new BigInt[2*l+1];
    int aaaaaaaaaaaa=0;
    for(auto itr_e = e_matrix.begin(); itr_e != e_matrix.end(); ++ itr_e){ 
        for(auto itr_d = d_i.begin(); itr_d != d_i.end(); ++ itr_d){
            if(itr_e->first.b == itr_d->first.b){
                aaaaaaaaaaaa++;
                BN_mod_mul(tmp_bi.n, itr_e->second.n, itr_d->second.n, order.n, G->bn_ctx);
                BN_mod_mul(tmp_bi.n, tmp_bi.n, y[itr_e->first.b].n, order.n, G->bn_ctx);
                int j = itr_e->first.a - itr_d->first.a + l;
                // cout<<"aaaaaaaaaa="<<aaaaaaaaaaaa<<"   "<<j<<" "<<2*l+1<<endl;
                BN_mod_add(d_phi[j].n, d_phi[j].n, tmp_bi.n, order.n, G->bn_ctx);
            }
        }
    }
    

    six = clock();
    cout<<"finish d_phi"<<double(six-five)/CLOCKS_PER_SEC<<"s"<<endl;

    BigInt *rd_phi = new BigInt[2*l+1];
    for(int i=0;i<=2*l;i++){
        G->get_rand_bn(rd_phi[i]);
    }
    BN_zero(rd_phi[l+1].n);
    Point cd_phi[2*l+1];
    for(int i=0;i<=2*l;i++){
        cd_phi[i]=G->mul_gen(d_phi[i]);
        tmp_p = h.mul(rd_phi[i]);
        cd_phi[i]=cd_phi[i].add(tmp_p);
    }
    cd_phi[l+1] = G->mul_gen(rd_phi[l+1]);
    Point ce_send[2], cd_send[2];
    ce_send[0] = ce_a[0];
    ce_send[1] = ce_b[0];
    cd_send[0] = cd_a[N+1];
    cd_send[1] = cd_b[N+1];
    io->send_pt(ce_send, 2);
    io->send_pt(cd_send, 2);
    io->send_pt(cd_phi, 2*l+1);

    seven = clock();
    // cout<<"seven"<<double(seven-six)/CLOCKS_PER_SEC<<"s"<<endl;


    BigInt *challenge_x = new BigInt[2*l+1];
    BigInt *challenge_e = new BigInt[M];
    // BigInt *old_challenge_e = new BigInt[M];
    BigInt challenge_re, challenge_rd, challenge_t;
    BigInt *challenge_d = new BigInt[M];
    // BigInt *old_challenge_d = new BigInt[M];
    unsigned char *x_char = new unsigned char[32];
    io->recv_data(x_char, 32);
    challenge_x[1].from_bin(x_char, 32);

    BN_one(challenge_x[0].n);
    BN_one(tmp_power.n);
    for(int i=2;i<=2*l;i++){
        BN_mod_add(tmp_power.n, tmp_power.n, BN_value_one(), order.n, G->bn_ctx);
        BN_mod_exp(challenge_x[i].n, challenge_x[1].n, tmp_power.n, order.n, G->bn_ctx);
    }

    for(auto itr_e = e_matrix.begin(); itr_e != e_matrix.end(); ++ itr_e){ 
        int i = itr_e->first.a;
        int j = itr_e->first.b;
        BN_mod_mul(tmp_bi.n,itr_e->second.n, challenge_x[i].n, order.n, G->bn_ctx);
        BN_mod_add(challenge_e[j].n, challenge_e[j].n, tmp_bi.n, order.n, G->bn_ctx);
    }
    for(auto itr_d = d_i.begin(); itr_d != d_i.end(); ++ itr_d){
        int i = itr_d->first.a;
        int j = itr_d->first.b;
        BN_mod_mul(tmp_bi.n,itr_d->second.n, challenge_x[l-i].n, order.n, G->bn_ctx);
        BN_mod_add(challenge_d[j].n, challenge_d[j].n, tmp_bi.n, order.n, G->bn_ctx);
    }

    for(int i=0;i<=l;i++){
        // for(int j=0;j<M;j++){
        //     BN_mod_mul(tmp_bi.n, old_e_matrix[i][j].n, challenge_x[i].n, order.n, G->bn_ctx);
        //     BN_mod_add(old_challenge_e[j].n, old_challenge_e[j].n, tmp_bi.n, order.n, G->bn_ctx);
        //     BN_mod_mul(tmp_bi.n, old_d_i[i][j].n, challenge_x[l-i].n, order.n, G->bn_ctx);
        //     BN_mod_add(old_challenge_d[j].n, old_challenge_d[j].n, tmp_bi.n, order.n, G->bn_ctx);
        // }
        BN_mod_mul(tmp_bi.n, re_i[i].n, challenge_x[i].n, order.n, G->bn_ctx);
        BN_mod_add(challenge_re.n, challenge_re.n, tmp_bi.n, order.n, G->bn_ctx);
        BN_mod_mul(tmp_bi.n, rd_i[i].n, challenge_x[l-i].n, order.n, G->bn_ctx);
        BN_mod_add(challenge_rd.n, challenge_rd.n, tmp_bi.n, order.n, G->bn_ctx);
    }
    // for(int i=0;i<M;i++){
    //     if(BN_cmp(old_challenge_d[i].n, challenge_d[i].n)!=0){
    //         cout<<M<<"  wrong wrong challenge d              "<<i<<endl;
    //         if(BN_cmp(old_challenge_d[i].n, BN_value_one())!=0){
    //             cout<<M<<"  0             "<<i<<endl;
    //         }
    //     }
        
    // }




    eight = clock();
    // cout<<"eight"<<double(eight-seven)/CLOCKS_PER_SEC<<"s"<<endl;
    for(int i=0;i<=2*l;i++){
        BN_mod_mul(tmp_bi.n, challenge_x[i].n, rd_phi[i].n, order.n, G->bn_ctx);
        BN_mod_add(challenge_t.n, challenge_t.n, tmp_bi.n, order.n, G->bn_ctx);
    }
    for(int i=0;i<M;i++){
        unsigned char *bn_char = new unsigned char[32];
        challenge_e[i].to_bin(bn_char);
        BigInt test_e;
        test_e.from_bin(bn_char, 32);
        if(BN_cmp(test_e.n, challenge_e[i].n)!=0){
            cout<<"wrong e"<<endl;
        }
        io->send_data(bn_char,32);
    }
    for(int i=0;i<M;i++){
        unsigned char *bn_char = new unsigned char[32];
        challenge_d[i].to_bin(bn_char);
        BigInt test_d;
        test_d.from_bin(bn_char, 32);
        if(BN_cmp(test_d.n, challenge_d[i].n)!=0){
            cout<<"wrong d"<<endl;
        }
        io->send_data(bn_char,32);
    }

    nine = clock();
    // cout<<"nine"<<double(nine-eight)/CLOCKS_PER_SEC<<"s"<<endl;

    unsigned char* re_char = new unsigned char[32];
    challenge_re.to_bin(re_char);
    BigInt test_re;
    test_re.from_bin(re_char, 32);
    if(BN_cmp(test_re.n, challenge_re.n)!=0){
        cout<<"re wrong"<<endl;
    }
    io->send_data(re_char,32);
    unsigned char* rd_char = new unsigned char[32];
    challenge_rd.to_bin(rd_char);
    BigInt test_rd;
    test_rd.from_bin(rd_char, 32);
    if(BN_cmp(test_rd.n, challenge_rd.n)!=0){
        cout<<"rd wrong"<<endl;
    }
    io->send_data(rd_char,32);
    unsigned char* t_char = new unsigned char[32];
    challenge_t.to_bin(t_char);
    io->send_data(t_char,32);
    BigInt test_t;
    test_t.from_bin(t_char, 32);
    if(BN_cmp(test_t.n, challenge_t.n)!=0){
        cout<<"t wrong"<<endl;
    }
    end = clock();
    cout<<"zk zero time = "<<double(end-mid)/CLOCKS_PER_SEC<<"s"<<endl; 
    return true;
}

bool zk_EncEP_v(Group * G, Point h, Point * generators, Point * ca, Point * cb, int M, int N, NetIO *io){
    clock_t start,mid,end;
    start = clock();
    BigInt order;
    EC_GROUP_get_order(G->ec_group, order.n, G->bn_ctx);
    BigInt tmp_power;
    BigInt *w = new BigInt[N];
    BigInt W;
    G->get_rand_bn(w[0]);
    unsigned char* w_char = new unsigned char[32];
    w[0].to_bin(w_char);
    BigInt test_w;
    test_w.from_bin(w_char, 32);
    if(BN_cmp(test_w.n, w[0].n)!=0){
        cout<<"w wrong"<<endl;
    }
    io->send_data(w_char, 32);

    BN_one(tmp_power.n);
    BN_add(W.n, w[0].n,W.n);
    for(int i=1;i<N;i++){
        BN_add(tmp_power.n, tmp_power.n, BN_value_one());
        BN_mod_exp(w[i].n, w[0].n, tmp_power.n, order.n, G->bn_ctx);
        BN_mod_add(W.n, W.n, w[i].n,order.n, G->bn_ctx);
    }
    //w
    Point Ca, Cb, tmp_p;
    Ca = ca[0].mul(w[0]);
    Cb = cb[0].mul(w[0]);
    for(int i=1;i<N;i++){
        tmp_p = ca[i].mul(w[i]);
        Ca = Ca.add(tmp_p);
        tmp_p = cb[i].mul(w[i]);
        Cb = Cb.add(tmp_p);
    }
    //C

    Point ce = Cb;
    Point ce_p = G->mul_gen(W);
    BigInt rand_u,tmp_bi;
    G->get_rand_bn(rand_u);
    Point u[1];
    u[0] = G->mul_gen(rand_u);
    io->send_pt(u);
    int l = M;
    Point g_p_result[l+l%2];
    BigInt *y_p_result = new BigInt[l+l%2];
    BigInt *e_p_result = new BigInt[l+l%2];
    for(int i=0;i<l;i++){
        g_p_result[i] = generators[i];
        BN_one(y_p_result[i].n);
    }
    while(l!=1){
        int pad = l%2;
        int mid = (l + pad)/2;
        // cout<<"l mid "<<l<<" "<<mid<<endl;
        Point g_p[pad+l];
        BigInt *y_p = new BigInt[pad+l];
        for(int i=0;i<l;i++){
            g_p[i] = g_p_result[i];
            y_p[i] = y_p_result[i];
        }
        if(pad == 1){
            G->get_rand_bn(tmp_bi);
            g_p[l]=G->mul_gen(tmp_bi);
            io->recv_pt(G, g_p+l, 1);
            BN_zero(y_p[l].n);
        }
        Point v_receive[4];
        io->recv_pt(G, v_receive, 4);
        Point v_l, v_r, v_l_p, v_r_p;
        v_l = v_receive[0];
        v_r = v_receive[1];
        v_l_p = v_receive[2];
        v_r_p = v_receive[3];

        BigInt alpha;
        G->get_rand_bn(alpha);
        unsigned char* alpha_char = new unsigned char[32];
        alpha.to_bin(alpha_char);
        BigInt test_alpha;
        test_alpha.from_bin(alpha_char, 32);
        if(BN_cmp(test_alpha.n, alpha.n)!=0){
            cout<<"wrong alpha"<<endl;
        }
        io->send_data(alpha_char,32);

        BigInt alpha_square, alpha_square_inverse, alpha_inverse;
        BN_mod_inverse(alpha_inverse.n, alpha.n, order.n, G->bn_ctx);
        BN_mod_mul(alpha_square.n, alpha.n, alpha.n, order.n, G->bn_ctx);
        BN_mod_inverse(alpha_square_inverse.n, alpha_square.n, order.n, G->bn_ctx);


        tmp_p = v_l.mul(alpha_square);
        ce = ce.add(tmp_p);
        tmp_p = v_r.mul(alpha_square_inverse);
        ce = ce.add(tmp_p);
        tmp_p = v_l_p.mul(alpha_square);
        ce_p = ce_p.add(tmp_p);
        tmp_p = v_r_p.mul(alpha_square_inverse);
        ce_p = ce_p.add(tmp_p);
        //ce, cep

        for(int i=0;i<mid;i++){
            tmp_p = g_p[mid+i].mul(alpha);
            g_p_result[i] = g_p[i].mul(alpha_inverse);
            g_p_result[i] = g_p_result[i].add(tmp_p);
            BN_mod_mul(y_p_result[i].n, alpha_inverse.n, y_p[i].n, order.n, G->bn_ctx);
            BN_mod_mul(tmp_bi.n, alpha.n, y_p[mid+i].n, order.n, G->bn_ctx);
            BN_mod_add(y_p_result[i].n, y_p_result[i].n, tmp_bi.n, order.n, G->bn_ctx);
        }
        Point g_receive[1];
        io->recv_pt(G, g_receive,1);
        // if(g_receive[0] == g_p_result[0]){
        //     cout<<" g receive true"<<endl;
        // }else{
        //     cout<<" g receive false"<<endl;
        // }
        l = mid;
    }
    // cout<<"finish v   "<<endl;
    BigInt y_bar;
    Point g_bar;
    g_bar = g_p_result[0];
    y_bar = y_p_result[0];
    Point gamma = G->mul_gen(y_bar);
    Point g_receive[1];
    io->recv_pt(G, g_receive, 1);
    // if(g_receive[0] == g_bar){
    //     cout<<"g true"<<endl;
    // }else{
    //     cout<<"g false"<<endl;
    // }
    Point a[3];
    io->recv_pt(G, a, 3);
    // cout<<"v one"<<endl;
    BigInt alpha;
    G->get_rand_bn(alpha);
    unsigned char* alpha_char = new unsigned char[32];
    alpha.to_bin(alpha_char);
    BigInt test_alpha;
    test_alpha.from_bin(alpha_char, 32);
    if(BN_cmp(test_alpha.n , alpha.n)!=0){
        cout<<"wrong alpha"<<endl;
    }
    io->send_data(alpha_char,32);
    BigInt z_1, z_2, z_3, z_4;
    unsigned char * z_1_char = new unsigned char[32];
    io->recv_data(z_1_char, 32);
    z_1.from_bin(z_1_char,32);
    unsigned char * z_2_char = new unsigned char[32];
    io->recv_data(z_2_char, 32);
    z_2.from_bin(z_2_char,32);
    unsigned char * z_3_char = new unsigned char[32];
    io->recv_data(z_3_char, 32);
    z_3.from_bin(z_3_char,32);
    unsigned char * z_4_char = new unsigned char[32];
    io->recv_data(z_4_char, 32);
    z_4.from_bin(z_4_char,32);
    // cout<<"v two"<<endl;

    Point verify_a, verify_b;
    verify_a = g_bar.mul(z_1);
    tmp_p = u[0].mul(z_2);
    verify_a = verify_a.add(tmp_p);
    tmp_p = h.mul(z_3);
    verify_a = verify_a.add(tmp_p);
    tmp_p = ce.mul(alpha);
    verify_b = a[0].add(tmp_p);
    if(!(verify_a == verify_b)){
        cout<<"333333333333333333333333333333333333333false"<<endl;
        return false;
    }else{
        // cout<<"333333333333333333333333333333333333333true"<<endl;
    }
    verify_a = gamma.mul(z_1);
    tmp_p = u[0].mul(z_4);
    verify_a = verify_a.add(tmp_p);
    tmp_p = ce_p.mul(alpha);
    verify_b = a[1].add(tmp_p);
    if(!(verify_a == verify_b)){
        cout<<"4444444444444444444444444444444444444444false"<<endl;
        return false;
    }else{
        // cout<<"4444444444444444444444444444444444444444true"<<endl;
    }
    verify_a = G->mul_gen(z_3);
    tmp_p = Ca.mul(alpha);
    verify_b = a[2].add(tmp_p);
    if(!(verify_a == verify_b)){
        cout<<"5555555555555555555555555555555555555555false"<<endl;
        return false;
    }else{
        // cout<<"5555555555555555555555555555555555555555true"<<endl;
    }

    mid = clock();
    cout<<"zk sum time = "<<double(mid-start)/CLOCKS_PER_SEC<<"s"<<endl; 

    l = N+1;
    BigInt *x = new BigInt[N], *y = new BigInt[M];
    G->get_rand_bn(x[0]);
    G->get_rand_bn(y[0]);
    unsigned char* xy_char = new unsigned char[32];
    x[0].to_bin(xy_char);
    // BigInt test_x;
    // test_x.from_bin(xy_char, 32);
    // if(BN_cmp(test_x.n, x[0].n)!=0){
    //     cout<<"wrong x"<<endl;
    // }
    io->send_data(xy_char, 32);
    y[0].to_bin(xy_char);
    // BigInt test_y;
    // test_y.from_bin(xy_char, 32);
    // if(BN_cmp(test_y.n, y[0].n)!=0){
    //     cout<<"wrong y"<<endl;
    // }
    io->send_data(xy_char, 32);

    BN_one(tmp_power.n);
    for(int i=1;i<N;i++){
        BN_add(tmp_power.n, tmp_power.n, BN_value_one());
        BN_mod_exp(x[i].n, x[0].n, tmp_power.n, order.n, G->bn_ctx);
    }
    BN_one(tmp_power.n);
    for(int i=1;i<M;i++){
        BN_add(tmp_power.n, tmp_power.n, BN_value_one());
        BN_mod_exp(y[i].n, y[0].n, tmp_power.n, order.n, G->bn_ctx);
    }
    // randomly choose x,y
    Point cd_a[N+2], cd_b[N+2];
    cd_a[0] = ca[0].mul(x[0]);
    cd_b[0] = cb[0].mul(x[0]);
    cd_a[N] = cd_a[0];
    cd_b[N] = cd_b[0];
    for (int i = 1; i < N; i++)
    {
        cd_a[i] = ca[i].mul(x[i]);
        cd_b[i] = cb[i].mul(x[i]);
        cd_a[N] = cd_a[N].add(cd_a[i]);
        cd_b[N] = cd_b[N].add(cd_b[i]);
    }
    BigInt minus_one, zero;
    BN_zero(zero.n);
    BN_mod_sub(minus_one.n, zero.n, BN_value_one(), order.n, G->bn_ctx);

    Point ce_a[N+2], ce_b[N+2];
    for(int i = 0;i<N;i++){
        ce_a[i+1] = ca[i];
        ce_b[i+1] = cb[i];
    }

    ce_a[N+1] = G->mul_gen(zero);
    ce_b[N+1] = generators[0].mul(minus_one);
    for(int i=1;i<M;i++){
        tmp_p = generators[i].mul(minus_one);
        ce_b[N+1] = ce_b[N+1].add(tmp_p);
    }

    Point ce_receive[2], cd_receive[2], cd_phi[2*N+3];
    io->recv_pt(G, ce_receive, 2);
    io->recv_pt(G, cd_receive, 2);
    io->recv_pt(G, cd_phi, 2*N+3);
    ce_a[0] = ce_receive[0];
    ce_b[0] = ce_receive[1];
    cd_a[l] = cd_receive[0];
    cd_b[l] = cd_receive[1];




    BigInt *challenge_x = new BigInt[2*l+1], *challenge_e = new BigInt[M], challenge_re, *challenge_d = new BigInt[M], challenge_rd, challenge_t;
    unsigned char* x_char = new unsigned char[32];
    G->get_rand_bn(challenge_x[1]);
    challenge_x[1].to_bin(x_char);
    // BigInt test_cx;
    // test_cx.from_bin(x_char, 32);
    // if(BN_cmp(test_cx.n, challenge_x[1].n)!=0){
    //     cout<<"challenge x wrong"<<endl;
    // }
    io->send_data(x_char, 32);
    BN_one(challenge_x[0].n);
    BN_one(tmp_power.n);
    for(int i=2;i<=2*l;i++){
        BN_mod_add(tmp_power.n, tmp_power.n, BN_value_one(), order.n, G->bn_ctx);
        BN_mod_exp(challenge_x[i].n, challenge_x[1].n, tmp_power.n, order.n, G->bn_ctx);
    }
    for(int i=0;i<M;i++){
        unsigned char* bn_recv=new unsigned char[32];
        io->recv_data(bn_recv, 32);
        challenge_e[i].from_bin(bn_recv, 32);
    }
    for(int i=0;i<M;i++){
        unsigned char* bn_recv = new unsigned char[32];
        io->recv_data(bn_recv, 32);
        challenge_d[i].from_bin(bn_recv, 32);
    }



    unsigned char* re_recv = new unsigned char[32];
    io->recv_data(re_recv, 32);
    challenge_re.from_bin(re_recv, 32);
    unsigned char* rd_recv = new unsigned char[32];
    io->recv_data(rd_recv, 32);
    challenge_rd.from_bin(rd_recv, 32);
    unsigned char* t_recv = new unsigned char[32];
    io->recv_data(t_recv, 32);
    challenge_t.from_bin(t_recv, 32);


    Point challenge_a, challenge_b;

    challenge_a = ce_a[0].mul(challenge_x[0]);
    for(int i=1;i<l;i++){
        tmp_p = ce_a[i].mul(challenge_x[i]);
        challenge_a = challenge_a.add(tmp_p);
    }                                            
    challenge_b = G->mul_gen(challenge_re);
    if(!(challenge_a == challenge_b)){
        cout<<"66666666666666666666666false"<<endl;
        // return false;
    }else{
        // cout<<"66666666666666666666666true"<<endl;
    }
    challenge_a = ce_b[0].mul(challenge_x[0]);
    for(int i=1;i<=l;i++){
        tmp_p = ce_b[i].mul(challenge_x[i]);
        challenge_a = challenge_a.add(tmp_p);
    }      
    challenge_b = generators[0].mul(challenge_e[0]);                                     
    for(int i=1;i<M;i++){
        tmp_p = generators[i].mul(challenge_e[i]);
        challenge_b = challenge_b.add(tmp_p);
    }
    // Point chb[1];
    // io->recv_pt(G, chb);  
    // if(chb[0] == challenge_b){
    //     cout<<" b true"<<endl;
    // }else{
    //     cout<<" b wrong"<<endl;
    // }
    tmp_p = h.mul(challenge_re);
    challenge_b = challenge_b.add(tmp_p);
    if(!(challenge_a == challenge_b)){
        cout<<"777777777777777777777777777777false"<<endl;
        // return false;
    }else{
        // cout<<"77777777777777777777777777777true"<<endl;
    }
    challenge_a = cd_a[0].mul(challenge_x[l]);
    for(int i=1;i<=l;i++){
        tmp_p = cd_a[i].mul(challenge_x[l-i]);
        challenge_a = challenge_a.add(tmp_p);
    }
    challenge_b = G->mul_gen(challenge_rd);
    if(!(challenge_a == challenge_b)){
        cout<<"8888888888888888888888888888888888888888false"<<endl;
        // return false;
    }else{
        // cout<<"888888888888888888888888888888888888888true"<<endl;
    }
    challenge_a = cd_b[0].mul(challenge_x[l]);
    for(int i=1;i<=l;i++){
        tmp_p = cd_b[i].mul(challenge_x[l-i]);
        challenge_a = challenge_a.add(tmp_p);
    }
    challenge_b = generators[0].mul(challenge_d[0]);                                     
    for(int i=1;i<M;i++){
        tmp_p = generators[i].mul(challenge_d[i]);
        challenge_b = challenge_b.add(tmp_p);
    }
    tmp_p = h.mul(challenge_rd);
    challenge_b = challenge_b.add(tmp_p);      
    
    if(!(challenge_a == challenge_b)){
        cout<<"9999999999999999999999999999999999999false"<<endl;
        // return false;
    }else{
        // cout<<"999999999999999999999999999999999true"<<endl;
    }

    challenge_a = cd_phi[0].mul(challenge_x[0]);
    for(int i=1;i<=2*l;i++){
        tmp_p = cd_phi[i].mul(challenge_x[i]);
        challenge_a = challenge_a.add(tmp_p);
    }

    BigInt challenge_map;
    for(int i=0;i<M;i++){
        BN_mod_mul(tmp_bi.n, challenge_e[i].n, challenge_d[i].n, order.n, G->bn_ctx);
        BN_mod_mul(tmp_bi.n, tmp_bi.n, y[i].n, order.n, G->bn_ctx);
        BN_mod_add(challenge_map.n, challenge_map.n, tmp_bi.n, order.n, G->bn_ctx);
    }
    challenge_b = G->mul_gen(challenge_map);
    tmp_p = h.mul(challenge_t);
    challenge_b = challenge_b.add(tmp_p);
    // challenge_b = h.mul(challenge_t);
    if(!(challenge_a == challenge_b)){
        cout<<"101001010101001010100100101010010010false"<<endl;
        // return false;
    }else{
        // cout<<"10101010101010101010true"<<endl;
    }
    end = clock();
    cout<<"zk zero time = "<<double(end-mid)/CLOCKS_PER_SEC<<"s"<<endl; 
    return true;
}

// bool zk_sum_p(Group * G, Point h, Point * generators, Point * ca, Point * cb, vector<int> ep, BigInt * r, int M, int N, NetIO *io){

// }

// bool zk_sum_v(Group * G, Point h, Point * generators, Point * ca, Point * cb, int M, int N, NetIO *io){
    
// }

// bool zk_zero_p(){

// }

// bool zk_zero_v(){

// }
#endif
