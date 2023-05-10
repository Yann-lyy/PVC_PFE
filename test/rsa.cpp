#include <fstream>
#include "openssl/rsa.h"
#include <openssl/pem.h>
using namespace std;
#define KEY_LENGTH  2048             // 密钥长度
#define PUB_KEY_FILE "pubkey.pem"    // 公钥路径
#define PRI_KEY_FILE "prikey.pem"    // 私钥路径
 
/*
制造密钥对：私钥和公钥
**/
int main(){
    string  out_pub_key;
    string  out_pri_key;

	size_t pri_len = 0; // 私钥长度
	size_t pub_len = 0; // 公钥长度
	char *pri_key = nullptr; // 私钥
	char *pub_key = nullptr; // 公钥
 
	// 生成密钥对
	RSA *keypair = RSA_generate_key(KEY_LENGTH, RSA_3, NULL, NULL);
 
	BIO *pri = BIO_new(BIO_s_mem());
	BIO *pub = BIO_new(BIO_s_mem());
 
    // 生成私钥
	PEM_write_bio_RSAPrivateKey(pri, keypair, NULL, NULL, 0, NULL, NULL);
	PEM_write_bio_RSAPublicKey(pub, keypair);
 
	// 获取长度  
	pri_len = BIO_pending(pri);
	pub_len = BIO_pending(pub);
 
	// 密钥对读取到字符串  
	pri_key = (char *)malloc(pri_len + 1);
	pub_key = (char *)malloc(pub_len + 1);
 
	BIO_read(pri, pri_key, pri_len);
	BIO_read(pub, pub_key, pub_len);
 
	pri_key[pri_len] = '\0';
	pub_key[pub_len] = '\0';
 
	out_pub_key = pub_key;
	out_pri_key = pri_key;
 
	// 将公钥写入文件
	std::ofstream pub_file(PUB_KEY_FILE, std::ios::out);
	if (!pub_file.is_open())
	{
		perror("pub key file open fail:");
		return -1;
	}
	pub_file << pub_key;
	pub_file.close();
 
	// 将私钥写入文件
	std::ofstream pri_file(PRI_KEY_FILE, std::ios::out);
	if (!pri_file.is_open())
	{
		perror("pri key file open fail:");
		return -1;
	}
	pri_file << pri_key;
	pri_file.close();
 
	// 释放内存
	RSA_free(keypair);
	BIO_free_all(pub);
	BIO_free_all(pri);
 
	free(pri_key);
	free(pub_key);
}