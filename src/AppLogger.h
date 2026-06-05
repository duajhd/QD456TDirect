#ifndef APPLOGGER_H
#define APPLOGGER_H

#include <QString>

class AppLogger
{
public:
    static void install();

    static QString logDirectory();
    static QString appLogPath();
    static QString roiDebugReportPath();

    static void appendRoiReport(const QString& message);
};

#endif // APPLOGGER_H
