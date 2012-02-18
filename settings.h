#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QSettings>
#include <QDir>

namespace Ui {
class Settings;
}

class Settings : public QDialog
{
    Q_OBJECT
    
public:
    explicit Settings(QWidget *parent = 0);
    ~Settings();
    static QString getAddonPath();
    static QString getHedgewarsBinary();
    static QStringList getIgnoredAddons();
    static bool getRunGameAfterInstall();
    static bool getCloseOnHwStart();
    static bool getIngoreUnchecked();
    static QString getLocalVersion();
    static void setIgnoredAddons(QStringList list);
    static void setVersion(QString version);

public slots:
    void saveAll();
    void removeSelectedIgnoredAddons();
    void resetVersion();

signals:

private:
    Ui::Settings *ui;
};

#endif // SETTINGS_H
