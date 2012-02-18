#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "quazip/JlCompress.h"
#include "settings.h"
#include "bammodel.h"

#include <QMainWindow>
#include <QUrl>
#include <QMessageBox>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    void defaultSettings();
    ~MainWindow();
    
public slots:
    void resizeView();
    void setNewVersionInfo(const bool empty, const QString oldVersion, const QString newVersion);
    void setTotalFilesSize(const int fileSize);
    void openSettings();
    void runGame();
    void downloadList();
    void hwRunError(QProcess::ProcessError error);
private:
    Ui::MainWindow *ui;
    BamModel *_model;
};

#endif // MAINWINDOW_H
