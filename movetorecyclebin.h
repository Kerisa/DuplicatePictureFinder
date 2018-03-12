#ifndef MOVETORECYCLEBIN_H
#define MOVETORECYCLEBIN_H

#include <QString>

class MoveToRecycleBin
{
public:
    static bool execute(const QString & filename);

#ifdef Q_OS_LINUX
    static bool    TrashInitialized;
    static QString TrashPath;
    static QString TrashPathInfo;
    static QString TrashPathFiles;
#endif
};

#endif // MOVETORECYCLEBIN_H
