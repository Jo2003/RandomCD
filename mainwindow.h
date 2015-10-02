#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <taglib/mpegfile.h>
#include <taglib/tpropertymap.h>
#include <QtCore/QtGlobal>
#include <QSettings>

namespace RandCD
{
    typedef QVector<TagLib::MPEG::File*> tagFiles_t;
}

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    int myRand(int min, int max);
    void cleanTags();
    QString cueSheet(const TagLib::MPEG::File* pFile, int numb);
    QString diskHeader(int numb);

private slots:
    void on_pushOpenFolder_clicked();

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::MainWindow *ui;
    RandCD::tagFiles_t tagFiles;
    QSettings* pSet;
};

#endif // MAINWINDOW_H
