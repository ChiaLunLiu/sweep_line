#include<cstdio>
#include<queue>
#include<vector>
#include<set>
#include<memory>
#include<utility>
#include<cassert>
#include "geometry.h"

extern FILE* log_fp;
namespace{


class base_point_t;
class point_compare;
class segment_stasher;
//typedef std::priority_queue< shared_ptr<base_point_t> ,std::vector< shared_ptr<base_point_t> >,point_compare>  point_pq_t;


class base_point_t: public point_t, public std::enable_shared_from_this<base_point_t> {
  public:
	base_point_t(){}
	base_point_t(const base_point_t & bp):point_t(bp),angle(0){}
	//virtual void process(point_pq_t & pq,multiset<segment_stasher>& ss) = 0;
	short angle;
};


	
	class left_end_point_t: public base_point_t{
	  public:
		left_end_point_t():base_point_t(){}
		left_end_point_t(const left_end_point_t& p):base_point_t(p){}
		
		//virtual void process(point_pq_t & pq,multiset<segment_stasher>& ss);
	  private:
		shared_ptr<segment_t> seg; // the segment to which the point belongs
	};

	class intersection_point_t: public base_point_t{
	  public:
		intersection_point_t():base_point_t(){}
		intersection_point_t(const intersection_point_t& p):base_point_t(p){}
//		virtual void process(point_pq_t & pq,multiset<segment_stasher>& ss);
	  private:
		std::vector< shared_ptr<segment_t> > seg_vec; // the segments which intersect at this point
	};

	class right_end_point_t: public base_point_t{
	  public:
		right_end_point_t():base_point_t(){}
		right_end_point_t(const right_end_point_t& p):base_point_t(p){}
//		virtual void process(point_pq_t & pq,multiset<segment_stasher>& ss);
	  private:
		shared_ptr<segment_t> seg; // the segment to which the point belongs
	};
	
class point_compare{
  public:
       	bool operator()(const point_t & a,const point_t & b)const{
               	return a.y > b.y || (a.y == b.y  && a.x < b.x);
       	}
};
bool point_comparator(const point_t & a,const point_t & b){
	return a.x > b.x || (a.x == b.x  && a.y > b.y);
}

class segment_ptr_compare{
  public:
	typedef segment_t* segment_t_ptr;
       	bool operator()(const segment_t_ptr & a,const segment_t_ptr & b)const{
		if( point_comparator(a->p[0],b->p[0] )){
			return true;
		}
		else if(point_comparator( b->p[0],a->p[0])){
			return true;
		}
		else{
			// a->p[0] and b->p[0] are the same
			return point_comparator(a->p[1],b->p[1]);
		}
       	}
};

bool point_cos0_comparator(const std::pair<point_t,double>& a,const std::pair<point_t,double>& b){
	// pair::second keeps cos0 value
	return a.first.x > b.first.x || (a.first.x == b.first.x  && a.first.y > b.first.y ) ||
	      ( a.first.x == b.first.x && a.first.y == b.first.y && a.second > b.second );
}


class point_angle_compare{
  public:
       	bool operator()(const std::pair<point_t,int> & a,const std::pair<point_t,int> & b)const{
               	return a.first.x > b.first.x || (a.first.x == b.first.x  && a.first.y > b.first.y) || (
               	  a.first.x == b.first.x && a.first.y == b.first.y && a.second > b.second 
               	);
       	}
};
	/*
// segment_stasher is only used to associate multiset index with segment 
class point_event{
  public:
    bool operator<(const point_event& pe) const{
		return point->x < ss.p->x || 
			(point->x == ss.point->x && point->y < ss.point->y) ||
			(point->x == ss.point->x && point->y ==ss.point->y && point->angle < point->angle);
	}
	point_t p;
	std::vector<segmemt_t*> upper,lower,intermediate;
};
*/

/*
 *     . . . p1 
 *     .   0/
 *     .   /
 *     .  /
 *     . /          
 *     p2          
 *
 * p1 should be the event point
 */
double calc_cos(const point_t & p1,const point_t & p2){

	double X,Y;	
	assert( p1.x != p2.x || (p1.x == p2.x && p1.y != p2.y) );
	X = p2.x - p1.x;
	Y = p2.y - p1.y;
	return  X/sqrt(X*X + Y*Y);
}

void segment_insert(struct rb_root *root, segment_t *data){
	
  	struct rb_node **cur = &(root->rb_node), *parent = NULL;
  	/* Figure out where to put new node */
  	while (*cur) {
  		segment_t *seg = container_of(*cur, segment_t, rbnode);
  		int result = point_cos0_comparator(*((std::pair<point_t,double>*)data->priv), *((std::pair<point_t,double>*)seg->priv));

		parent = *cur;
  		if (result == 0)
  			cur = &((*cur)->rb_left);
  		else
  			cur = &((*cur)->rb_right);
  	}

  	/* Add new node and rebalance tree. */
  	rb_link_node(&data->rbnode, parent, cur);
  	rb_insert_color(&data->rbnode, root);

}


} /* end of namespace */



