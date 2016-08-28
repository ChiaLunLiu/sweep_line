#include<cstdio>
#include<queue>
#include<vector>
#include<ctime>
#include<cstdlib>
#include "geometry.h"
#include "segments_intersection.h"
#define DEFAULT_NUM 8
#define VALUE_RANGE 20
#define VALUE_BASE 0
// argv[1]: file log
// argv[2]: size of testcase
FILE* log_fp;
void bye (void)
{
  fclose(log_fp);
  puts("Goodbye, cruel world....");
}
int main(int argc, char** argv)
{
	/* gen test case */
	int n;
	if(argc >= 2){
		if(!(log_fp = fopen(argv[1],"w"))){
			fprintf(stderr,"freopen fails\n");
			return 0;
		}
		atexit (bye);
	}
	if( argc >= 3){
		n = atoi( argv[2]);
	}
	else n = DEFAULT_NUM;
	if(log_fp){
		fprintf(log_fp,"%d\n",n);
	}
	srand(time(NULL));
	std::vector<segment_t> segs;
	std::vector<point_t> intersection_points;
	segment_t s;
	for(int i = 0 ;i < n ; i++){
		s.p[0].x = rand()%VALUE_RANGE  - VALUE_BASE ;
		s.p[0].y = rand()%VALUE_RANGE  - VALUE_BASE  ;
		s.p[1].x = rand()%VALUE_RANGE  - VALUE_BASE  ;
		s.p[1].y = rand()%VALUE_RANGE  - VALUE_BASE  ;
//		s.init();
		s.print();
		if(log_fp) fprintf(log_fp,"%.2lf %.2lf %.2lf %.2lf\n",s.p[0].x,s.p[0].y,s.p[1].x,s.p[1].y);	
		segs.push_back(s);
	}
	
	find_all_intersection_points(segs,intersection_points);
	
	puts("intersection points:");
	for(size_t i = 0 ;i < intersection_points.size(); i++){
			printf("%.2lf %.2lf\n",intersection_points[i].x,intersection_points[i].y);
	}

	if(log_fp) fclose(log_fp);	
	return 0;
}
