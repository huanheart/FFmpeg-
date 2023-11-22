#include <QCoreApplication>
#include<iostream>

#include<fstream>

using namespace std;

#define __STDC_CONSTANT_MACROS
extern "C" {
#include"libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libavdevice/avdevice.h"
#include"libswscale/swscale.h"
#include<libavutil/imgutils.h>
}


void saveFrame(uint8_t **data,int *linesizes,int width,int height,int count,string filename)
{
    ofstream ofs;
    string finally_fileName=filename+"/"+to_string(count)+".ppm"; //把他变成pcm数据

    ofs.open(finally_fileName,ios::out|ios::binary); //以二进制模式打开文件。
    //图像、音频或其他不需要按照文本规则进行解释的文件，就需要以二进制模式打开文件。,还有一些追加模式，输入模式的打开方式
    if(!ofs.is_open()){
        cout<<"打不开这个文件"<<' '<<finally_fileName<<endl;
            return ;
    }
    string header = "P6\n" + to_string(width) + " " + to_string(height) + "\n255\n";
    //这行代码是用来构建 PPM 文件的头部信息的字符串。PPM 文件是一种简单的图像文件格式，头部信息描述了图像的一些基本属性。
    ofs.write(header.c_str(),header.size());
    for(int i=0;i<height;i++)
    {
        ofs.write((const char*)data[0]+i*linesizes[0],width*3);
                //这个偏移量得设置正确，查看的文章有错误,width是图像的宽度
            //        (const char*)data[0] + i * linesizes[0] 表示从 data 数组中的第一个元素开始的偏移量，
            //        width * 3 表示每行的字节数，即每个像素占用 3 个字节。这是因为你处理的是 RGB 数据，每个像素包含红、绿、蓝三个通道。
    }
    ofs.close();
}



