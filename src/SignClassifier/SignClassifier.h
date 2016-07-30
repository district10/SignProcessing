#ifndef SIGNCLASSIFIER_H
#define SIGNCLASSIFIER_H

#include <QDialog>
#include <QStandardItemModel>
#include <opencv2/core.hpp>
#include <opencv2/ml.hpp>

namespace Ui {
    class SignClassifier;
}

class SignClassifier : public QDialog
{
    Q_OBJECT

public:
    explicit SignClassifier( QWidget *parent = 0 );
    ~SignClassifier();

    enum SignFlag { Sign, NotSign, Unknown };
    typedef QHash<QString, SignFlag> SignPairs;

private slots:
    void on_filepos_clicked();
    void on_fileneg_clicked();
    void on_fileunknown_clicked();

    void on_train_clicked();
    void on_save_clicked();
    void on_load_clicked();
    void on_predict_clicked();

private:
    bool isSign( const QString &file );

    Ui::SignClassifier *ui;
    QStringList signsPos;
    QStringList signsNeg;
    QStringList signsUnknown;
    QStandardItemModel *signsModelPos;
    QStandardItemModel *signsModelNeg;
    QStandardItemModel *signsModelUnknown;

    cv::Ptr<cv::ml::ANN_MLP> mlp;
    SignPairs signPairs;
};

#endif // SIGNCLASSIFIER_H
