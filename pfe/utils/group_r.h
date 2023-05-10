#ifndef Ppoint_GROUP_R_H__
#define Ppoint_GROUP_R_H__
#define ECCLVL 251
#define ceil_divide(x, y)			(( ((x) + (y)-1)/(y)))

extern "C"
{
	#include "relic.h"
}
#include <memory>
#include <vector>
namespace emp {
class Num_relic;
class Point_relic;
class Brickexp;
class Group_relic{
public:
	Group_relic(uint8_t* seed){
		init(seed);
	};
	Group_relic(){
		core_set(NULL);
		core_init();
		eb_param_set_any_plain();
		this->context = core_get();
		this->generator = get_generator();
		this->order = get_order();
	}

	~Group_relic();

	Num_relic* get_num();
	Num_relic* get_rnd_num(uint32_t bitlen);
	Point_relic* get_point();
	Point_relic* get_rnd_point();
	Point_relic* get_generator();
	Num_relic* get_order();
	uint32_t num_byte_size();
	uint32_t get_field_size();
	

	Brickexp* get_brick(Point_relic* gen);

	ctx_t* get_context();

protected:
	void init(uint8_t* seed);
private:
	Point_relic* sample_random_point();
	Point_relic* generator;
	ctx_t* context;
	Num_relic* order;
};

class Num_relic{

public:
	Num_relic(){};
	Num_relic(Group_relic* fld);
	Num_relic(Group_relic* fld, bn_t src);
	~Num_relic();
	void set(Num_relic* src);
	void set(Group_relic* fld);
	Num_relic operator+( Num_relic a);
	Num_relic operator-( Num_relic a);
	Num_relic operator*( Num_relic a);
	Num_relic operator/( Num_relic a);
	Num_relic operator^( Num_relic a);
	Num_relic operator%( Num_relic a);
	Num_relic operator=( Num_relic a);
    bool operator==( Num_relic a);
    bool operator!=( Num_relic a);


	void get_val (bn_t res);

	void to_bin(uint8_t* buf, uint32_t field_size_bytes);
	void from_bin(uint8_t* buf, uint32_t field_size_bytes);
	void print();

private:
	void shallow_copy(bn_t to, bn_t from);

	bn_t val;
	Group_relic* field;
	ctx_t* context;
};

class Point_relic{
public:
	Point_relic(){};
	Point_relic(Group_relic* fld);
	Point_relic(Group_relic* fld, eb_t src);
	~Point_relic();
	void set(Point_relic* src);
	void set(Group_relic* fld);
	void get_val (eb_t res);
    Point_relic operator=( Point_relic a);
    Point_relic operator+( Point_relic a);
    Point_relic operator-( Point_relic a);
    Point_relic operator^( Num_relic e);
    bool operator==( Point_relic a);
    bool operator!=( Point_relic a);
	Group_relic* get_field(){
		return field;
	};

	void to_bin(uint8_t* buf);
	void from_bin(uint8_t* buf);

	void print();

private:
	void init();
	void shallow_copy(eb_t to, eb_t from);
	eb_t val;
	Group_relic* field;
	ctx_t* context;
};

class Brickexp {
public:
	Brickexp(Point_relic* generator, Group_relic* field);
	~Brickexp();

