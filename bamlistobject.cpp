#include "bamlistobject.h"

BamListObject::BamListObject(QObject *parent) :
    QObject(parent)
{
    _checkedItem=true;
}

void BamListObject::_myInit(const BamListObject &blo){
    setInfo(blo._id, blo._type, blo._name, blo._author, blo._mode, blo._actionType);
    _thumbnail = blo._thumbnail;
    _thumbnailPath = blo._thumbnailPath;
    _checkedItem = blo._checkedItem;
    _downloadPath = blo._downloadPath;
    _fileSize = blo._fileSize;
}

BamListObject::BamListObject(const BamListObject &blo){
    this->_myInit(blo);
}

BamListObject BamListObject::operator=(const BamListObject &blo)
{
    this->_myInit(blo);
    return *this;
}

bool BamListObject::operator==(const BamListObject &blo) const
{
    return this->_id == blo._id;
}

void BamListObject::setId(const QString id){ _id=id.toInt(); }
void BamListObject::setId(const int id){ _id=id; }
void BamListObject::setType(const QString type){ _type=type; }
void BamListObject::setName(const QString name){ _name=name; }
void BamListObject::setAuthor(const QString author){ _author=author; }
void BamListObject::setMode(const QString mode){ _mode=mode; }
void BamListObject::setActionType(const QString actionType){ _actionType= actionType; }
void BamListObject::setThumbnail(const QPixmap &thumbnail){
    _thumbnail=thumbnail;
}
void BamListObject::setThumbnailPath(const QString path){ _thumbnailPath=path; }
void BamListObject::setDownloadPath(const QString path){ _downloadPath=path; }
void BamListObject::setFileSize(const QString size){_fileSize=size.toLongLong();}
void BamListObject::setMd5sum(const QString md5sum){ _md5sum=md5sum; }
void BamListObject::setChecked(const bool checked){
    _checkedItem = checked; }

void BamListObject::setInfo(const int id, const QString type, const QString name, const QString author, const QString mode, const QString actionType){
    _id=id;
    _type=type;
    _name=name;
    _author=author;
    _mode=mode;
    _actionType=actionType;
}

bool BamListObject::isChecked() const { return _checkedItem; }
QString BamListObject::getThumbnailPath() const{ return _thumbnailPath; }
QString BamListObject::getDownloadPath() const{ return _downloadPath; }
QString BamListObject::getActionType() const{ return _actionType; }
qint64 BamListObject::getFileSize() const{ return _fileSize; }
QString BamListObject::getMd5sum() const{ return _md5sum; }

QPixmap BamListObject::getThumbnail() const{
    if(this->_thumbnail.isNull()){
        return QPixmap(":/noPreviewAvaible.png");
    }else{
        return this->_thumbnail;
    }
}

QString BamListObject::getInfo() const{
    return tr("Action type: %1\n"
              "Type: %2\n"
              "Name: %3\n"
              "Author: %4\n"
              "Mode: %5").arg(this->_actionType,
                                this->_type,
                                this->_name,
                                this->_author,
                                this->_mode);
}

QString BamListObject::getAddonPath() const{
    QString typedir;
    if(_type == "Map"){
        typedir = "Maps";
    }else if(_type == "Theme"){
        typedir = "Themes";
    }else{
        return "";
    }
    return QDir::toNativeSeparators(typedir+"/"+_name);
}
