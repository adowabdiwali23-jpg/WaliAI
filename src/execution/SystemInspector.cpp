#include "execution/SystemInspector.h"

#include <QFile>
#include <QProcess>
#include <QSysInfo>
#include <QStorageInfo>

SystemInspector::SystemInspector(QObject *parent)
    : QObject(parent)
{
}

QVariantMap SystemInspector::cpuInfo() const
{
    QVariantMap info;
    info["architecture"] = QSysInfo::currentCpuArchitecture();

    QFile cpuFile("/proc/cpuinfo");
    if (cpuFile.open(QIODevice::ReadOnly)) {
        QString content = QString::fromUtf8(cpuFile.readAll());
        int cores = content.count("processor");
        info["cores"] = cores > 0 ? cores : 1;

        int modelIdx = content.indexOf("model name");
        if (modelIdx >= 0) {
            int colonIdx = content.indexOf(':', modelIdx);
            int newlineIdx = content.indexOf('\n', colonIdx);
            if (colonIdx >= 0 && newlineIdx >= 0) {
                info["model"] = content.mid(colonIdx + 1, newlineIdx - colonIdx - 1).trimmed();
            }
        }
    }
    return info;
}

QVariantMap SystemInspector::memoryInfo() const
{
    QVariantMap info;
    QFile memFile("/proc/meminfo");
    if (memFile.open(QIODevice::ReadOnly)) {
        QString content = QString::fromUtf8(memFile.readAll());
        auto extractKB = [&](const QString &key) -> qint64 {
            int idx = content.indexOf(key);
            if (idx < 0) return 0;
            int colonIdx = content.indexOf(':', idx);
            int newlineIdx = content.indexOf('\n', colonIdx);
            QString val = content.mid(colonIdx + 1, newlineIdx - colonIdx - 1).trimmed();
            return val.split(' ').first().toLongLong();
        };
        qint64 totalKB = extractKB("MemTotal");
        qint64 availKB = extractKB("MemAvailable");
        info["total_mb"] = totalKB / 1024;
        info["available_mb"] = availKB / 1024;
        info["used_mb"] = (totalKB - availKB) / 1024;
    }
    return info;
}

QVariantMap SystemInspector::diskInfo() const
{
    QVariantMap info;
    QStorageInfo storage = QStorageInfo::root();
    info["total_gb"] = storage.bytesTotal() / (1024 * 1024 * 1024);
    info["free_gb"] = storage.bytesFree() / (1024 * 1024 * 1024);
    info["filesystem"] = QString::fromUtf8(storage.fileSystemType());
    return info;
}

QVariantMap SystemInspector::osInfo() const
{
    QVariantMap info;
    info["kernel"] = QSysInfo::kernelType();
    info["kernel_version"] = QSysInfo::kernelVersion();
    info["os"] = QSysInfo::prettyProductName();
    info["hostname"] = QSysInfo::machineHostName();
    return info;
}

QString SystemInspector::fullReport() const
{
    auto cpu = cpuInfo();
    auto mem = memoryInfo();
    auto disk = diskInfo();
    auto os = osInfo();

    return QString(
        "=== System Diagnostics ===\n"
        "OS: %1\nKernel: %2 %3\nHostname: %4\n"
        "CPU: %5 (%6 cores)\nArchitecture: %7\n"
        "Memory: %8 MB used / %9 MB total (%10 MB available)\n"
        "Disk: %11 GB free / %12 GB total (%13)\n"
    ).arg(os["os"].toString(),
          os["kernel"].toString(),
          os["kernel_version"].toString(),
          os["hostname"].toString(),
          cpu.value("model", "Unknown").toString(),
          QString::number(cpu.value("cores", 1).toInt()),
          cpu["architecture"].toString(),
          QString::number(mem.value("used_mb", 0).toLongLong()),
          QString::number(mem.value("total_mb", 0).toLongLong()),
          QString::number(mem.value("available_mb", 0).toLongLong()),
          QString::number(disk.value("free_gb", 0).toLongLong()),
          QString::number(disk.value("total_gb", 0).toLongLong()),
          disk.value("filesystem", "unknown").toString());
}
