#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDir>
#include <QTime>
#include <QFileInfo>
#include <QMessageBox>
#include <QFile>
#include <QStringList>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qsrand(QTime::currentTime().msec());
    pSet = new QSettings("Jo2003.com", "RamCD", this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

int MainWindow::myRand(int min, int max)
{
    return qrand() % ((max + 1) - min) + min;
}

void MainWindow::cleanTags()
{
    for (int i = 0; i < tagFiles.count(); i++)
    {
        if (tagFiles.at(i) != NULL)
        {
            delete tagFiles.at(i);
        }
    }

    tagFiles.clear();
}

QString MainWindow::cueSheet(const TagLib::MPEG::File *pFile, int numb)
{
    QString title = QString::fromUtf8(pFile->properties()["TITLE"].toString().toCString(true));
    QString artist = QString::fromUtf8(pFile->properties()["ARTIST"].toString().toCString(true));

    title  = title.replace("\"", "\\\"").trimmed();
    artist = artist.replace("\"", "\\\"").trimmed();

    QString fileName = QString::fromUtf8(pFile->name().toString().toCString(true));

#ifdef Q_OS_WIN
    fileName.replace("/", "\\");
#endif

    QString s = QString("FILE \"%1\" MP3\n").arg(fileName);
    s += QString("    TRACK %1 AUDIO\n").arg(numb, 2, 10, QChar('0'));

    if (!title.isEmpty())
    {
        s += QString("    TITLE \"%1\"\n").arg(title);
    }

    if (!artist.isEmpty())
    {
        s += QString("    PERFORMER \"%1\"\n").arg(artist);
    }

    s += QString("    PREGAP 00:01:00\n");
    s += QString("    INDEX 01 00:00:00");

    return s;
}

QString MainWindow::diskHeader(int numb)
{
    QString s = QString("PERFORMER \"VARIOUS ARTISTS\"\n");
    s += QString("TITLE \"%1 %2\"").arg(ui->lineCollName->text()).arg(numb);
    return s;
}

void MainWindow::on_pushOpenFolder_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open MP3 Source Directory"),
                                                    pSet->value("src_dir").toString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty())
    {
        pSet->setValue("src_dir", dir);

        ui->lineSrcFolder->setText(dir);
        cleanTags();

        QDir mp3Dir(dir, "*.mp3");
        QStringList files = mp3Dir.entryList();
        TagLib::MPEG::File* ptlFile;

        foreach (QString file, files)
        {
            QString path = QString("%1/%2").arg(mp3Dir.absolutePath()).arg(file);

            ptlFile = new TagLib::MPEG::File(path.toLatin1().constData());

            if (ptlFile && ptlFile->isOpen())
            {
                tagFiles.append(ptlFile);
            }
            else if(ptlFile)
            {
                delete ptlFile;
            }
        }

        int cnt;
        int length = 0;
        int numb   = 0;
        int dnumb  = 0;

        while ((cnt = tagFiles.count()))
        {
            int curr = myRand(0, cnt - 1);
            ptlFile  = tagFiles.at(curr);

            if (!dnumb)
            {
                ui->plainTextEdit->appendPlainText(diskHeader(++dnumb));
            }

            if ((length + ptlFile->audioProperties()->length() + numb) > (80 * 60))
            {
                QTime tm;
                tm = tm.addSecs(length + numb);
                ui->plainTextEdit->appendPlainText(QString("-------------- start next disk %1 ----------------").arg(tm.toString("h:mm:ss")));
                length = ptlFile->audioProperties()->length();
                numb   = 1;
                ui->plainTextEdit->appendPlainText(diskHeader(++dnumb));
            }
            else
            {
                length += ptlFile->audioProperties()->length();
                numb++;
            }

            ui->plainTextEdit->appendPlainText(cueSheet(ptlFile, numb));

            delete ptlFile;
            tagFiles.remove(curr);
        }
    }
}

void MainWindow::on_buttonBox_accepted()
{
    QString tmpl = QFileDialog::getSaveFileName(this, tr("Save cue sheet file(s)"), pSet->value("trg_dir").toString(), "Cue Sheet File (*.cue);;Other File (*.*)");

    if (!tmpl.isEmpty())
    {
        // create file name template ...
        QFileInfo fInfo(tmpl);

        pSet->setValue("trg_dir", fInfo.absolutePath());

        tmpl = QString("%1/%2_").arg(fInfo.absolutePath()).arg(fInfo.baseName());

        QStringList content = ui->plainTextEdit->toPlainText().split("\n");

        int numb = 0;
        QFile* pCue = NULL;
        QTextStream ts;

        for (int i = 0; i < content.count(); i++)
        {
            if (!pCue)
            {
                if ((pCue = new QFile(QString("%1%2.%3").arg(tmpl).arg(++numb).arg(fInfo.suffix()))))
                {
                    if (!pCue->open(QIODevice::Text | QIODevice::Truncate | QIODevice::WriteOnly))
                    {
                        delete pCue;
                        pCue = NULL;
                        break;
                    }
                    else
                    {
                        ts.setDevice(pCue);
                        ts.setCodec("UTF-8");
                        ts.setGenerateByteOrderMark(true);
                    }
                }
            }

            if (content.at(i).contains("--------------"))
            {
                pCue->close();
                delete pCue;
                pCue = NULL;
            }
            else
            {
                ts << content.at(i) << endl; //  pCue->write(QString("%1\n").arg(content.at(i)).toUtf8());
            }
        }

        if (pCue != NULL)
        {
            pCue->close();
            delete pCue;
        }
    }

    ui->plainTextEdit->clear();
}

void MainWindow::on_buttonBox_rejected()
{
    close();
}
