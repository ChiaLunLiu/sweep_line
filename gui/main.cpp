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
void append_file(struct evbuffer* buf,const char* filename);
int main()
{
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
  void (*OnReq)(evhttp_request *req, void *) = [] (evhttp_request *req, void *)
  {
    auto *OutBuf = evhttp_request_get_output_buffer(req);
    if (!OutBuf)
      return;
     append_file(OutBuf,"display.html");
    evhttp_send_reply(req, HTTP_OK, "", OutBuf);
  };
  evhttp_set_gencb(Server.get(), OnReq, nullptr);
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
