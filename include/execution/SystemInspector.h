#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

class SystemInspector : public QObject
{
    Q_OBJECT

public:
    explicit SystemInspector(QObject *parent = nullptr);

    QVariantMap cpuInfo() const;
    QVariantMap memoryInfo() const;
    QVariantMap diskInfo() const;
    QVariantMap osInfo() const;
    QString fullReport() const;
};
