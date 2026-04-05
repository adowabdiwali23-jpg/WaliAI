#pragma once

#include <QObject>
#include <QString>

class IntentClassifier : public QObject
{
    Q_OBJECT

public:
    enum class Intent {
        Chat,
        Code,
        Execute,
        Project,
        Diagnostic,
        Unknown
    };
    Q_ENUM(Intent)

    explicit IntentClassifier(QObject *parent = nullptr);

    Intent classify(const QString &input) const;
    static QString intentToString(Intent intent);

signals:
    void intentClassified(Intent intent);
};
