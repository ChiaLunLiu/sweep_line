/*
 * DATE 2016.8.1
 * tested by SPOJ LINESEG - Intersection
 */
#include<iostream>
#include<vector>
#include<map>
#include<set>
#pragma once
#include<queue>
#include<cstdio>
#include<cstring>
#include<cmath>
#include "rbtree.h"
#define ESP 1e-6
using namespace std;
typedef enum _intersection_result_t{
    NONE,
    POINT,
    LINE
}intersection_result_t;



struct point_t{

    double x,y;
    struct rb_node rbnode;
    friend inline bool operator==(const point_t & p1, const point_t& p2);
    friend inline bool operator!=(const point_t & p1, const point_t& p2);
};

struct vector_t{
    double vx,vy;
    vector_t(){}
    vector_t(const point_t & start,const point_t & ending):vx(ending.x - start.x),vy(ending.y - start.y){}
    double cross_product(const vector_t& v)const;
};

struct segment_t{
	segment_t():priv(NULL){}
    void init(const point_t & p1,const point_t & p2);
    void print();
    // the input point is on one of end points of the segment
    // return true if it does
    // otherwise, return false.
    bool on_end_point(const point_t & p)const;

    bool is_parallel(const segment_t& s)const;
    // return true if on segment
    bool on_segment(const point_t & p)const;
    //
    //  input arg intersect_point is only meaningful if the return value is POINT
    //
    intersection_result_t intersect(const segment_t& s,point_t & intersect_point);
    static segment_t create(const point_t & p1,const point_t & p2);
    void print()const;
    /* p[0]: smaller x if x is the same smaller y */
    struct point_t p[2];
    struct rb_node rbnode;
    double gradient;
    void* priv; /* this can keep any data */
private:
    // return value: (this->p[1].y - this->p[0].y,this->p[1].x - this->p[0].x) cross
    // (s->p[1].y - s->p[0].y,s->p[1].x - s->p[0].x)
    double cross_product(const segment_t& s)const;
    
};
/*
 * 1: same segment
 * 0: not same segment
 */
bool same_segment(const segment_t & s1, const segment_t& s2);
bool find_intersection_by_crammer_rule(const segment_t & s1,const segment_t & s2,point_t &p);
int cross(int x1,int y1,int x2,int y2,int x3,int y3);