int main(int argc, char *argv[])
{
    AVFormatContext *pFormatCtx=NULL;
    AVCodecContext * pCodecCtx=NULL;

    AVCodecParameters *pVideoCodecPar=NULL;

    const AVCodec *pCodec=NULL;
    AVFrame *pFrame=NULL;
    AVPacket *packet=NULL;
    uint8_t *out_buffer=NULL;
    static struct SwsContext *img_convert_ctx=NULL;

    uint8_t *data[4]={nullptr};
    int linesizes[4]={0};

    int videoStream,i;

    //    avformat_network_init(); //网络模块的初始化
    avdevice_register_all();
    unsigned version=avcodec_version();
    cout<<"版本是"<<' '<<version<<endl;

        pFormatCtx=avformat_alloc_context();

    char * file_path="D:\\CloudMusic\\MV\\one.mp4"; //这个\要改成/
    if(avformat_open_input(&pFormatCtx,file_path,NULL,NULL)!=0)//打开输入流并读取标头。
    {//后面两个参数分别为AVInputFormat指定视频格式，AVDictionary指定码流的各种参数，传空的话FFmpeg会自动检测。
        cout<<"打开文件失败"<<endl;
        return -1;
    }   //pFormatCtx包含了文件的格式信息。

    if(avformat_find_stream_info(pFormatCtx,NULL)<0){
        cout<<"获取信息失败"<<endl;
    }//这个函数用于获取多媒体文件的流信息

    videoStream=-1;
    for(int i=0;i<pFormatCtx->nb_streams;i++) //nb_streams是AVFormatContext.streams中的元素数量。
    {
        if(pFormatCtx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO) //stream是文件中所有流的列表。
        {
            //codecpar是 与该流关联的编解码器参数
            //codec_type编码数据的一般类型。
            videoStream=i;
            break;
        }
    }
    //循环查找视频中包含的流信息，直到找到视频类型的流
    //便将其记录下来 保存到videoStream变量中
    if(videoStream==-1)
    {
        cout<<"没有这个视频流"<<endl;
        return -1;
    }
    //查找解码器
    //AVCodecParameters包含视频流使用的codec的相关参数（原先的AVStream.codec已被弃用）。有了我们就能找到具体的解码器并打开它。

    pVideoCodecPar=pFormatCtx->streams[videoStream]->codecpar;  //codecpar是 与该流关联的编解码器参数

    pCodec=avcodec_find_decoder(pVideoCodecPar->codec_id); //这种要改成AVCodecParameters这种结构，由于AVStream.codec
    if(pCodec==NULL)
    {
        cout<<"没有找到对应的解码器"<<endl;
        return -1;
    }
    //avcodec_open2前必须使用avcodec_alloc_context3生成context,
    pCodecCtx=avcodec_alloc_context3(pCodec);    //分配AVCodecContext并将其字段设置为默认值。
    if(avcodec_parameters_to_context(pCodecCtx,pVideoCodecPar)<0)   //根据提供的编解码器参数的值填充编解码器上下文。
    {
        cout<<"编解码器参数的值填充编解码器上下文失败"<<endl;
        return -1;
    }
    //打开解码器
    if(avcodec_open2(pCodecCtx,pCodec,NULL)<0)
    {
        cout<<"打开解码器失败"<<endl;
        return -1;
    }

    //思考，这个的话我读取一个视频，她不用进行一个解复用吗？？
    pFrame=av_frame_alloc();
    packet=av_packet_alloc();
    //pFrameRGB=av_frame_alloc();

    //SwsContext
    //申请存储RGB的buffer
    //原先avpicture_get_size=>av_malloc=>avpicture_fill这套接口被弃用了
    //新接口 <libavutil/imgutils.h>::av_image_alloc

    //av_image_alloc 进行显式的内存分配
    if (av_image_alloc(data, linesizes,pCodecCtx ->width, pCodecCtx->height, AV_PIX_FMT_RGB24, 16) < 0) {
        cout << "av_image_alloc failed" << endl;
        return -1;
    }

    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                     pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24,
                                     SWS_BILINEAR, nullptr, nullptr, nullptr);//进行一个图像转换的操作
    //AV_PIX_FMT_RGB24: 输出图像的像素格式（在这里是RGB24）。
    //SWS_BILINEAR: 指定了转换过程中使用的算法，这里是双线性插值。
    //   第一个  nullptr: 如果你不需要指定源图像的裁剪区域，可以将这个参数设置为 nullptr。
    //   第二个  nullptr: 如果你不需要指定目标图像的裁剪区域，同样可以将这个参数设置为 nullptr。
    //   第三个  nullptr: 如果你不需要进行额外的操作，可以将这个参数设置为 nullptr。
    if(img_convert_ctx==NULL)
    {
        cout<<"sws_getContext分配失败"<<endl;
        return -1;
    }


    //下面是要读取packet（存放未解码的数据，frame存放解码后的数据）
    int count=0;
    while(av_read_frame(pFormatCtx,packet)>=0) //读取到packet中
    {
        if(packet->stream_index==videoStream)
        {
            avcodec_send_packet(pCodecCtx,packet); //将packet发送给这个解码器进行解码
            if(avcodec_receive_frame(pCodecCtx,pFrame)==0){ //从ffmpeg拿解码后的数据
                sws_scale(img_convert_ctx,pFrame->data,pFrame->linesize,0,pFrame->height,data,linesizes);
                //将yuv转化成rgb数据
                string filename="D:/ffmpeg_study_1";
                saveFrame(data,linesizes,pCodecCtx->width,pCodecCtx->height,count,filename);
                count++;
                if(count>1000) {
                    cout<<"成功保存"<<endl;
                    return 0; //这里就保存50张图片
                }//它会将RGB保存为PPM文件。
            }
            av_packet_unref(packet);
        }
    }
    av_freep(data);
    av_free(pFrame);
    //av_free(pFrameRGB);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx); //传给一个二级指针
    return 0;
}
