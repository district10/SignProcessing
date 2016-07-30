#include "SignLogger.h"
#include "Utils.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>

int Log::count = 0;
QMutex SignLogger::mutex;
QList<Log> SignLogger::logs;

QList<Log> SignLogger::refresh( )
{
    QList<Log> copy;

    QMutexLocker locker( &mutex );
    foreach ( const Log &log, logs ) {
        copy.append( log );
    }
    // clear();

    return copy;
}

void SignLogger::save( const QString &path )
{
    QList<Log> logs = refresh();
    if ( logs.isEmpty() ) {
        return;
    }

    FILE *fp = fopen( qPrintable(path), "a+" );
    if ( NULL == fp ) { return; }

    foreach ( const Log &log, logs ) {
        fprintf( fp, "%s\n", qPrintable( log.save() ) );
    }

    fclose( fp );
}

void SignLogger::load( const QString &path )
{
    std::ifstream logfile( qPrintable(path) );
    std::string line;
    while ( std::getline( logfile, line ) ) {
        Log log;
        if ( log.load( QString(line.c_str()) ) ) {
            logging( log );
        }
    }
}
