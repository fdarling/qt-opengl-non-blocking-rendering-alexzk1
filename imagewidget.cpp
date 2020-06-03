#include "imagewidget.h"

#include <QPainter>

ImageWidget::ImageWidget(QWidget *parent) : QWidget(parent)
{
}

ImageWidget::~ImageWidget()
{
}

void ImageWidget::setImage(const QImage &newImage)
{
    img = newImage;
    update();
}

void ImageWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.drawImage(0, 0, img);
}
