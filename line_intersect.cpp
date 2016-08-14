/*
 * DATE 2016.8.1
 * tested by SPOJ LINESEG - Intersection
 */
#include<iostream>
#include<vector>
#include<map>
#include<set>
#include<queue>
#include<cstdio>
#include<cstring>
#include<cmath>
#include<cfloat>
#include "geometry.h"
#define ESP 1e-6
using namespace std;

bool same_segment(const segment_t & s1, const segment_t& s2)
{
    return (s1.p[0] == s2.p[0]) && (s1.p[1] == s2.p[1]);
}
void segment_t::init(const point_t & p1,const point_t & p2)
{
    if(p1.x < p2.x || (p1.x == p2.x && p1.y < p2.y)){
        p[0] = p1; p[1] = p2;
    }
    else{
        p[0] = p2; p[1] = p1;
    }
    if(p[1].x - p[0].x == 0) gradient = DBL_MAX;
    else gradient = (p[1].y - p[0].y ) / (p[1].x - p[0].x);
}
bool operator==(const point_t & p1, const point_t& p2)
{
    return (p1.x == p2.x && p1.y == p2.y);
}
bool operator!=(const point_t & p1, const point_t& p2)
{
    return !(p1.x == p2.x && p1.y == p2.y);
}
intersection_result_t segment_t::intersect(const segment_t& s,point_t & ip)
{
    // special case preprocess
    if(same_segment(*this,s)){
            return LINE;
    }
    // intersect at end point
    for(int i = 0 ;i < 2 ; i++)
        for(int j = 0 ;j < 2 ; j++){
            if(p[i] == s.p[j]){
                    if(on_segment(s.p[j^1]) || s.on_segment(p[i^1]) ) return LINE;
                    ip = p[i];
                    return POINT;
            }
        }
    // parallel test
    if(is_parallel(s)){
            if(on_segment(s.p[0]) || on_segment(s.p[1]) || s.on_segment(p[0]) || s.on_segment(p[1])){
                return LINE;
            }
        return NONE;
    }
    //cout<<"fff"<<endl;
    if(find_intersection_by_crammer_rule(*this,s,ip)) return POINT;


    return NONE;
}
void segment_t::print(){
        cout<<"("<<p[0].x<<','<<p[0].y<<")  --> ("<<p[1].x<<","<<p[1].y<<")"<<endl;
}
bool segment_t::on_end_point(const point_t & p)const{
    return (this->p[0] == p || this->p[1] == p);
}
bool segment_t::is_parallel(const segment_t& s)const{
    double cross_product_value;
    cross_product_value = cross_product(s);
    if(cross_product_value < 0 )cross_product_value = -cross_product_value;
    return cross_product_value < ESP;
}
double segment_t::cross_product(const segment_t& s)const{
    double thisvx ,thisvy;
    double svx,svy;

    thisvx = this->p[1].x - this->p[0].x;
    thisvy = this->p[1].y - this->p[0].y;

    svx = s.p[1].x - s.p[0].x;
    svy = s.p[1].y - s.p[0].y;

    return thisvx*svy - thisvy*svx;

}
// return true if on segment
bool segment_t::on_segment(const point_t & pp)const
{
     double res=0;
    vector_t v1(pp,this->p[0]),v2(pp,this->p[1]);

    res = v1.cross_product(v2);
    if(res<0)res=-res;

    // on the line
    if(res<= ESP){
    return  (min(this->p[0].x,this->p[1].x) - ESP)<= pp.x && pp.x <= (max(this->p[0].x,this->p[1].x)+ESP) &&
            (min(this->p[0].y,this->p[1].y) - ESP)<= pp.y && pp.y <= (max(this->p[0].y,this->p[1].y)+ESP);
    }
    return false;
}

bool find_intersection_by_crammer_rule(const segment_t & s1,const segment_t & s2,point_t &p){

    double dx,dx1,dy,dy1;
    double D,Dx;//,Dy;
    dx = s1.p[0].x - s1.p[1].x;
    dy = s1.p[0].y - s1.p[1].y;
    dx1 = s2.p[0].x - s2.p[1].x;
    dy1 = s2.p[0].y - s2.p[1].y;


    D = dx*(-dy1) + dy*dx1;
    Dx = (s2.p[0].x - s1.p[0].x)*(-dy1) + dx1*(s2.p[0].y - s1.p[0].y);
   // Dy = (dx)*(s2.p[0].y - s1.p[0].y) - (dy)*(s2.p[0].x - s1.p[0].x);
   // cout<<D<<' '<<Dx<<' '<<Dy<<endl;
    if((-ESP)>D || D > ESP) // one solution
    {
        double t1 = Dx/D;
        // make sure the point is inside the segment b
        double x = s1.p[0].x + dx*t1;
        double y = s1.p[0].y + dy*t1;
        p.x = x;p.y = y;
       // cout<<p.x<<' '<<p.y<<endl;
        return  s1.on_segment(p) && s2.on_segment(p);
    }
    return false;

}
segment_t segment_t::create(const point_t & p1,const point_t & p2){
    segment_t s;
    s.init(p1,p2);
    return s;
}
double vector_t::cross_product(const vector_t& v)const{

    return vx*v.vy - vy*v.vx;
}
void segment_t::print()const{
	std::printf("(%.2lf %.2lf) -> (%.2lf %.2lf)\n",p[0].x,p[0].y,p[1].x,p[1].y);
}
