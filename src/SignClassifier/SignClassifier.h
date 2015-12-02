#ifndef SIGNCLASSIFIER_H
#define SIGNCLASSIFIER_H

#include <QDialog>
#include <QStandardItemModel>
#include <opencv2/core/core.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cv.h>
#include <highgui.h>

namespace Ui {
class SignClassifier;
}

/*
class SignThumbnail : public QStandardItem
{
    Q_OBJECT
public:
    SignThumbnail( QWidget *parent = 0 );
};
*/


class SignClassifier : public QDialog
{
    Q_OBJECT

public:
    explicit SignClassifier(QWidget *parent = 0);
    ~SignClassifier();
    
    enum SignFlag { Sign, NotSign, Unknown };
    enum SignSize { 
        Sign_Width = 24,
        Sign_Height = 24,
    };
    enum TransConfig {
        NORMAL,
    };

    typedef QHash<QString, SignFlag> SignPairs;

    void train( const QString &file, enum SignFlag flag );
    bool img2mat( const QString &file, cv::Mat &mat, TransConfig config = NORMAL );
    bool isSign( const QString &file );

private slots:
    void on_filepos_clicked();
    void on_fileneg_clicked();
    void on_fileunknown_clicked();
    void on_train_clicked();
    void on_predict_clicked();

private:
    Ui::SignClassifier *ui;
    QStringList signsPos;
    QStringList signsNeg;
    QStringList signsUnknown;
    QStandardItemModel *signsModelPos;
    QStandardItemModel *signsModelNeg;
    QStandardItemModel *signsModelUnknown;

    CvANN_MLP bp;
    CvANN_MLP_TrainParams params;
    SignPairs signPairs;
};

#endif // SIGNCLASSIFIER_H
