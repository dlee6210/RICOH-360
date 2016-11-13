#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtNetwork>
#include <QUrl>
#include <QByteArray>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_mode = SESSION;
    m_request.setUrl(QUrl("http://192.168.1.1/osc/commands/execute"));

    m_manager = new QNetworkAccessManager(this);

    connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    ui->listView->setModel(&m_model);

    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(slotAbout()));
    connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadPictureList()
{
    m_mode = LISTPICTURES;
    int entries = ui->spinBoxNumEntries->value();
    QString data("{ \"name\": \"camera._listAll\", \"parameters\": {  \"entryCount\": " + QString::number(entries) + "  } }");
    QByteArray postData(data.toStdString().c_str());
    m_manager->post(m_request, postData);
}

void MainWindow::on_pushButtonStartSession_clicked()
{
    m_mode = SESSION;
    QByteArray postData("{ \"name\": \"camera.startSession\", \"parameters\": {} }");

    m_manager->post(m_request, postData);
}

void MainWindow::replyFinished (QNetworkReply *reply)
{
    if(reply->error()) {
        qDebug() << "ERROR!";
        qDebug() << reply->errorString();
    } else {

        QApplication::setOverrideCursor(Qt::WaitCursor);
        QByteArray data = reply->readAll();
        switch (m_mode) {
        case SESSION:
        {
            QString strReply(data);
            QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());
            QJsonObject jsonObject = jsonResponse.object();
            QString sID = jsonObject["results"].toObject()["sessionId"].toString();

            ui->lineEditID->setText(sID);

            qDebug() << QString(data);
            break;
        }
        case TAKEPICTURE:
        {
            qDebug() << QString(data);
            break;
        }
        case LISTPICTURES:
        {
            //json list all in enty
            QStringList propertyNames;
            QStringList propertyKeys;

            QString strReply(data);
            QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());
            QJsonObject jsonObject = jsonResponse.object();
            QJsonArray jsonArray = jsonObject["results"].toObject()["entries"].toArray();

            foreach (const QJsonValue & value, jsonArray) {
                QJsonObject obj = value.toObject();
                propertyNames.append(obj["name"].toString());
                propertyKeys.append(obj["uri"].toString());
            }
            m_model.setStringList(propertyNames);
            break;
        }
        case GRABPICTURE:
        {
            QImage image =  QImage::fromData(data, "JPEG" );
            ui->labelImage->setPixmap(QPixmap::fromImage(image).scaled(1024, 512, Qt::KeepAspectRatio));
            break;
        }
        default:
        {
            qDebug() << QString(data);
            break;
        }
        }

        QApplication::restoreOverrideCursor();
    }

    reply->deleteLater();
}

void MainWindow::on_pushButtonCapture_clicked()
{
    m_mode = TAKEPICTURE;
    QString sId = ui->lineEditID->text();

    QString data(" { \"name\":\"camera.takePicture\", "
                 " \"parameters\": { "
                 " \"sessionId\": \"" + sId +  "\""
                 "  } "
                 "}");

    QByteArray postData(data.toStdString().c_str());

    m_manager->post(m_request, postData);

    loadPictureList();

}

void MainWindow::on_listView_doubleClicked(const QModelIndex &index)
{
    m_mode = GRABPICTURE;
    QString name = index.data().toString();

    QString data("{ \"name\":\"camera.getImage\", "
                 "\"parameters\": { "
                 "\"fileUri\": \"100RICOH/" + name +  "\","
                 "\"_type\": \"image\""
                 "} "
                 "}");


    QByteArray postData(data.toStdString().c_str());
    m_manager->post(m_request, postData);

    ui->groupBoxResult->setTitle(name);
}

void MainWindow::on_pushButtonRefresh_clicked()
{
    loadPictureList();
}

void MainWindow::slotAbout()
{
    QMessageBox::information(this, tr("About"), tr("This is an open source application for testing RECOH 360 Theta camera developed "
                                                   "by Celal Savur \n"
                                                   "email: c.savur@gmail.com"));
}
