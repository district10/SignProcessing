#ifndef UTILS_H
#define UTILS_H

#include <QtGui>
#include <QStringList>
#include <QChar>

class Utils
{
public:
	 Utils( ) { }
     ~Utils( ) { }
	 
	static QString strippedName( const QString &fullFileName, QString &fileDir );
	static bool isPicture( const QString &fileName );
	static QStringList openMultipleFiles( const QString &filter, QWidget *widget );
	static bool deleteFolderContent( const QString &dirPath, bool deleteFolder = false );
	static bool makeFolder( const QString &folderName );
	static QDir transferDirectory( const QString &folderName );
	static QString joinPath( const QString &shortPath, const QString &suffix );
    static QStringList basename( const QStringList &paths );
    static QString basename( const QString &path );
    static QString basenameWithoutExt( const QString &path );
    static QString dirname( const QString &path );
    static void sleep( const quint16 &t );


private:
    static const QChar sep;
};

#endif  // UTILS_H