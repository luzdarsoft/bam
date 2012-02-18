#ifndef BAMLISTOBJECT_H
#define BAMLISTOBJECT_H

#include <QObject>
#include <QImage>
#include <QPixmap>
#include <QDebug>
#include <QDir>

class BamListObject : public QObject
{
    Q_OBJECT
public:
    explicit BamListObject(QObject *parent = 0);
    BamListObject(const BamListObject &blo);
    bool operator==(const BamListObject &blo) const;
    BamListObject operator=(const BamListObject &blo);
    void setId(const QString id);
    void setId(const int id);
    void setType(const QString type);
    void setName(const QString name);
    void setAuthor(const QString author);
    void setMode(const QString mode);
    void setActionType(const QString actionType);
    void setThumbnailPath(const QString path);
    void setDownloadPath(const QString path);
    void setInfo(const int id, const QString type, const QString name, const QString author, const QString mode, const QString actionType);
    void setThumbnail(const QPixmap &thumbnail);
    void setFileSize(const QString size);
    void setMd5sum(const QString md5sum);
    void setChecked(const bool checked);

    QString getThumbnailPath() const;
    QString getDownloadPath() const;
    QString getInfo() const;
    QPixmap getThumbnail() const;
    QString getActionType() const;
    QString getAddonPath() const;
    qint64 getFileSize() const;
    bool isChecked() const;
    QString getMd5sum() const;

private:
    int _id;
    QString _type;
    QString _name;
    QString _author;
    QString _mode;
    QString _actionType;
    QPixmap _thumbnail;
    QString _thumbnailPath;
    QString _downloadPath;
    qint64 _fileSize;
    bool _checkedItem;
    QString _md5sum;
    void _myInit(const BamListObject &blo);
signals:

public slots:

};

#endif // BAMLISTOBJECT_H
