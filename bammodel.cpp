#include "bammodel.h"

BamModel::BamModel(QObject *parent)
{
    Q_UNUSED(parent)
    readSettings();
    connect(this, SIGNAL(newVersionFound(bool, QString,QString)),
           this, SLOT(setNewVersion(bool, QString,QString)),
           Qt::UniqueConnection);
    _progress=NULL;
    _totalFilesSize = _alreadyDownloaded = 0;
    _networkManager = NULL;
}

void BamModel::addObject(BamListObject &blo){
    if(Settings::getIngoreUnchecked()){
        QStringList ignored = Settings::getIgnoredAddons();

        if(ignored.contains(blo.getAddonPath())){
            return;
        }
    }

    QDir dir = _addonsDir+blo.getAddonPath();

    if( (blo.getActionType()=="Delete") && !dir.exists() ){
        //there's no point to remove something what doesn't exist
        return;
    }else if(blo.getActionType() == "Add"){
        if(dir.exists()){
            //getting sure that user has the same files version
            QCryptographicHash crypto(QCryptographicHash::Md5);
            crypto.reset();

            dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
            QStringList list = dir.entryList();
            list.sort();

            for(int i=0; i<list.count(); i++){
                QFile file(dir.filePath(list.at(i)));
                file.open(QFile::ReadOnly);
                while(!file.atEnd()){
                    crypto.addData(file.read(8192));
                }
                file.close();
            }

            if(blo.getMd5sum() == crypto.result().toHex()){
                return;
            }else{
                blo.setActionType("Replace");
            }
        }
        _totalFilesSize += blo.getFileSize();
        emit totalFilesSizeChanged(_totalFilesSize-_alreadyDownloaded);
    }

    this->_bamList.append(blo);
}

int BamModel::rowCount(const QModelIndex &parent) const{
    Q_UNUSED(parent)
    return this->_bamList.count();
}

int BamModel::columnCount(const QModelIndex &parent) const{
    Q_UNUSED(parent)
    return 2;
}

QVariant BamModel::data(const QModelIndex &index, int role) const{
    if (!index.isValid()){
        return QVariant();
    }
    if (index.column()==0 && role==Qt::DisplayRole){
        return this->_bamList.at(index.row()).getInfo();
    }else if(index.column()==1 && role==Qt::DecorationRole){
        return this->_bamList.at(index.row()).getThumbnail();
    }else if(index.column()==0 && role == Qt::CheckStateRole) // this shows checkbox
    {
        if(this->_bamList.at(index.row()).isChecked()){
            return Qt::Checked;
        }else{
            return Qt::Unchecked;
        }
    }
    return QVariant();
}

void BamModel::setCheckBox(QModelIndex index){
    if (!index.isValid()){
            return;
    }

    BamListObject *blo = &_bamList[index.row()];

    if(blo->isChecked()){
        _totalFilesSize -= blo->getFileSize();
        blo->setChecked(false);
    }else{
        _totalFilesSize += blo->getFileSize();
        blo->setChecked(true);
    }

    emit totalFilesSizeChanged(_totalFilesSize);
    reset();
}


QVariant BamModel::headerData(int section, Qt::Orientation orientation, int role) const{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)
    return QVariant();
}


void BamModel::downloadList(QUrl url){
    if(_networkManager!=NULL){
        delete _networkManager;
    }
    _networkManager = new QNetworkAccessManager();
    connect(_networkManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(handleXMLList(QNetworkReply*)));
    _url = url;
    _networkManager->get(QNetworkRequest(url));
}

void BamModel::downloadFromPath(QString path){
    QUrl url = _url;
    url.setPath(path);
    //thumbReplays.append(_networkManager.get(QNetworkRequest(url)));
}

void BamModel::downloadAddon(QString path){
    if(_abort) return;

    _mutex.lock();

    _downloadQueue.append(path);
    if(_downloadQueue.count()==1){
        downloadNext();
    }
    _mutex.unlock();
}

void BamModel::downloadNext(){
    if(_downloadQueue.count()>0 && !_abort){
        QUrl url = _url;
        url.setPath(_downloadQueue.first());
        _reply = _networkManager->get(QNetworkRequest(url));

        if(_progress != NULL){
            QString filename = url.toString();
            _progressFileName = filename.mid(filename.lastIndexOf("/")+1); //cuting string from last "/" char
            _progress->infoLine1->setText(tr("Downloading:"));
            _progress->infoLine2->setText(_progressFileName + " (0%)");

            connect(_reply, SIGNAL(downloadProgress(qint64,qint64)),
                    this, SLOT(setProgress(qint64,qint64)));
            connect(_networkManager, SIGNAL(finished(QNetworkReply*)),
                    this, SLOT(downloadFinished(QNetworkReply*)));
        }
    }
}

