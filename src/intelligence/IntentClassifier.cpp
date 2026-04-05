#include "intelligence/IntentClassifier.h"
#include "service/Logger.h"

IntentClassifier::IntentClassifier(QObject *parent)
    : QObject(parent)
{
}

IntentClassifier::Intent IntentClassifier::classify(const QString &input) const
{
    QString lower = input.toLower().trimmed();

    // Execute commands
    if (lower.startsWith("run ") || lower.startsWith("execute ") ||
        lower.startsWith("$ ") || lower.startsWith("shell ") ||
        lower.startsWith("cmd ")) {
        return Intent::Execute;
    }

    // Project management
    if (lower.startsWith("create project") || lower.startsWith("new project") ||
        lower.startsWith("list projects") || lower.startsWith("delete project") ||
        lower.startsWith("init project") || lower.startsWith("build project")) {
        return Intent::Project;
    }

    // System diagnostics
    if (lower.startsWith("system info") || lower.startsWith("disk space") ||
        lower.startsWith("memory usage") || lower.startsWith("cpu info") ||
        lower.startsWith("show system") || lower.startsWith("diagnostics") ||
        lower.startsWith("disk usage") || lower.startsWith("os info")) {
        return Intent::Diagnostic;
    }

    // Code generation
    if (lower.startsWith("write code") || lower.startsWith("generate code") ||
        lower.startsWith("create a script") || lower.contains("write a function") ||
        lower.startsWith("code ") || lower.startsWith("implement ") ||
        lower.startsWith("write a class") || lower.startsWith("create a class") ||
        lower.startsWith("refactor ")) {
        return Intent::Code;
    }

    // Default to chat
    return Intent::Chat;
}

QString IntentClassifier::intentToString(Intent intent)
{
    switch (intent) {
        case Intent::Chat:       return "CHAT";
        case Intent::Code:       return "CODE";
        case Intent::Execute:    return "EXECUTE";
        case Intent::Project:    return "PROJECT";
        case Intent::Diagnostic: return "DIAGNOSTIC";
        case Intent::Unknown:    return "UNKNOWN";
    }
    return "UNKNOWN";
}
