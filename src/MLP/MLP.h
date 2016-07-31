#ifndef MLP_H
#define MLP_H

#include <QList>
#include <QPair>
#include <QString>
#include <QStringList>

#include <opencv2/core.hpp>
#include <opencv2/ml.hpp>

class MLP
{
public:
    MLP();
    ~MLP() { }

    void loadXML( const QString &xml  );
    void saveXML( const QString &xml  );
    void loadCSV( const QString &csv );
    void train();
    QList<QPair<QString, bool> > predictImages( const QStringList &images );
    bool predictFeatureMat( const cv::Mat &featureMat );

    static QStringList loadImagePaths( const QString &path );

private:
    cv::Ptr<cv::ml::ANN_MLP> mlp;
    QStringList pos;
    QStringList neg;
    QStringList trained;
    QList<QPair<QString, bool> > predicted;
};

#endif // MLP_H
