#include<cstdio>
#include<queue>
#include<vector>
#include<ctime>
#include<cstdlib>
#include "geometry.h"
#include "segments_intersection.h"

int main()
{
	/* gen test case */
	int n;
	srand(time(NULL));
	std::cin>>n;

	std::vector<segment_t> segs;
	std::vector<point_t> intersection_points;
	segment_t s;
	for(int i = 0 ;i < n ; i++){
		s.p[0].x = rand()%200  -100;
		s.p[0].y = rand()%200  -100;
		s.p[1].x = rand()%200  -100;
		s.p[1].y = rand()%200  -100;
		s.init(s.p[0],s.p[1]);
		s.print();
		segs.push_back(s);
	}
	
	find_all_intersection_points(segs,intersection_points);
	
	puts("intersection points:");
	for(size_t i = 0 ;i < intersection_points.size(); i++){
			printf("%.2lf %.2lf\n",intersection_points[i].x,intersection_points[i].y);
	}
	
	return 0;
}
