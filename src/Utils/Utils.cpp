#include "Utils.h"
#include "Config.h"

const QChar Utils::sep = QChar('/');

QString Utils::strippedName( const QString &fullFileName, QString &fileDir )
{
	fileDir = QFileInfo( fullFileName ).absolutePath();
	return QFileInfo( fullFileName ).fileName();
}

bool Utils::isPicture( const QString &fileName )
{
	if( 
        fileName.endsWith( ".jpg",  Qt::CaseInsensitive ) ||
        fileName.endsWith( ".bmp",  Qt::CaseInsensitive ) ||
		fileName.endsWith( ".png",  Qt::CaseInsensitive ) || 
        fileName.endsWith( ".gif",  Qt::CaseInsensitive ) ||
		fileName.endsWith( ".jpeg", Qt::CaseInsensitive ) || 
        fileName.endsWith( ".pgm",  Qt::CaseInsensitive ) ||
		fileName.endsWith( ".ppm",  Qt::CaseInsensitive ) || 
        fileName.endsWith( ".tif",  Qt::CaseInsensitive ) ||
		fileName.endsWith( ".tiff", Qt::CaseInsensitive ) || 
        fileName.endsWith( ".xbm",  Qt::CaseInsensitive ) ||
		fileName.endsWith( ".tiff", Qt::CaseInsensitive ) || 
        fileName.endsWith( ".xpm",  Qt::CaseInsensitive ) ||
		fileName.endsWith( ".pbm",  Qt::CaseInsensitive ) || 
        fileName.endsWith( ".xbm",  Qt::CaseInsensitive ) 
       ) {
		return true;
    } else {
		return false;	
    }
}

QStringList Utils::openMultipleFiles( const QString &filter, QWidget *widget )
{
	QStringList fileNames;

	QFileDialog::Options options;
	options |= QFileDialog:: DontUseNativeDialog;

	fileNames = QFileDialog::getOpenFileNames( widget,
                                               QObject::tr( "select multiple image files" ),
                                               DEFAULT_INPUT_IMAGE_DIR,  // entry dir
                                               filter,                   // filter
                                               (QString *)0,             // selected filter
                                               options );
	return fileNames;
}


static QFileInfoList getFilOfDir( const QString &dirPath )
{
	QFileInfoList fil;

	QDir dir = Utils::transferDirectory( dirPath );
	QString path = dir.path();

	if( !dir.exists() ) {
		return fil;
    }
	
	fil = dir.entryInfoList( QDir::Dirs
                           | QDir::Files
                           | QDir::Readable
                           | QDir::Writable
                           | QDir::Hidden
                           | QDir::NoDotAndDotDot, QDir::Name );
    return fil;
}

bool Utils::deleteFolderContent( const QString &dirPath, bool deleteFolder /* = false */ )
{
	QDir dir = transferDirectory(dirPath);
	QString path = dir.path();

	QFileInfoList fil = getFilOfDir( dirPath );

	QFileInfo curFile;
	
	if( !dir.exists() ) {
		return true;
    }
	
	while( fil.size() > 0 ) {
		int infoNum = fil.size();
		for( int i = infoNum-1; i >= 0; --i ) {
			curFile = fil[i];
            fil.removeAt( i );
			
			if ( curFile.isFile() ) {

                // remove file
				QFile fileTemp( curFile.filePath() );
				fileTemp.remove();

			} else if( curFile.isDir() ) {

                // append files in dir
                fil.append( getFilOfDir( curFile.filePath() ) );

			} else {

                qDebug() << "I have no idea what's going on...";

            }
		}
	}

	if ( deleteFolder && fil.size() == 0 ) {
		dir.rmdir( path );
    }

	return ( fil.size() == 0 );
}

bool Utils::makeFolder( const QString &folderName )
{
	QDir dir = QDir::current();
	dir.cdUp();

	if ( dir.exists(folderName) ) {
		deleteFolderContent( folderName );
	}
    
    return dir.mkdir( folderName );
}

QDir Utils::transferDirectory( const QString &folderName )
{
	QDir dir = QDir::current();
	dir.cdUp();
	return QDir( dir.path() + sep + folderName );
}

QString Utils::joinPath( const QString &dirPath, const QString &basename )
{
	QString joinedPath = dirPath;

	if ( dirPath.right(1) == sep ) {
		joinedPath.chop( 1 );
    }
	return joinedPath + sep + basename;
}

QStringList Utils::basename( const QStringList &paths )
{
    QStringList basenames;
    foreach ( const QString &path, paths ) {
        basenames << basename( path );
    }
    return basenames;
}

QString Utils::basename( const QString &path )
{
    return path.split( sep ).last();
}

QString Utils::basenameWithoutExt(const QString &path)
{
    QString bn = basename( path );
    bn.chop( 1 + bn.split('.').last().length() );
    return bn;
}

QString Utils::dirname(const QString &path)
{
    QString dir = path;
    QStringList s1 = path.split( QChar('/') );
    QStringList s2 = path.split( QChar('\\') );
    if ( s1.length() != 1 ) {
        dir.chop( s1.last().length() + 1 );
    } else if ( s2.length() != 1 ) {
        dir.chop( s2.last().length() + 1 );
    } else {
        qDebug() << "chopping failed...";
        dir = QDir::currentPath();
    }
    return dir;
}

void Utils::sleep( const quint16 &t )
{
    QTime dieTime = QTime::currentTime().addMSecs(t);
    while( QTime::currentTime() < dieTime ) {
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
    }
}
