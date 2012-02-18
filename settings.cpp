#include "settings.h"
#include "ui_settings.h"
#include <QDebug>
Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    QCoreApplication::setOrganizationName("Bending software");
    QCoreApplication::setApplicationName("Beta addon downloader");

    ui->setupUi(this);
    ui->addonsPath->setText(getAddonPath());
    ui->gamePath->setText(getHedgewarsBinary());
    ui->ignoredAddons->addItems(getIgnoredAddons());
    ui->runGameAfterInst->setChecked(getRunGameAfterInstall());
    ui->ignoreUnchecked->setChecked(getIngoreUnchecked());
    ui->versionInfo->setText(tr("Local version: %1")
                             .arg(getLocalVersion()));

    connect(this, SIGNAL(accepted()),
            this, SLOT(saveAll()));
    connect(ui->removeIgnoredAddon, SIGNAL(clicked()),
            this, SLOT(removeSelectedIgnoredAddons()));
    connect(ui->resetVersion, SIGNAL(clicked()),
            this, SLOT(resetVersion()));


}

Settings::~Settings()
{
    delete ui;
}

QString Settings::getAddonPath(){
#ifdef Q_OS_MAC
    const QString def = QDir::homePath().append("/Library/Application Support/Hedgewars/Data/");;
#else
    #ifdef Q_OS_WIN
    QSettings reg(QSettings::UserScope, "Microsoft", "Windows");
    reg.beginGroup("CurrentVersion/Explorer/Shell Folders");

    const QString def = reg.value("Personal").toString() + "\\Hedgewars\\Data\\";
    #else
    const QString def = QDir::homePath().append("/.hedgewars/Data/");
#endif
#endif

    QSettings settings;
    return settings.value("app/addonsDirectory", def).toString();
}

QString Settings::getHedgewarsBinary(){
#ifdef Q_WS_MAC
    const QString def = "";
#else
#ifdef Q_WS_WIN
    const QString def = "C:\\Program Files\\Hedgewars 0.9.17\\bin\\hedgewars.exe";
#else
    const QString def = "/usr/bin/hedgewars";
#endif
#endif

    QSettings settings;
    return settings.value("app/hedgewarsBinary", def).toString();
}

QStringList Settings::getIgnoredAddons()
{
    QSettings s;
    return s.value("app/ignoredAddons").toStringList();
}

bool Settings::getRunGameAfterInstall()
{
    QSettings s;
    return s.value("app/runGameAfterInstall",false).toBool();
}

bool Settings::getCloseOnHwStart()
{
    QSettings s;
    return s.value("app/closeOnHwStart",true).toBool();
}

bool Settings::getIngoreUnchecked()
{
    QSettings s;
    return s.value("app/ignoreUnchecked",true).toBool();
}

QString Settings::getLocalVersion()
{
    QSettings s;
    return s.value("updates/version","0.0").toString();
}

void Settings::setIgnoredAddons(QStringList list)
{
    QSettings settings;
    if(settings.value("app/ignoredAddons").toStringList() != list){
        settings.setValue("app/ignoredAddons", list);
    }
}

void Settings::setVersion(QString version)
{
    QSettings settings;
    settings.setValue("updates/version",version);
}

void Settings::saveAll()
{
    QSettings settings;
    settings.setValue("app/addonsDirectory", ui->addonsPath->text());
    settings.setValue("app/hedgewarsBinary", ui->gamePath->text());
    settings.setValue("app/runGameAfterInstall", ui->runGameAfterInst->isChecked());
    settings.setValue("app/closeOnHwStart", ui->closeAfterHwRun->isChecked());

    QStringList list;
    for(int i=0; i<ui->ignoredAddons->count(); ++i){
        list.append(ui->ignoredAddons->item(i)->text());
    }


    if(ui->ignoreUnchecked->isChecked()){
        setIgnoredAddons(list);
    }else{
        setIgnoredAddons(QStringList());
        resetVersion();
    }

    settings.setValue("app/ignoreUnchecked", ui->ignoreUnchecked->isChecked());
}

void Settings::removeSelectedIgnoredAddons()
{
    QList<QListWidgetItem *> selected = ui->ignoredAddons->selectedItems();
    foreach(QListWidgetItem *i, selected){
        delete i;
    }
    QSettings settings;
    settings.setValue("updates/version", "0.0"); //clearing version to get fresh full list of addons (user must have possibility to download no longer ingnored)
}

void Settings::resetVersion()
{
    QSettings s;
    s.setValue("updates/version", "0.0");
    ui->versionInfo->setText(tr("Local version: 0.0"));
}
