#include "AppLogger.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMessageLogContext>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>
#include <QThread>
#include <QtGlobal>
#include <cstdio>

namespace {

QMutex& LogMutex()
{
    static QMutex mutex;
    return mutex;
}

QtMessageHandler& PreviousMessageHandler()
{
    static QtMessageHandler handler = nullptr;
    return handler;
}

QString MessageTypeName(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return QStringLiteral("DEBUG");
    case QtInfoMsg:
        return QStringLiteral("INFO");
    case QtWarningMsg:
        return QStringLiteral("WARN");
    case QtCriticalMsg:
        return QStringLiteral("CRITICAL");
    case QtFatalMsg:
        return QStringLiteral("FATAL");
    }

    return QStringLiteral("UNKNOWN");
}

QString CurrentThreadId()
{
    return QString::number(reinterpret_cast<quintptr>(QThread::currentThreadId()), 16);
}

bool EnsureLogDirectory()
{
    QDir dir(AppLogger::logDirectory());
    if (dir.exists()) {
        return true;
    }

    return dir.mkpath(QStringLiteral("."));
}

void AppendLogLine(const QString& filePath, const QString& line)
{
    QMutexLocker locker(&LogMutex());

    if (!EnsureLogDirectory()) {
        QTextStream stream(stderr);
        stream << "Cannot create log directory: " << AppLogger::logDirectory() << Qt::endl;
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(stderr);
        stream << "Cannot open log file: " << filePath << Qt::endl;
        return;
    }

    QTextStream stream(&file);
    stream << line << Qt::endl;
}

QString FormatMessage(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    QString location;
    if (context.file && context.line > 0) {
        location = QStringLiteral(" %1:%2").arg(QString::fromLocal8Bit(context.file)).arg(context.line);
    }

    QString category;
    if (context.category && QString::fromLocal8Bit(context.category) != QStringLiteral("default")) {
        category = QStringLiteral(" [%1]").arg(QString::fromLocal8Bit(context.category));
    }

    return QStringLiteral("%1 [%2] [tid:%3]%4%5 %6")
        .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz")))
        .arg(MessageTypeName(type))
        .arg(CurrentThreadId())
        .arg(category)
        .arg(location)
        .arg(message);
}

void AppMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    AppendLogLine(AppLogger::appLogPath(), FormatMessage(type, context, message));

    if (PreviousMessageHandler()) {
        PreviousMessageHandler()(type, context, message);
    } else {
        const QByteArray output = FormatMessage(type, context, message).toLocal8Bit();
        std::fprintf(stderr, "%s\n", output.constData());
    }

    if (type == QtFatalMsg) {
        abort();
    }
}

}

void AppLogger::install()
{
    if (PreviousMessageHandler()) {
        return;
    }

    EnsureLogDirectory();
    PreviousMessageHandler() = qInstallMessageHandler(AppMessageHandler);
    AppendLogLine(appLogPath(), QStringLiteral("%1 [INFO] logging started")
                                .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"))));
}

QString AppLogger::logDirectory()
{
    const QString basePath = QCoreApplication::instance()
        ? QCoreApplication::applicationDirPath()
        : QDir::currentPath();

    return QDir(basePath).filePath(QStringLiteral("logs"));
}

QString AppLogger::appLogPath()
{
    return QDir(logDirectory()).filePath(QStringLiteral("app.log"));
}

QString AppLogger::roiDebugReportPath()
{
    return QDir(logDirectory()).filePath(QStringLiteral("roi_debug_report.log"));
}

void AppLogger::appendRoiReport(const QString& message)
{
    const QString line = QStringLiteral("%1 %2")
        .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz")))
        .arg(message);

    AppendLogLine(roiDebugReportPath(), line);
}
