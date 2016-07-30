#ifndef SIGNTRANSFORMER_H
#define SIGNTRANSFORMER_H

#include <QDialog>
#include <QStringList>
#include <QStringListModel>
#include <QGraphicsScene>
#include <QImage>
#include <QList>

#include <cv.h>
#include <highgui.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace Ui {
class SignTransformer;
}

class SignTransformer : public QDialog
{
    Q_OBJECT

public:
    static QString _demoPath;

    explicit SignTransformer(QWidget *parent = 0);
    ~SignTransformer();
    QStringList processSign( const QString &path );
    void showImage( const QString &path );

    static QStringList genPositives( const int &idx,
                                     const QString &path, 
                                     double x, double y, 
                                     double dx, double dy, 
                                     const QString &outdir, 
                                     const QStringList &outs,
                                     const QList<int> &angles,
                                     const QList<int> &shiftsX,
                                     const QList<int> &shiftsY );

    static QStringList genNegatives( const int &idx,
                                     const QString &path, 
                                     double x, double y, 
                                     double dx, double dy, 
                                     const QString &outdir, 
                                     const QStringList &outs );
private slots:
    void on_pushButton_open_signs_clicked();
    void on_pushButton_tx_signs_clicked();
    void on_listView_ins_clicked(const QModelIndex &index);
    void on_listView_outs_clicked(const QModelIndex &index);
    void on_pushButton_out_dir_clicked();

private:
    static const QString filter;
    Ui::SignTransformer *ui;

    QStringList signs;
    QStringList signXs;
    QString outdir;

    QStringListModel signsModel;
    QStringListModel signXsModel;

    QImage image;
    QPixmap pixmap;
    QGraphicsScene scene;
    cv::Mat src;
    cv::Mat dst;
};

struct Twin {
    cv::Mat m[2];
    int idx;
    QString log;
    QString inPath;
    QString outDir;
    bool valid;

    int src() { return idx % 2; }
    int dst() { return (1+idx) % 2; }
    void oxo() { ++idx; }

    Twin( const QString &path, const QString &dir );
    QString save( QString suffix = "" );
    void flipLeftRight();
    void flipUpdown();
    void rotate( int angle = 0 );
    void translate( double wp, double hp );
};

struct MatType {
    cv::Mat *m;
    cv::RotatedRect *rr;
    double imgX;
    double imgY;
    double imgW;
    double imgH;
};

#endif // SIGNTRANSFORMER_H