void BamModel::downloadFinished(QNetworkReply *networkReply){
    if(_abort) return;
    disconnect(_networkManager, SIGNAL(finished(QNetworkReply*)),
               this, SLOT(downloadFinished(QNetworkReply*)));

    QUrl url = networkReply->url();
    handleAddon(networkReply);

    QMutableListIterator<BamListObject> i(_bamList);

    while(i.hasNext()){
        if(i.next().getDownloadPath() == url.path()){
            _alreadyDownloaded += i.peekPrevious().getFileSize();
            i.remove();
        }
    }

    emit totalFilesSizeChanged(_totalFilesSize-_alreadyDownloaded);

    _mutex.lock();

    int idx = _downloadQueue.indexOf(url.path());
    _downloadQueue.removeAt(idx);

    if(_downloadQueue.count()>0){
        downloadNext();
    }else if(_canFinish){
        allDone();
    }

    _mutex.unlock();
}

void BamModel::setProgress(qint64 bytesReceived, qint64 bytesTotal){
    if(_progress != NULL){
        _progress->infoLine2->setText(_progressFileName + " (" + QString::number(bytesReceived*100/bytesTotal) + "%)");
        _progress->progressBar->setValue(_alreadyDownloaded + bytesReceived);
    }
}

void BamModel::handleThumbnail(QNetworkReply *networkReply){
    QUrl url = networkReply->url();
    QMutableListIterator<BamListObject> i(_bamList);
    QPixmap thumb;

    while(i.hasNext()){
        if(i.next().getThumbnailPath() == url.path()){
            QByteArray response(networkReply->readAll());
            thumb.loadFromData(response,"PNG");
            i.peekPrevious().setThumbnail(thumb);
            break;
        }
    }
    networkReply->deleteLater();
}

void BamModel::handleAddon(QNetworkReply *networkReply){
    QUrl url = networkReply->url();
    QTemporaryFile file;

    if (file.open()) {
        file.write(networkReply->readAll());
    }
    JlCompress::extractDir(file.fileName(), _addonsDir);
    file.close();

    networkReply->deleteLater();
}

void BamModel::handleXMLList(QNetworkReply *networkReply){
    disconnect(_networkManager, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(handleXMLList(QNetworkReply*)));
    connect(_networkManager, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(handleThumbnail(QNetworkReply*)));

    BamListObject *tmpaction;
    QString newversion = "";

    if (!networkReply->error()) {
        QByteArray response(networkReply->readAll());
        QXmlStreamReader xml(response);
        QXmlStreamReader::TokenType token;

        while(!xml.atEnd() && !xml.hasError()) {
            token = xml.readNext();
            if(token == QXmlStreamReader::StartDocument) {
                continue;
            }
            if(token == QXmlStreamReader::StartElement) {
                //actions list
                if(xml.name() == "actions") {
                    continue;
                }else if(xml.name() == "action") {
                    //action element
                    tmpaction = new BamListObject;
                    while(!(token == QXmlStreamReader::EndElement && xml.name() == "action")) {
                        token = xml.readNext();
                        if(xml.tokenType() == QXmlStreamReader::StartElement){
                            if(xml.name() == "addon"){
                                //addon object
                                //Split to other method because this was getting too long and nested
                                this->parseXMLAddon(xml, *tmpaction);
                            }else if(xml.name() == "action_type"){
                                //what to do with addon
                                while(!(token == QXmlStreamReader::EndElement && xml.name() == "action_type")) {
                                    token = xml.readNext();
                                    if(xml.tokenType() == QXmlStreamReader::Characters){
                                        tmpaction->setActionType(xml.text().toString().trimmed());
                                    }
                                }
                            }
                        }
                    }
                    //single action element should be complete at this point
                    addObject(*tmpaction);
                    delete tmpaction;
                }else if(xml.name() == "version"){
                    while(!(token == QXmlStreamReader::EndElement && xml.name() == "version")) {
                        token = xml.readNext();
                        if(xml.tokenType() == QXmlStreamReader::Characters){
                            newversion = xml.text().toString().trimmed();
                        }
                    }
                }
            }
        }
    }

    if(newversion!=""){
        if(_bamList.count()>0){
            emit newVersionFound(false, _oldVersion, newversion);
        }else{
            emit newVersionFound(true, _oldVersion, newversion);
        }
    }else{
        emit newVersionFound(true, _oldVersion, newversion);
    }
    reset();
    emit xmlProcessed();
    networkReply->deleteLater();
}

