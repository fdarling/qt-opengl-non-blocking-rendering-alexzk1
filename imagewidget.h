#include <QWidget>
#include <QImage>

class ImageWidget : public QWidget
{
    Q_OBJECT
public:
    ImageWidget(QWidget *parent = nullptr);
    ~ImageWidget();
    void setImage(const QImage &newImage);
protected:
    void paintEvent(QPaintEvent *event);
    
    QImage img;
};
