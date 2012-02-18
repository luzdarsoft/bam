#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QCoreApplication::setOrganizationName("Bending software");
    QCoreApplication::setApplicationName("Beta addon downloader");

    _model = new BamModel;
    ui->addonList->setModel(_model);
    downloadList();

    connect(ui->actionSettings, SIGNAL(triggered()),
            this, SLOT(openSettings()));
    connect(ui->runGame, SIGNAL(clicked()),
            this, SLOT(runGame()));
}

void MainWindow::resizeView(){
    ui->addonList->resizeColumnsToContents();
    ui->addonList->resizeRowsToContents();
}

void MainWindow::setNewVersionInfo(const bool empty, const QString oldVersion, const QString newVersion){
    if((oldVersion == newVersion) || empty){
        ui->updateTo->setText(tr("Everything is up to date"));
        ui->install->setDisabled(true);
    }else{
        ui->updateTo->setText(tr("Update from %1 to %2")
                              .arg(oldVersion)
                              .arg(newVersion));
        ui->install->setDisabled(false);
    }
}

void MainWindow::setTotalFilesSize(const int fileSize){
    ui->downloadSize->setText(tr("Download size: %1 MB")
                              .arg(QString::number((float)fileSize/(1024*1024), 'f', 1)));
}

void MainWindow::openSettings(){
    Settings *sWindow = new Settings(this);
    sWindow->show();
    connect(sWindow, SIGNAL(accepted()),
            this, SLOT(downloadList()));
}

void MainWindow::runGame()
{
    bool success = true;
#ifdef Q_WS_WIN
    //windows version of hedgewars require workdir
    QString dirname = Settings::getHedgewarsBinary();
    dirname.truncate(dirname.lastIndexOf("\\")-4); //i hope it's not weird installation
    success = QProcess::startDetached(Settings::getHedgewarsBinary(), QStringList(), dirname);
#else
    success = QProcess::startDetached(Settings::getHedgewarsBinary());
#endif
    if(success){
        if(Settings::getCloseOnHwStart()){
            //QApplication::quit();
        }
    }else{
        QMessageBox msgBox;
        msgBox.setText("Can't run hedgewars game. Try setting up correct path in Options->Settings->Game path");
        msgBox.exec();
    }
}

void MainWindow::downloadList()
{
    QSettings settings;
    QString oldVersion = settings.value("updates/version", "0.0").toString();

    delete _model;
    _model = new BamModel;
    ui->addonList->setModel(_model);

    //_model->downloadList(QUrl("http://127.0.0.1:8000/bam/addon_pack/" + oldVersion + "/"));
    _model->downloadList(QUrl("http://hh.unit22.org/bam/addon_pack/" + oldVersion + "/"));

    connect(_model, SIGNAL(xmlProcessed()),
            this, SLOT(resizeView()));
    connect(ui->addonList ,SIGNAL(clicked(QModelIndex)),
            _model, SLOT(setCheckBox(QModelIndex)), Qt::UniqueConnection );
    connect(ui->install, SIGNAL(clicked()),
            _model, SLOT(installChecked()));
    connect(_model, SIGNAL(newVersionFound(bool, QString, QString)),
            this, SLOT(setNewVersionInfo(bool, QString, QString)));
    connect(_model, SIGNAL(totalFilesSizeChanged(int)),
            this, SLOT(setTotalFilesSize(int)));
    connect(_model, SIGNAL(runGame()),
            this, SLOT(runGame()));
}

void MainWindow::hwRunError(QProcess::ProcessError error)
{
    qDebug()<<error;
}

MainWindow::~MainWindow()
{
    delete ui;
}
