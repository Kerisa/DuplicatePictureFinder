#include "movetorecyclebin.h"


#ifdef _WIN32

#include <QFileInfo>
#include <windows.h>


bool MoveToRecycleBin::execute(const QString &filename)
{
    QFileInfo fileinfo(filename);
    if (!fileinfo.exists())
        return false;

    WCHAR from[MAX_PATH];
    memset(from, 0, sizeof(from));
    int l = fileinfo.absoluteFilePath().toWCharArray(from);
    Q_ASSERT( 0 <= l && l < MAX_PATH );
    from[l] = '\0';

    SHFILEOPSTRUCT fileop;
    memset(&fileop, 0, sizeof(fileop));
    fileop.wFunc = FO_DELETE;
    fileop.pFrom = from;
    fileop.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
    int rv = SHFileOperation(&fileop);
    return 0 == rv;
}

#endif



//#ifdef Q_OS_LINUX

//#include <QFile>

//bool MoveToRecycleBin::execute(const QString &filename)
//{
//    return QFile::remove(filename);
//}

//#endif


#ifdef Q_OS_LINUX

bool MoveToRecycleBin::TrashInitialized;
QString MoveToRecycleBin::TrashPath;
QString MoveToRecycleBin::TrashPathInfo;
QString MoveToRecycleBin::TrashPathFiles;

void MoveToRecycleBin::execute(const QString &file)
{
    if( !TrashInitialized ){
        QStringList paths;
        const char* xdg_data_home = getenv( "XDG_DATA_HOME" );
        if( xdg_data_home ){
            qDebug() << "XDG_DATA_HOME not yet tested";
            QString xdgTrash( xdg_data_home );
            paths.append( xdgTrash + "/Trash" );
        }
        QString home = QStandardPaths::writableLocation( QStandardPaths::HomeLocation );
        paths.append( home + "/.local/share/Trash" );
        paths.append( home + "/.trash" );
        foreach( QString path, paths ){
            if( TrashPath.isEmpty() ){
                QDir dir( path );
                if( dir.exists() ){
                    TrashPath = path;
                }
            }
        }
        if( TrashPath.isEmpty() )
            throw Exception( "Cant detect trash folder" );
        TrashPathInfo = TrashPath + "/info";
        TrashPathFiles = TrashPath + "/files";
        if( !QDir( TrashPathInfo ).exists() || !QDir( TrashPathFiles ).exists() )
            throw Exception( "Trash doesnt looks like FreeDesktop.org Trash specification" );
        TrashInitialized = true;
    }
    QFileInfo original( file );
    if( !original.exists() )
        throw Exception( "File doesnt exists, cant move to trash" );
    QString info;
    info += "[Trash Info]\nPath=";
    info += original.absoluteFilePath();
    info += "\nDeletionDate=";
    info += QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss.zzzZ");
    info += "\n";
    QString trashname = original.fileName();
    QString infopath = TrashPathInfo + "/" + trashname + ".trashinfo";
    QString filepath = TrashPathFiles + "/" + trashname;
    int nr = 1;
    while( QFileInfo( infopath ).exists() || QFileInfo( filepath ).exists() ){
        nr++;
        trashname = original.baseName() + "." + QString::number( nr );
        if( !original.completeSuffix().isEmpty() ){
            trashname += QString( "." ) + original.completeSuffix();
        }
        infopath = TrashPathInfo + "/" + trashname + ".trashinfo";
        filepath = TrashPathFiles + "/" + trashname;
    }
    QDir dir;
    if( !dir.rename( original.absoluteFilePath(), filepath ) ){
        throw Exception( "move to trash failed" );
    }
    File infofile;
    infofile.createUtf8( infopath, info );
}
#endif
