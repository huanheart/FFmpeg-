#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    Mythread=new Parse();
    Mythread->start();
    connect(Mythread,&Parse::send_image,this,[=](QImage image){
        Myimage=image;
        if(image.isNull()) //看看是否有图片没有打印成功
        cout<<1<<endl;
        update(); //执行重绘事件
    });
}

Widget::~Widget()
{
    delete Mythread;
    delete ui;
}

void Widget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setBrush(Qt::black);
    painter.drawRect(0,0,this->width(),this->height());
    if(Myimage.size().width()<=0) return ;//说明还没有进行一个图片的赋值
    QImage img=Myimage.scaled(this->size(),Qt::KeepAspectRatio);//将图像进行一个缩放然后给到img
    int x=this->width()-img.width();
    int y=this->height()-img.height();
    x/=2;
    y/=2;
    painter.drawImage(QPoint(x,y),img);
    return ;
}
void Widget::closeEvent(QCloseEvent *event)
{
    Mythread->decide=false;
    QThread::msleep(30); //这里
    QWidget::closeEvent(event);
}

