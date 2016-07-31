#ifndef SIGNPROCESSOR_H
#define SIGNPROCESSOR_H

#include <QWidget>

#include "MLP.h"

class QStandardItemModel;

namespace Ui {
class SignProcessor;
}

class SignProcessor : public QWidget
{
    Q_OBJECT

public:
    explicit SignProcessor( QWidget *parent = 0 );
    ~SignProcessor();

    void loadImage( QString filePath );

signals:
    void init_model();

private slots:
    void initModel();
    void on_pushButton_loadimg_clicked( );
    void on_pushButton_loadxml_clicked( );

    void on_pushButton_selectdir_clicked( );
    void on_pushButton_selectimgs_clicked( );

private:
    Ui::SignProcessor *ui;
    MLP mlp;
    QStandardItemModel *unknowns;
    QStringList unknownsImgPath;
};

#endif // SIGNPROCESSOR_H