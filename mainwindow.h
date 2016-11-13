#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QStringListModel>

class QNetworkReply;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum Mode { SESSION, TAKEPICTURE, LISTPICTURES, GRABPICTURE };

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void loadPictureList();

private slots:
    void replyFinished(QNetworkReply *reply);

    void on_pushButtonStartSession_clicked();
    void on_pushButtonCapture_clicked();
    void on_listView_doubleClicked(const QModelIndex &index);

    void on_pushButtonRefresh_clicked();

    void slotAbout();

private:
    Ui::MainWindow *ui;

    Mode m_mode;
    QNetworkAccessManager *m_manager;
    QNetworkRequest m_request;

    QStringListModel m_model;
};

#endif // MAINWINDOW_H
