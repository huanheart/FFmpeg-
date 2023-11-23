#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include<parse.h>
#include<QPainter>
QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void paintEvent(QPaintEvent *event) override;
    void closeEvent(QCloseEvent * event) override;
private:
    QImage Myimage;
    Parse * Mythread=nullptr;
    Ui::Widget *ui;

};
#endif // WIDGET_H