	void pow(Point_relic* res, Num_relic* e);
private:
	uint32_t eb_pre_size;
	eb_t* eb_table;
	Group_relic* field;
	ctx_t* context;
};
void Group_relic::init(uint8_t* seed){
    core_set(NULL);
    core_init();
    rand_seed(seed, 14);
    eb_param_set_any_plain();

	this->context = core_get();
    this->generator = get_generator();
	this->order = get_order();
}

Group_relic::~Group_relic(){
    delete generator;
	delete order;
    core_set(this->context);
	core_clean();
}
Num_relic* Group_relic::get_num(){
    return new Num_relic(this);
}
Num_relic* Group_relic::get_rnd_num(uint32_t bitlen = 0){
    if (bitlen == 0) {
		bitlen = 251;
	}

	core_set(this->context);

	bn_t rnd;
	bn_null(rnd);
	bn_new(rnd);

	bn_rand(rnd, RLC_POS, bitlen);

	Num_relic* res = new Num_relic(this, rnd);
	return res;
}
Point_relic* Group_relic::get_point(){
    return new Point_relic(this);
}
Point_relic* Group_relic::get_rnd_point(){
    return sample_random_point();
}
Point_relic* Group_relic::get_generator(){
    core_set(this->context);

	eb_t eb_generator;
	eb_null(eb_generator);
	eb_new(eb_generator);
	eb_curve_get_gen(eb_generator);

	return new Point_relic(this, eb_generator);
}
Num_relic* Group_relic::get_order(){
    core_set(this->context);

	bn_t bn_order;
	bn_null(bn_order);
	bn_new(bn_order);
	eb_curve_get_ord(bn_order);

	return new Num_relic(this, bn_order);
}

uint32_t Group_relic::num_byte_size(){
    return ceil_divide(ECCLVL, 8);
}
uint32_t Group_relic::get_field_size(){
    return ECCLVL;
}

Brickexp* Group_relic::get_brick(Point_relic* gen){
    return new Brickexp(gen, this);
}
ctx_t* Group_relic::get_context(){
    return this->context;
}
Point_relic* Group_relic::sample_random_point(){
    core_set(this->context);

	eb_t tmp_eb;
	eb_null(tmp_eb);
	eb_new(tmp_eb);
	eb_rand(tmp_eb);


	Point_relic* tmp_point = new Point_relic(this, tmp_eb);
	return tmp_point;
}

Num_relic::Num_relic(Group_relic* fld){
    this->field = fld;
	this->context = field->get_context();

	core_set(this->context);

	bn_null(val);
	bn_new(val);
}
Num_relic::Num_relic(Group_relic* fld, bn_t src){
    this->field = fld;
	this->context = field->get_context();

	core_set(this->context);

	bn_null(val);
	bn_new(val);
	bn_copy(val, src);
}
Num_relic::~Num_relic(){
    core_set(this->context);

	bn_free(val);
}
void Num_relic::set(Num_relic* src){
	core_set(this->context);

	bn_t tmp_val;
	src->get_val(tmp_val);
	bn_copy(val, tmp_val);
}
void Num_relic::set(Group_relic* fld){
    this->field = fld;
	this->context = field->get_context();

	core_set(this->context);

	bn_null(val);
	bn_new(val);
}
Num_relic Num_relic::operator+( Num_relic a){
    Num_relic c(this->field);
    bn_t bn_a;
	bn_t bn_b;
	this->get_val(bn_a);
	a.get_val(bn_b);
	bn_add(c.val, bn_a, bn_b);
    return c;
}
Num_relic Num_relic::operator-( Num_relic a){
    Num_relic c(a.field);
    bn_t bn_a;
	bn_t bn_b;
	this->get_val(bn_a);
	a.get_val(bn_b);
	bn_sub(c.val, bn_a, bn_b);
    return c;
}
Num_relic Num_relic::operator*( Num_relic a){
    Num_relic c(a.field);
    bn_t bn_a;
	bn_t bn_b;
	this->get_val(bn_a);
	a.get_val(bn_b);
	bn_mul(c.val, bn_a, bn_b);
    return c;
}
Num_relic Num_relic::operator%( Num_relic a){
    Num_relic c(a.field);
    bn_t bn_a;
	bn_t bn_b;
	this->get_val(bn_a);
	a.get_val(bn_b);
	bn_mod(c.val, bn_a, bn_b);
    return c;
}
Num_relic Num_relic::operator=( Num_relic a){
    Num_relic c(a.field);
    bn_copy(c.val, a.val);
    return c;
}
bool Num_relic::operator==( Num_relic a){
    bn_t bn_a;
	bn_t bn_b;
	this->get_val(bn_a);
	a.get_val(bn_b);
	return bn_cmp(bn_a, bn_b) == RLC_EQ;
}
bool Num_relic::operator!=( Num_relic a){
    bn_t bn_a;
	bn_t bn_b;
	this->get_val(bn_a);
	a.get_val(bn_b);
	return bn_cmp(bn_a, bn_b) != RLC_EQ;
}


void Num_relic::get_val (bn_t res){
    shallow_copy(res, val);
}

void Num_relic::to_bin(uint8_t* buf, uint32_t field_size_bytes){
    core_set(this->context);
	bn_write_bin(buf, field_size_bytes, val);
}
void Num_relic::from_bin(uint8_t* buf, uint32_t field_size_bytes){
    core_set(this->context);

	bn_read_bin(val, buf, field_size_bytes);
}
void Num_relic::print(){
    core_set(this->context);

	bn_print(this->val);
}

void Num_relic::shallow_copy(bn_t to, bn_t from){
    *to = *from;
}
Point_relic::Point_relic(Group_relic* fld){
    this->field = fld;
	init();
}
Point_relic::Point_relic(Group_relic* fld, eb_t src){
    this->field = fld;
	init();

	core_set(this->context);

	eb_copy(this->val, src);
}
Point_relic::~Point_relic(){
    core_set(this->context);

	eb_free(val);
}
void Point_relic::set(Group_relic* fld){
    this->field = fld;
	init();
}
void Point_relic::set(Point_relic* src){
    core_set(this->context);

	eb_t tmp_val;
	src->get_val(tmp_val);
	eb_copy(this->val, tmp_val);
}
void Point_relic::get_val  (eb_t res){
    shallow_copy(res, this->val);
}
Point_relic Point_relic::operator=( Point_relic a){
	Point_relic c(a.field);
    core_set(a.context);
	eb_t tmp_val;
	a.get_val(tmp_val);
	eb_copy(c.val, tmp_val);
	return c;
}
Point_relic Point_relic::operator+( Point_relic a){
    Point_relic c(this->field);
    core_set(this->context);
	eb_t eb_a;
	eb_t eb_b;
	this->get_val(eb_a);
	a.get_val(eb_b);

	eb_add(c.val, eb_a, eb_b);
    return c;
}
Point_relic Point_relic::operator-( Point_relic a){
    Point_relic c(this->field);
    core_set(this->context);

	eb_t eb_a;
	eb_t eb_b;
	this->get_val(eb_a);
	a.get_val(eb_b);

	eb_sub(c.val, eb_a, eb_b);
    return c;
}
Point_relic Point_relic::operator^( Num_relic e){
    Point_relic c(this->field);
    core_set(this->context);

	eb_t eb_a;
	bn_t bn_e;
	this->get_val(eb_a);
	e.get_val(bn_e);

	eb_mul(c.val, eb_a, bn_e);
    return c;
}
bool Point_relic::operator==( Point_relic a){
    core_set(this->context);

	eb_t to_cmp,from_cmp;
	a.get_val(to_cmp);
    this->get_val(from_cmp);
	return eb_cmp(from_cmp, to_cmp) == RLC_EQ;
}
bool Point_relic::operator!=( Point_relic a){
    core_set(this->context);

	eb_t to_cmp,from_cmp;
	a.get_val(to_cmp);
    this->get_val(from_cmp);
	return eb_cmp(from_cmp, to_cmp) != RLC_EQ;
}

void Point_relic::to_bin(uint8_t* buf){
    core_set(context);
	eb_write_bin(buf, 33, this->val, true);
}
void Point_relic::from_bin(uint8_t* buf){
    core_set(context);
	eb_read_bin(val, buf, 33);
}

void Point_relic::init(){
    this->context = this->field->get_context();

	core_set(this->context);

	eb_null(val);
	eb_new(val);
}
void Point_relic::shallow_copy(eb_t to, eb_t from){
    *to = *from;
}
Brickexp::Brickexp(Point_relic* generator, Group_relic* field) {
	this->field = field;
	context = this->field->get_context();

	core_set(this->context);

	eb_t tmp_val;
	generator->get_val(tmp_val);
	this->eb_table = new eb_t[RLC_EB_TABLE_MAX];
	for(uint32_t i = 0; i < RLC_EB_TABLE_MAX; i++) {
		eb_null(this->eb_table[i]);
	}
	for(uint32_t i = 0; i < RLC_EB_TABLE; i++) {
		eb_new(this->eb_table[i]);
	}
	eb_mul_pre(this->eb_table, tmp_val);
}

Brickexp::~Brickexp() {
	core_set(this->context);

	for(uint32_t i = 0; i < RLC_EB_TABLE; ++i) {
		eb_free(this->eb_table[i]);
	}
	delete this->eb_table;
}


void Brickexp::pow(Point_relic* result, Num_relic* e) {
	core_set(this->context);

	eb_t eb_res;
	eb_null(eb_res);
	eb_new(eb_res);
	bn_t bn_e;
	e->get_val(bn_e);
	eb_mul_fix(eb_res, this->eb_table, bn_e);


	Point_relic tmp_point(this->field, eb_res);
	result->set(&tmp_point);

}
}


#endif