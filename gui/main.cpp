#include <memory>
#include <cstdint>
#include <iostream>
#include <evhttp.h>
#include <event2/buffer.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include<cstring>
void OnReq(evhttp_request *req, void *);
void append_file(struct evbuffer* buf,const char* filename);
int main()
{
	int state;
  if (!event_init())
  {
    std::cerr << "Failed to init libevent." << std::endl;
    return -1;
  }
  char const SrvAddress[] = "127.0.0.1";
  std::uint16_t SrvPort = 5555;
  std::unique_ptr<evhttp, decltype(&evhttp_free)> Server(evhttp_start(SrvAddress, SrvPort), &evhttp_free);
  if (!Server)
  {
    std::cerr << "Failed to init http server." << std::endl;
    return -1;
  }
  evhttp_set_gencb(Server.get(), OnReq, (void*)&state);
  if (event_dispatch() == -1)
  {
    std::cerr << "Failed to run messahe loop." << std::endl;
    return -1;
  }
  return 0;
}

void append_file(struct evbuffer* buf,const char* filename){
    int fd;
    struct stat s;
    int status;
    /* Open the file for reading. */
    fd = open (filename, O_RDONLY);
    
    assert(fd >=0);
    /* Get the size of the file. */
    status = fstat (fd, & s);
    assert(status>=0);
    evbuffer_add_file(buf,fd,0,s.st_size);
}

void OnReq(evhttp_request *req, void *arg){
    int *state = (int*)arg;
    auto *OutBuf = evhttp_request_get_output_buffer(req);
	const struct evhttp_uri * uri = evhttp_request_get_evhttp_uri(req);
	const char * 	query_type = evhttp_uri_get_query (uri);
	struct evkeyvalq kv_pairs;
	const char* action;
    if (!OutBuf)
      return;
	
	struct evbuffer *  in_buf=	evhttp_request_get_input_buffer (req);
	char data[256];
	evbuffer_copyout(in_buf, data, 255);
	data[255]='\0';
	printf("query res = %d\n",evhttp_parse_query_str(data,&kv_pairs));
	action = evhttp_find_header(&kv_pairs, "action");
	printf("action = %s\n",action);
	if(action){
		if(!strcmp(action,"restart")){
			// gen testcase
			evbuffer_add_printf(OutBuf,"hihihihihi");		
		}
		else{
			assert("no such action");
		}
	}
	else{
     		append_file(OutBuf,"display.html");
	}
	printf("data\n");
	puts(data);
	evhttp_clear_headers(&kv_pairs);
	if(query_type) printf("type = %s\n",query_type);
	else puts("null query type");
	printf("host = %s\n",evhttp_request_get_host (req));
    evhttp_send_reply(req, HTTP_OK, "", OutBuf);
  };

