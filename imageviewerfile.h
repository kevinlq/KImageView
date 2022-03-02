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

#pragma once

#include <QObject>

QT_BEGIN_NAMESPACE
class QGraphicsItem;
class QMovie;
class QPixmap;

#ifndef QT_NO_SVG
class QGraphicsSvgItem;
#endif

QT_END_NAMESPACE

namespace ImageViewer {
namespace Internal {

class ImageViewerFile : public QObject
{
    Q_OBJECT

public:
    enum class OpenResult {
        Success,
        ReadError,
        CannotHandle
    };
    enum ImageType {
        TypeInvalid,
        TypeSvg,
        TypeMovie,
        TypePixmap
    };
    enum ChangeTrigger {
        TriggerInternal,
        TriggerExternal
    };
    enum ChangeType {
        TypeContents,
        TypePermissions,
        TypeRemoved
    };

    enum ReloadBehavior {
        BehaviorAsk,
        BehaviorSilent
    };

    enum ReloadFlag {
        FlagReload,
        FlagIgnore
    };

    ImageViewerFile();
    ~ImageViewerFile() override;

    OpenResult open(QString *errorString, const QString &fileName,
                    const QString &realFileName);

    ReloadBehavior reloadBehavior(ChangeTrigger state, ChangeType type) const;
    bool reload(QString *errorString, ReloadFlag flag, ChangeType type) ;

    bool isPaused() const;
    void setPaused(bool paused);

    QGraphicsItem *createGraphicsItem() const;
    ImageType type() const;

    void updateVisibility();

    bool isModified() const;
    QString filePath() const;

signals:
    void changed();
    void aboutToReload();
    void reloadFinished(bool success);
    void openFinished(bool success);
    void imageSizeChanged(const QSize &size);
    void isPausedChanged(bool paused);

private:
    void cleanUp();
    OpenResult openImpl(QString *errorString, const QString &fileName);

    ImageType m_type = TypeInvalid;
#ifndef QT_NO_SVG
    mutable QGraphicsSvgItem *m_tempSvgItem = nullptr;
#endif
    QMovie *m_movie = nullptr;
    QPixmap *m_pixmap = nullptr;
    bool m_isPaused = false;
    QString m_strFilePath = "";
};

} // namespace Internal
} // namespace ImageViewer
