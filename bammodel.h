#ifndef BAMMODEL_H
#define BAMMODEL_H

#include "bamlistobject.h"
#include "ui_installationProgress.h"
#include "quazip/JlCompress.h"
#include "settings.h"

#include <QAbstractTableModel>
#include <QtNetwork>
#include <QDebug>

class BamModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit BamModel(QObject *parent = 0);
    void addObject(BamListObject &blo);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    static bool removeDir(const QString &dirName);

    //QList<QNetworkReply*> thumbReplays;


signals:
    void xmlProcessed();
    void dataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight);
    void newVersionFound(const bool empty, const QString oldVersion, const QString newVersion);
    void totalFilesSizeChanged(const int newSize);
    void runGame();

public slots:
    void handleXMLList(QNetworkReply *networkReply);
    void handleThumbnail(QNetworkReply *networkReply);
    void handleAddon(QNetworkReply *networkReply);
    void setCheckBox(QModelIndex index);
    void installChecked();
    void setNewVersion(bool empty, QString oldVersion, QString newVersion);
    void setProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished(QNetworkReply* networkReply);
    void cancelInstallation();
    void downloadList(QUrl url);

private:
    QNetworkAccessManager *_networkManager;
    QNetworkReply *_reply; //used for addons
    QUrl _url;
    QList<BamListObject> _bamList;
    QString _addonsDir;
    QString _newVersion;
    QString _oldVersion;
    Ui::installationProgress *_progress;
    QString _progressFileName;
    QStringList _downloadQueue;
    QMutex _mutex;
    bool _canFinish;
    qint64 _totalFilesSize;
    qint64 _alreadyDownloaded;
    bool _abort;
    QStringList _ignoredList;

    void parseXMLAddon(QXmlStreamReader & xml, BamListObject & blo);
    QString parseXMLAddonElement(QXmlStreamReader & xml);
    void downloadFromPath(QString path);
    void downloadAddon(QString path);
    void downloadNext();
    void readSettings();
    void allDone();
};

#endif // BAMMODEL_H

