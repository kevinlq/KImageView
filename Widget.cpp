#include "Widget.h"

#include "imageview.h"
#include "imageviewerfile.h"

// Qt lib file
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QDebug>

struct ImageViewerPrivate
{
    QString displayName;
    QSharedPointer<ImageViewer::Internal::ImageViewerFile> file;
    ImageViewer::Internal::ImageView *m_pImageView = nullptr;
};

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , m_pImpl(new ImageViewerPrivate)
{
    m_pImpl->file.reset(new ImageViewer::Internal::ImageViewerFile);
    m_pImpl->m_pImageView = new ImageViewer::Internal::ImageView(m_pImpl->file.data());

    auto buttonZoomIn = new QToolButton(this);
    buttonZoomIn->setText("zoomIn");
    auto buttonZoomOut = new QPushButton("zoomOut",this);
    auto buttonRestoreOrg = new QPushButton("org",this);
    auto toolButtonFitToScreen = new QPushButton("fitScreen",this);
    auto toolButtonBackground = new QToolButton(this);
    toolButtonBackground->setText("background");
    toolButtonBackground->setCheckable(true);

    auto toolButtonOutline = new QToolButton( this);
    toolButtonOutline->setText("outline");
    toolButtonOutline->setCheckable(true);

    auto toolButtonPlayPause = new QToolButton(this);
    toolButtonPlayPause->setText("play");
    toolButtonPlayPause->setCheckable(true);

    auto buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(8);
    buttonLayout->addWidget(buttonZoomIn);
    buttonLayout->addWidget(buttonZoomOut);
    buttonLayout->addWidget(buttonRestoreOrg);
    buttonLayout->addWidget(toolButtonFitToScreen);
    buttonLayout->addWidget(toolButtonBackground);
    buttonLayout->addWidget(toolButtonOutline);
    buttonLayout->addWidget(toolButtonPlayPause);

    buttonLayout->addStretch();

    auto mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(m_pImpl->m_pImageView);
    setLayout(mainLayout);

    connect(buttonZoomIn, &QAbstractButton::clicked, m_pImpl->m_pImageView,
            &ImageViewer::Internal::ImageView::zoomIn);
    connect(buttonZoomOut, &QAbstractButton::clicked, m_pImpl->m_pImageView,
            &ImageViewer::Internal::ImageView::zoomOut);
    connect(buttonRestoreOrg, &QAbstractButton::clicked, m_pImpl->m_pImageView,
            &ImageViewer::Internal::ImageView::resetToOriginalSize);
    connect(toolButtonFitToScreen, &QAbstractButton::clicked, m_pImpl->m_pImageView,
            &ImageViewer::Internal::ImageView::fitToScreen);
    connect(toolButtonBackground, &QAbstractButton::toggled, m_pImpl->m_pImageView,
            &ImageViewer::Internal::ImageView::setViewBackground);
    connect(toolButtonOutline, &QAbstractButton::toggled, m_pImpl->m_pImageView,
            &ImageViewer::Internal::ImageView::setViewOutline);
    connect(toolButtonPlayPause, &QAbstractButton::toggled, m_pImpl->file.data(),
            &ImageViewer::Internal::ImageViewerFile::setPaused);

    connect(m_pImpl->file.data(), &ImageViewer::Internal::ImageViewerFile::openFinished,
            m_pImpl->m_pImageView, &ImageViewer::Internal::ImageView::createScene);
    connect(m_pImpl->file.data(), &ImageViewer::Internal::ImageViewerFile::reloadFinished,
            m_pImpl->m_pImageView, &ImageViewer::Internal::ImageView::createScene);
    connect(m_pImpl->file.data(), &ImageViewer::Internal::ImageViewerFile::aboutToReload,
            m_pImpl->m_pImageView, &ImageViewer::Internal::ImageView::reset);

    QString strError = "";
    QString strFile = "D:/ui.gif";
    QString realFn = strFile;
    ImageViewer::Internal::ImageViewerFile::OpenResult result = m_pImpl->file->open(&strError, strFile, realFn);

    //qDebug() << "open file :" << (int)result << strError << (int)m_pImpl->file->type();
}

Widget::~Widget()
{
    delete m_pImpl;
    m_pImpl = nullptr;
}

