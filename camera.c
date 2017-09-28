#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>           
#include <fcntl.h>            
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
 
#include <asm/types.h>        
#include <linux/videodev2.h>

int file_fd;
int fd;

struct buffer  
{  
   void *start;  
   size_t length;  
};

struct buffer *buffers;

void read_frame()
{
    struct v4l2_buffer buf;
    
    //1、帧缓冲出队 -VIDIOC_QBUF
    buf.type =V4L2_BUF_TYPE_VIDEO_CAPTURE;  
    buf.memory =V4L2_MEMORY_MMAP;
     
    ioctl (fd, VIDIOC_QBUF, &buf);
    
    //2、访问帧缓冲
    write(file_fd,buffers[buf.index].start,buffers[buf.index].length);
    
    //3、帧缓冲重新入队 -VIDIOC_QBUF
    ioctl (fd, VIDIOC_QBUF, &buf);
    	
	
	
} 

int main()
{
    
    int file_fd, fd;
    struct v4l2_capability cap;
    struct v4l2_format fmt;
    struct v4l2_requestbuffers req; 
    unsigned int i;
    struct v4l2_buffer buf;
    enum v4l2_buf_type type;
    fd_set fds;
    
    
    file_fd = ("test.jpg", O_RDWR|O_CREAT, 077);
    //1、打开摄像头设备文件
    fd = ("/dev/video3", O_RDWR|O_NONBLOCK, 0);
    
    //2、获取驱动信息 -VIDIOC_QUERYCAP
    ioctl (fd, VIDIOC_QUERYCAP,&cap);
    printf ("driver name is %s",cap.driver);
    
    
    
    //3、设置图像的格式 -VIDIOC_S_FMT
    fmt.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = 640;
    fmt.fmt.pix.height = 480;
    fmt.fmt.pix.pixelformat=V4L2_PIX_FMT_MJPEG;
    
    ioctl (fd, VIDIOC_S_FMT, &fmt);
    
    //4、申请帧缓冲 -VIDIOC_REQBUFS
     
    req.count=4;  
    req.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;  
    req.memory=V4L2_MEMORY_MMAP;  
    ioctl(fd,VIDIOC_REQBUFS,&req);
    
    buffers = calloc(4, sizeof(buffers));
    
    for (i=0; i<req.count; ++i)
    {
        //5、获取帧缓冲地址长度信息 -VIDIOC_QUERYBUF
        buf.type =V4L2_BUF_TYPE_VIDEO_CAPTURE;  
        buf.memory =V4L2_MEMORY_MMAP;
        buf.index = i;
        ioctl (fd, VIDIOC_QUERYBUF, &buf);
        
        buffers[i].length = buf.length;
        
        //6、使用mmap把内核空间的帧缓冲映射到用户空间
        buffers[i].start = mmap (NULL, buf.length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        
    }
    	
    //7、帧缓冲入队列 -VIDIOC_QBUF
    for (i = 0; i< 4; ++i)  
    {  
    buf.type =V4L2_BUF_TYPE_VIDEO_CAPTURE;  
    buf.memory =V4L2_MEMORY_MMAP;  
    buf.index = i;  
    ioctl (fd,VIDIOC_QBUF, &buf);  
    }	
    
    //8、开始采集图像 -VIDIOC_STREAMON
    type =V4L2_BUF_TYPE_VIDEO_CAPTURE;  
    ioctl (fd,VIDIOC_STREAMON, &type);	
    
    FD_ZERO(&fds);
    FD_SET(fd,&fds);
    select (fd+1, &fds, NULL, NULL, NULL);
    
    read_frame();
    
    for (i = 0; i< 4; ++i)
        munmap (buffers[i].start, buffers[i].length);
        
    close(fd);
    close(file_fd);
    
	
}
