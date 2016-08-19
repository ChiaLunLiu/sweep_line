#include<cstdio>
#include<queue>
#include<vector>
#include<set>
#include<memory>
#include<utility>
#include<cassert>
#include "geometry.h"
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




int find_all_intersection_points(std::vector<segment_t>& segs,std::vector<point_t>& intersection_points){

	/* do not change the order of enum, for loop assumes this order */
	enum{UPPER,INTERIOR,LOWER,ENUM_MAX};
	std::set<point_t,point_compare> point_event;
	std::set<point_t,point_compare>::iterator pe_iter;
	struct rb_root sl = RB_ROOT;
	struct rb_node* prev_rb,*next_rb;
	segment_t* prev_seg,*next_seg;
	multimap<point_t,segment_t*,point_compare> point_map[ENUM_MAX];
	unsigned int query_size[ENUM_MAX];
	multimap<point_t,segment_t*,point_compare>::iterator start_iter[ENUM_MAX],end_iter[ENUM_MAX],tmp_iter;
	point_t ip;
	intersection_result_t res;
	
	/* 
	 * order the point in the segment, save UPPER point to p[0], and LOWER point to p[1]
	 * insert all end points of segments to point_event and to upper and lower map */
	for(size_t i = 0 ;i < segs.size() ; i++){
		double tmp;
		point_t& upper = segs[i].p[0]; 
		point_t& lower = segs[i].p[1];
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
		point_map[UPPER].insert(std::make_pair(upper,&segs[i]));
		point_map[LOWER].insert(std::make_pair(lower,&segs[i]));
	}

	// debug print
//	for(pe_iter = point_event.begin() ; pe_iter != point_event.end(); pe_iter++){
//		printf("(%.2lf,%.2lf)\n",pe_iter->x,pe_iter->y);
//	}
/*	puts("print segments");	
	for(struct rb_node* rb_ptr = rb_first( &sl ); rb_ptr != NULL ; rb_ptr = rb_next(rb_ptr) ){
		segment_t* tmp_seg = rb_entry(rb_ptr,segment_t,rbnode);	
		//			next_seg = container_of(next_rb, segment_t, rbnode);
		tmp_seg->print();
		printf("cos= %.2lf\n",((std::pair<point_t,double>*)(tmp_seg->priv))->second);
	}
*/	
	while( !point_event.empty()){
		
		pe_iter = point_event.begin();
		
		printf("next event point is %.2lf %.2lf\n",pe_iter->x,pe_iter->y);
	
		for(unsigned i = 0 ;i < ENUM_MAX;i++){
			start_iter[i] = point_map[i].lower_bound(*pe_iter);
			end_iter[i]   = point_map[i].upper_bound(*pe_iter);
			query_size[i] = point_map[i].count(*pe_iter);
		}
	
		// overlapped segments having the common end point would be counted as intersection	
		if(query_size[0]+query_size[1] + query_size[2] >= 2){
			/* intersection */
			printf("intersect at %.2lf %.2lf\n",pe_iter->x,pe_iter->y);
			printf("segments are:\n");
			for(unsigned i = 0 ;i < ENUM_MAX;i++){
				tmp_iter = start_iter[i];
				while(tmp_iter!=end_iter[i]){
				printf("(%.2lf,%.2lf) (%.2lf,%.2lf)\n",tmp_iter->second->p[0].x,tmp_iter->second->p[0].y,
													   tmp_iter->second->p[1].x,tmp_iter->second->p[1].y);
				tmp_iter++;
				}
			}
		}
		point_event.erase(pe_iter);
		continue;
			
		/* remove segment in the range of start_iter[INTERIOR|LOWER] and end_inter[INTERIOR|LOWER] 
		 * from sl 
		 */
		for(unsigned i = INTERIOR ; i <= LOWER ;i++){
			tmp_iter = start_iter[i];
			while(tmp_iter!=end_iter[i]){

				if(!RB_EMPTY_NODE(&tmp_iter->second->rbnode)){
					next_rb = rb_next(&tmp_iter->second->rbnode);
					prev_rb = rb_prev(&tmp_iter->second->rbnode);
					if(prev_rb && next_rb ){
					
						next_seg = container_of(next_rb, segment_t, rbnode);
						prev_seg = container_of(prev_rb, segment_t, rbnode);
						res = prev_seg->intersect(*next_seg,ip);
						//TODO, repeated ip in point_event
						if(res == intersection_result_t::POINT){
							point_event.insert(ip);
							// TODO, index precision problem
							point_map[INTERIOR].insert(std::make_pair(ip,prev_seg));
							point_map[INTERIOR].insert(std::make_pair(ip,next_seg));
						}
					}
					rb_erase(&tmp_iter->second->rbnode, &sl);
				}
				tmp_iter++;
			}
		}
		/* insert segments in the range of start_iter[UPPER|INTERIOR] and end_inter[UPPER|INTERIOR] 
		 * to sl
		 */
		for(unsigned i = UPPER; i<= INTERIOR ;i++){
			tmp_iter = start_iter[i];
			while(tmp_iter!=end_iter[i]){
				if(tmp_iter->second->priv)delete (std::pair<point_t,double>*)tmp_iter->second->priv;
				
				tmp_iter->second->priv = new std::pair<point_t,double>(*pe_iter,calc_cos(*pe_iter,tmp_iter->second->p[1]));
				segment_insert(&sl,tmp_iter->second);
				/* find its prev and next neighbors and test intersection */
				prev_rb = rb_prev(&tmp_iter->second->rbnode);

				if(prev_rb){
					prev_seg = container_of(prev_rb, segment_t, rbnode);
					res = prev_seg->intersect(*(tmp_iter->second),ip);
					if(res == intersection_result_t::POINT){
						point_event.insert(ip);
						// TODO, index precision problem
						point_map[INTERIOR].insert(std::make_pair(ip,prev_seg));
						point_map[INTERIOR].insert(std::make_pair(ip,tmp_iter->second));
					}
				}
				
				next_rb = rb_prev(&tmp_iter->second->rbnode);
				if(next_rb){
					next_seg = container_of(next_rb, segment_t, rbnode);
					res = next_seg->intersect(*(tmp_iter->second),ip);
					if(res == intersection_result_t::POINT){
						point_event.insert(ip);
						// TODO, index precision problem
						point_map[INTERIOR].insert(std::make_pair(ip,next_seg));
						point_map[INTERIOR].insert(std::make_pair(ip,tmp_iter->second));
					}
				}
				tmp_iter++;
			}
		}
	}
	
return 0;
}
