#ifndef PARSE_H
#define PARSE_H


extern "C" {
#include"libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libavdevice/avdevice.h"
#include"libswscale/swscale.h"
#include<libavutil/imgutils.h>
}
#include <QThread>
#include<QImage>
#include<fstream>
#include<iostream>
#include<string>
using namespace std;


class Parse : public QThread //这个线程是用来进行读取视频和解码视频的
{
    Q_OBJECT
public:
    explicit Parse(QObject *parent = nullptr);
    bool decide=true;

private:
    AVFormatContext *pFormatCtx=NULL;
    AVCodecContext * pCodecCtx=NULL;

    AVCodecParameters *pVideoCodecPar=NULL;

    const AVCodec *pCodec=NULL;
    AVFrame *pFrame_in=NULL;
    //AVFrame *pFrame_out=NULL;//其实也可以直接将data以及linesize保存到一个pFrame中，就不用开data和linesizes了
    AVPacket *packet=NULL;
    uint8_t *out_buffer=NULL;
     struct SwsContext *img_convert_ctx=NULL;
    uint8_t *data[4]={nullptr};
    int linesizes[4]={0};

    int videoStream,i;

    int count=0; //存放数据的数量，可能不会用得到
    void saveFrame(uint8_t **data,int *linesizes,int width,int height,int count,string filename);

protected:
    void run() override;


signals:
    void send_image(QImage image);

};

#endif // PARSE_H
