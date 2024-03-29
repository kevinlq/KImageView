/****************************************************************************
**
** Copyright (C) 2016 Denis Mingulov.
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "imageviewerfile.h"
#include "qtcassert.h"

#include <QFileInfo>
#include <QGraphicsPixmapItem>
#ifndef QT_NO_SVG
#include <QGraphicsSvgItem>
#endif
#include <QImageReader>
#include <QMovie>
#include <QPainter>
#include <QPixmap>
#include <QDebug>

namespace ImageViewer {
namespace Internal {

class MovieItem : public QObject, public QGraphicsPixmapItem
{
public:
    MovieItem(QMovie *movie)
        : m_movie(movie)
    {
        setPixmap(m_movie->currentPixmap());
        connect(m_movie, &QMovie::updated, this, [this](const QRectF &rect) {
            update(rect);
        });
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
    {
        const bool smoothTransform = painter->worldTransform().m11() < 1;
        painter->setRenderHint(QPainter::SmoothPixmapTransform, smoothTransform);
        painter->drawPixmap(offset(), m_movie->currentPixmap());
    }

private:
    QMovie *m_movie = nullptr;
};

ImageViewerFile::ImageViewerFile()
{
}

ImageViewerFile::~ImageViewerFile()
{
    cleanUp();
}

ImageViewerFile::OpenResult ImageViewerFile::open(QString *errorString, const QString &fileName)
{
    OpenResult success = openImpl(errorString, fileName);
    emit openFinished(success == OpenResult::Success);
    return success;
}

ImageViewerFile::OpenResult ImageViewerFile::openImpl(QString *errorString, const QString &fileName)
{
    cleanUp();

    if (!QFileInfo(fileName).isReadable())
    {
        return OpenResult::ReadError;
    }

    QByteArray format = QImageReader::imageFormat(fileName);
    // if it is impossible to recognize a file format - file will not be open correctly
    if (format.isEmpty())
    {
        if (errorString)
        {
            *errorString = tr("Image format not supported.");
        }

        return OpenResult::CannotHandle;
    }

#ifndef QT_NO_SVG
    if (format.startsWith("svg"))
    {
        m_tempSvgItem = new QGraphicsSvgItem(fileName);
        QRectF bound = m_tempSvgItem->boundingRect();
        if (qFuzzyIsNull(bound.width()) && qFuzzyIsNull(bound.height()))
        {
            delete m_tempSvgItem;
            m_tempSvgItem = 0;
            if (errorString)
            {
                *errorString = tr("Failed to read SVG image.");
            }
            return OpenResult::CannotHandle;
        }
        m_type = TypeSvg;
        emit imageSizeChanged(m_tempSvgItem->boundingRect().size().toSize());
    }
    else
#endif
    if (QMovie::supportedFormats().contains(format))
    {
        m_type = TypeMovie;
        m_movie = new QMovie(fileName, QByteArray(), this);
        m_movie->setCacheMode(QMovie::CacheAll);
        connect(m_movie, &QMovie::finished, m_movie, &QMovie::start);
        connect(m_movie, &QMovie::resized, this, &ImageViewerFile::imageSizeChanged);
        m_movie->start();
        m_isPaused = false; // force update
        setPaused(true);
    }
    else
    {
        m_pixmap = new QPixmap(fileName);
        if (m_pixmap->isNull())
        {
            if (errorString)
            {
                *errorString = tr("Failed to read image.");
            }

            delete m_pixmap;
            m_pixmap = nullptr;
            return OpenResult::CannotHandle;
        }
        m_type = TypePixmap;
        emit imageSizeChanged(m_pixmap->size());
    }

    m_strFilePath = fileName;
    return OpenResult::Success;
}

ImageViewerFile::ReloadBehavior ImageViewerFile::reloadBehavior(ChangeTrigger state, ChangeType type) const
{
    if (type == TypeRemoved || type == TypePermissions)
    {
        return BehaviorSilent;
    }

    if (type == TypeContents && state == TriggerInternal && !isModified())
    {
        return BehaviorSilent;
    }

    return BehaviorAsk;
}

bool ImageViewerFile::reload(QString *errorString,
                             ReloadFlag flag,
                             ChangeType type)
{
    if (flag == FlagIgnore)
    {
        return true;
    }

    if (type == TypePermissions)
    {
        emit changed();
        return true;
    }

    emit aboutToReload();
    bool success = (openImpl(errorString, filePath()) == OpenResult::Success);
    emit reloadFinished(success);
    return success;
}

bool ImageViewerFile::isPaused() const
{
    return m_isPaused;
}

void ImageViewerFile::setPaused(bool paused)
{
    if (nullptr == m_movie || m_isPaused == paused)
    {
        return;
    }

    m_isPaused = paused;
    m_movie->setPaused(paused);
    emit isPausedChanged(m_isPaused);
}

QGraphicsItem *ImageViewerFile::createGraphicsItem() const
{
    QGraphicsItem *val = nullptr;
    switch (m_type)
    {
    case TypeInvalid:
        break;
    case TypeSvg:
#ifndef QT_NO_SVG
        if (m_tempSvgItem)
        {
            val = m_tempSvgItem;
            m_tempSvgItem = nullptr;
        }
        else
        {
            val = new QGraphicsSvgItem(filePath());
        }
#endif
        break;
    case TypeMovie:
        val = new MovieItem(m_movie);
        break;
    case TypePixmap:
    {
        auto pixmapItem = new QGraphicsPixmapItem(*m_pixmap);
        pixmapItem->setTransformationMode(Qt::SmoothTransformation);
        val = pixmapItem;
        break;
    }
    default:
        break;
    }
    return val;
}

ImageViewerFile::ImageType ImageViewerFile::type() const
{
    return m_type;
}

void ImageViewerFile::updateVisibility()
{
    if (!m_movie || m_isPaused)
    {
        return;
    }
    bool visible = true;
    m_movie->setPaused(!visible);
}

bool ImageViewerFile::isModified() const
{
    return false;
}

QString ImageViewerFile::filePath() const
{
    return m_strFilePath;
}

void ImageViewerFile::cleanUp()
{
    delete m_pixmap;
    m_pixmap = nullptr;
    delete m_movie;
    m_movie = nullptr;
#ifndef QT_NO_SVG
    delete m_tempSvgItem;
    m_tempSvgItem = nullptr;
#endif
    m_type = TypeInvalid;
}

} // namespace Internal
} // namespace ImageViewer