/*
 * extra same segments are filtered
 */
int find_all_intersection_points(std::vector<segment_t>& segs,std::vector<point_t>& intersection_points){

	/* do not change the order of enum, for loop assumes this order */
	enum{UPPER,INTERIOR,LOWER,ENUM_MAX};
	std::set<point_t,point_compare> point_event;
	std::set<point_t,point_compare>::iterator pe_iter;
	struct rb_root sl = RB_ROOT;
	struct rb_node* prev_rb,*next_rb;
	segment_t* prev_seg,*next_seg, * tmp_seg;
	map<point_t,std::set<segment_t*,segment_ptr_compare>, point_compare> point_map[ENUM_MAX];
	std::set<segment_t*,segment_ptr_compare>* segset[ENUM_MAX]; 
	std::set<segment_t*,segment_ptr_compare> dummy; 	
//start_iter[ENUM_MAX],end_iter[ENUM_MAX],tmp_iter;

	point_t ip;
	intersection_result_t res;


	// TODO, filter the same segment
	
	/* 
	 * order the point in the segment, save UPPER point to p[0], and LOWER point to p[1]
	 * insert all end points of segments to point_event and to upper and lower map */
	for(size_t i = 0 ;i < segs.size() ; i++){
		double tmp;
		point_t& upper = segs[i].p[0]; 
		point_t& lower = segs[i].p[1];
		rb_init_node(&segs[i].rbnode);
		
		if( upper.y < lower.y || ( upper.y == lower.y && upper.x > lower.x  )){
			swap(upper,lower);
		}

		point_event.insert(upper);
		point_event.insert(lower);
		if(upper.y == lower.y){
			assert(upper.x != lower.x);
			// it could be -1 or 1. 
			tmp = 1;	
		}
		else{
			tmp = calc_cos(upper,lower);
		}
		// first is x, second is angle 
		// first would be set right before the segment is inserted into to sl 
		segs[i].priv = new std::pair<double,double>;
		((std::pair<double,double>*)segs[i].priv)->second = tmp;
		point_map[UPPER][upper].insert(&segs[i]);
		point_map[LOWER][lower].insert(&segs[i]);
	}

	// debug print
//	for(pe_iter = point_event.begin() ; pe_iter != point_event.end(); pe_iter++){
//		printf("(%.2lf,%.2lf)\n",pe_iter->x,pe_iter->y);
//	}
	
	while( !point_event.empty()){
		puts("print segments in sweep line set");	
		for(struct rb_node* rb_ptr = rb_first( &sl ); rb_ptr != NULL ; rb_ptr = rb_next(rb_ptr) ){
			segment_t* tmp_seg = rb_entry(rb_ptr,segment_t,rbnode);	
			tmp_seg->print();
			printf("cos= %.2lf\n",((std::pair<double,double>*)(tmp_seg->priv))->second);
		}
	
	
		pe_iter = point_event.begin();
		if(log_fp)fprintf(log_fp,"next %.2lf %.2lf\n",pe_iter->x,pe_iter->y);
		printf("next event point is %.2lf %.2lf\n",pe_iter->x,pe_iter->y);
	
		for(unsigned i = 0 ;i < ENUM_MAX;i++){
			if(point_map[i].count(*pe_iter) > 0 )segset[i] = &point_map[i][*pe_iter];
			else segset[i]=  &dummy;
		}
	
		// overlapped segments having the common end point would be counted as intersection	
		if(segset[UPPER]->size()+ segset[INTERIOR]->size()+ segset[LOWER]->size() >= 2){
			// intersection
			printf("intersect at %.2lf %.2lf\n",pe_iter->x,pe_iter->y);
			if(log_fp)fprintf(log_fp,"intersect %.2lf %.2lf\n",pe_iter->x,pe_iter->y);
			printf("segments are:\n");
			for(unsigned i = 0 ;i < ENUM_MAX;i++){
				for( std::set<segment_t*,segment_ptr_compare>::iterator it = segset[i]->begin(); 
					it!= segset[i]->end() ; it++)
				printf("(%.2lf,%.2lf) (%.2lf,%.2lf)\n",(*it)->p[0].x,(*it)->p[0].y,
								       (*it)->p[1].x,(*it)->p[1].y);
			}
		}
			
		// remove segment in the range of start_iter[INTERIOR|LOWER] and end_inter[INTERIOR|LOWER] 
		// from sl 
		//
		for(unsigned i = INTERIOR ; i <= LOWER ;i++){
			for( std::set<segment_t*,segment_ptr_compare>::iterator it = segset[i]->begin(); 
					it!= segset[i]->end() ; it++){
				tmp_seg = *it;
				if(!RB_EMPTY_NODE(&tmp_seg->rbnode)){
					printf("%s remove segment\n", i == INTERIOR ? "interior":"lower");
					if(log_fp)fflush(log_fp);
					tmp_seg->print();
					next_rb = rb_next(&tmp_seg->rbnode);
					prev_rb = rb_prev(&tmp_seg->rbnode);
					if(prev_rb && next_rb ){
					
						next_seg = container_of(next_rb, segment_t, rbnode);
						prev_seg = container_of(prev_rb, segment_t, rbnode);
						res = prev_seg->intersect(*next_seg,ip);
						printf("test intersection of\n");
						next_seg->print();
						prev_seg->print();
						//TODO, repeated ip in point_event
						if(res == intersection_result_t::POINT){
							printf("intersect at (%.2lf,%.2lf)\n",ip.x,ip.y);
							if(ip.y < pe_iter->y || (ip.y == pe_iter->y && ip.x > pe_iter->x) ){
								point_event.insert(ip);
								// TODO, index precision problem
								point_map[INTERIOR][ip].insert(prev_seg);
								point_map[INTERIOR][ip].insert(next_seg);
							}
						}
					}
					rb_erase(&tmp_seg->rbnode, &sl);
				
					rb_init_node(&tmp_seg->rbnode);
				}
				else assert("not empty node");
			}
		}

		// insert segments in the range of start_iter[UPPER|INTERIOR] and end_inter[UPPER|INTERIOR] 
		// to sl
		//
		for(unsigned i = UPPER; i<= INTERIOR ;i++){
			// TODO, can't insert pair to point_map[INTERIOR] in the loop 
			for( std::set<segment_t*,segment_ptr_compare>::iterator it = segset[i]->begin(); 
					it!= segset[i]->end() ; it++){

				tmp_seg = *it;
				((std::pair<double,double>*)(tmp_seg->priv))->first = pe_iter->x;
				printf("insert segment, x = %.2lf\n",pe_iter->x);
				tmp_seg->print();
				
				if(!RB_EMPTY_NODE(&tmp_seg->rbnode)){
					fprintf(stderr,"not empty node\n");
				}
				segment_insert(&sl,tmp_seg);
				// find its prev and next neighbors and test intersection
				prev_rb = rb_prev(&tmp_seg->rbnode);

				if(prev_rb){

					prev_seg = container_of(prev_rb, segment_t, rbnode);
					printf("prev is \n");
					prev_seg->print();
					res = prev_seg->intersect(*tmp_seg,ip);
					if(res == intersection_result_t::POINT){
						printf("*** intersect at (%.2lf,%.2lf)\n",ip.x,ip.y);
						if(ip.y < pe_iter->y || (ip.y == pe_iter->y && ip.x > pe_iter->x) ){
							point_event.insert(ip);
							// TODO, index precision problem
							point_map[INTERIOR][ip].insert(prev_seg);
							point_map[INTERIOR][ip].insert(tmp_seg);
						}
					}
				}
				
				next_rb = rb_next(&tmp_seg->rbnode);
				if(next_rb){
					next_seg = container_of(next_rb, segment_t, rbnode);
					printf("next is \n");
					next_seg->print();
					res = next_seg->intersect(*tmp_seg,ip);
					if(res == intersection_result_t::POINT){
						printf("intersect at (%.2lf,%.2lf)\n",ip.x,ip.y);
						if(ip.y < pe_iter->y || (ip.y == pe_iter->y && ip.x > pe_iter->x) ){
							point_event.insert(ip);
							// TODO, index precision problem
							point_map[INTERIOR][ip].insert(next_seg);
							point_map[INTERIOR][ip].insert(tmp_seg);
						}
					}
				}
			}
		}
		point_event.erase(pe_iter);
	}
	// check
	for(struct rb_node* rb_ptr = rb_first( &sl ); rb_ptr != NULL ; rb_ptr = rb_next(rb_ptr) ){
		segment_t* tmp_seg = rb_entry(rb_ptr,segment_t,rbnode);	
		tmp_seg->print();
		printf("cos= %.2lf\n",((std::pair<double,double>*)(tmp_seg->priv))->second);
		assert( 0 && "rb is not empty in the end");
	}
return 0;
}
