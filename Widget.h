#pragma once

#include <QWidget>

class ImageViewerPrivate;
class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    ImageViewerPrivate *m_pImpl;
};
