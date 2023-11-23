#include "parse.h"

Parse::Parse(QObject*parent)
    : QThread{parent}
{

}

void Parse::run()
{
    avdevice_register_all();
    unsigned version=avcodec_version();
    cout<<"版本是"<<' '<<version<<endl;

        pFormatCtx=avformat_alloc_context();

    const char * file_path="D:\\CloudMusic\\MV\\one.mp4"; //这个\要改成/
    if(avformat_open_input(&pFormatCtx,file_path,NULL,NULL)!=0)//打开输入流并读取标头。
    {//后面两个参数分别为AVInputFormat指定视频格式，AVDictionary指定码流的各种参数，传空的话FFmpeg会自动检测。
        cout<<"打开文件失败"<<endl;
        return ;
    }   //pFormatCtx包含了文件的格式信息。

    if(avformat_find_stream_info(pFormatCtx,NULL)<0){
        cout<<"获取信息失败"<<endl;
    }//这个函数用于获取多媒体文件的流信息

    videoStream=-1;
    for(int i=0;i<pFormatCtx->nb_streams;i++) //nb_streams是AVFormatContext.streams中的元素数量。
    {
        if(pFormatCtx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO) //stream是文件中所有流的列表。
        {
            videoStream=i;
            break;
        }
    }
    if(videoStream==-1)
    {
        cout<<"没有这个视频流"<<endl;
        return ;
    }

    pVideoCodecPar=pFormatCtx->streams[videoStream]->codecpar;  //codecpar是 与该流关联的编解码器参数

    pCodec=avcodec_find_decoder(pVideoCodecPar->codec_id); //这种要改成AVCodecParameters这种结构，由于AVStream.codec
    if(pCodec==NULL)
    {
        cout<<"没有找到对应的解码器"<<endl;
        return ;
    }
    //avcodec_open2前必须使用avcodec_alloc_context3生成context,
    pCodecCtx=avcodec_alloc_context3(pCodec);    //分配AVCodecContext并将其字段设置为默认值。
    if(avcodec_parameters_to_context(pCodecCtx,pVideoCodecPar)<0)   //根据提供的编解码器参数的值填充编解码器上下文。
    {
        cout<<"编解码器参数的值填充编解码器上下文失败"<<endl;
        return ;
    }
    //打开解码器
    if(avcodec_open2(pCodecCtx,pCodec,NULL)<0)
    {
        cout<<"打开解码器失败"<<endl;
        return ;
    }

    //思考，这个的话我读取一个视频，她不用进行一个解复用吗？？
    pFrame_in=av_frame_alloc();
    packet=av_packet_alloc();
    if (av_image_alloc(data, linesizes,pCodecCtx ->width, pCodecCtx->height, AV_PIX_FMT_RGB24, 16) < 0) {
        cout << "av_image_alloc failed" << endl;
        return ;
    }
    //av_image_alloc 创建了用于存储图像数据的内存空间,这里对data和linesizes只是开辟了一个空间
//    data 是一个指向图像数据的指针，也就是图像存储的实际内容。
//    linesizes 是一个数组，存储图像每一行的大小（以字节为单位）

    //img_convert_ctx这个东西用于图像转化的，里面包含了图像转化的上下文
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                     pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24,
                                     SWS_BILINEAR, nullptr, nullptr, nullptr);
    if(img_convert_ctx==NULL)
    {
        cout<<"sws_getContext分配失败"<<endl;
        return ;
    }

    while(av_read_frame(pFormatCtx,packet)>=0&&decide==true) //读取到packet中
    {
        if(packet->stream_index==videoStream)
        {
            avcodec_send_packet(pCodecCtx,packet); //将packet发送给这个解码器进行解码
            if(avcodec_receive_frame(pCodecCtx,pFrame_in)==0){ //从ffmpeg拿解码后的数据
                sws_scale(img_convert_ctx,pFrame_in->data,pFrame_in->linesize,0,pFrame_in->height,data,linesizes);
                //将yuv转化成rgb数据,这里是直接将pframe中的数据直接通过img_convert_ctx，转化给到data和linesizes
                QImage temp_image(data[0],pCodecCtx->width,pCodecCtx->height,linesizes[0],QImage::Format_RGB888);
                //图像使用 24 位 RGB 格式 (8-8-8) 存储。
                QImage image=temp_image;
                emit send_image(image);
                msleep(42); //一秒播放24帧左右
            }
            av_packet_unref(packet);
        }
    }
    av_freep(data);
    av_free(pFrame_in);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx); //传给一个二级指针
    return ;
}