void BamModel::parseXMLAddon(QXmlStreamReader& xml, BamListObject & blo){
    xml.readNext();
    QString extractedData;
    QStringRef name;

    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "addon")) {
        if(xml.tokenType() == QXmlStreamReader::StartElement) {
            name = xml.name();
            extractedData = this->parseXMLAddonElement(xml);
            if(name == "id") {
                blo.setId(extractedData);
            }else if(name == "type") {
                blo.setType(extractedData);
            }else if(name == "name") {
                blo.setName(extractedData);
            }else if(name == "author") {
                blo.setAuthor(extractedData);
            }else if(name == "mode") {
                blo.setMode(extractedData);
            }else if(name == "thumbnail") {
                blo.setThumbnail(QPixmap(":/loading.png"));
                blo.setThumbnailPath(extractedData);
                downloadFromPath(extractedData);
            }else if(name == "dlpath") {
                blo.setDownloadPath(extractedData);
            }else if(name == "file_size"){
                blo.setFileSize(extractedData);
            }else if(name == "md5sum"){
                blo.setMd5sum(extractedData);
            }
        }
        xml.readNext();
    }
}

QString BamModel::parseXMLAddonElement(QXmlStreamReader& xml){
    while(!(xml.tokenType() == QXmlStreamReader::EndElement)) {
        if(xml.tokenType() == QXmlStreamReader::Characters){
            return xml.text().toString().trimmed();
        }
        xml.readNext();
    }
    return QString(); //it will be never used, just removing warning
}

void BamModel::installChecked(){
    //QMutableListIterator<QNetworkReply*> thumbsIterator(thumbReplays);

    //while(thumbsIterator.hasNext()){
    //    QNetworkReply *r = thumbsIterator.next();
        //r->abort();
    //}

    disconnect(_networkManager, SIGNAL(finished(QNetworkReply*)),
                    this, SLOT(handleThumbnail(QNetworkReply*))); //thumbnails are no longer necessery
    connect(_networkManager, SIGNAL(finished(QNetworkReply*)),
                        this, SLOT(handleAddon(QNetworkReply*)));

    QDialog *dialog = new QDialog;
    _progress = new Ui::installationProgress;
    _progress->setupUi(dialog);
    _progress->progressBar->setMinimum(0);
    _progress->progressBar->setMaximum(0);
    dialog->connect(dialog, SIGNAL(rejected()),
                    this, SLOT(cancelInstallation()));
    dialog->show();

    QListIterator<BamListObject> i(_bamList);

    _canFinish = false;
    _abort=false;
    _alreadyDownloaded = 0;

    while(i.hasNext()){
        if(i.next().isChecked()){
            const BamListObject *tmp = &i.peekPrevious();
            if(tmp->getActionType() == "Add" || tmp->getActionType() == "Replace"){
                downloadAddon(tmp->getDownloadPath());
            }else if(tmp->getActionType() == "Delete"){
                removeDir(_addonsDir +tmp->getAddonPath());
            }
        }else{
            _ignoredList<<i.peekPrevious().getAddonPath();
        }
    }

    if(Settings::getIngoreUnchecked()){
        Settings::setIgnoredAddons(_ignoredList);
    }

    _progress->progressBar->setMaximum(_totalFilesSize);
    _canFinish = true;

    if(_downloadQueue.count()==0){
        allDone();
    }
}

void BamModel::readSettings(){
    QSettings settings("Bending software", "Beta addon downloader");

    _addonsDir = Settings().getAddonPath();
    _oldVersion = settings.value("updates/version", "0.0").toString();
    _ignoredList = Settings::getIgnoredAddons();
}

void BamModel::setNewVersion(bool empty, QString oldVersion, QString newVersion){
    Q_UNUSED(oldVersion);
    Q_UNUSED(empty);
    _newVersion = newVersion;
}

void BamModel::allDone(){
    if(_abort) return;

    if(_progress != NULL){
        _progress->infoLine1->setText("All done.");
        _progress->infoLine2->setText("");
        _progress->buttonBox->setStandardButtons(QDialogButtonBox::Ok);
    }

    qDebug()<<_newVersion;
    QSettings settings("Bending software", "Beta addon downloader");
    settings.setValue("updates/version", _newVersion);
    _oldVersion = _newVersion;

    emit newVersionFound(true, _oldVersion, _newVersion);

    _bamList.clear();

    if(Settings::getRunGameAfterInstall()){
        emit runGame();
    }
    reset();
}

void BamModel::cancelInstallation(){
   _mutex.lock();
   _abort=true;
   _reply->abort();
   _downloadQueue.clear();
   _mutex.unlock();
   reset();
   disconnect(_networkManager, SIGNAL(finished(QNetworkReply*)),
                           this, SLOT(handleAddon(QNetworkReply*)));
}

bool BamModel::removeDir(const QString &dirName)
{
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(info.absoluteFilePath());
            }
            else {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result) {
                return result;
            }
        }
        result = dir.rmdir(dirName);
    }

    return result;
}
