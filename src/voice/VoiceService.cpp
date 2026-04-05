#include "voice/VoiceService.h"
#include "infrastructure/ModelLocator.h"
#include "service/Logger.h"

#include <QCoreApplication>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

VoiceService::VoiceService(ModelLocator &locator, QObject *parent)
    : QObject(parent), m_locator(locator)
{
    // Load the default voice profile (Breeze)
    setVoiceProfile("breeze");
    // Silence timer fires when the user stops speaking
    m_silenceTimer.setSingleShot(true);
    m_silenceTimer.setInterval(kSilenceTimeoutMs);
    connect(&m_silenceTimer, &QTimer::timeout, this, [this]() {
        if (m_listening) {
            stopListening();
            // Auto-transcribe the captured audio
            if (!m_recordBuffer.isEmpty() && m_sttReady) {
                transcribeAsync(m_recordBuffer);
            }
        }
    });
}

VoiceService::~VoiceService()
{
    if (m_whisperProc) {
        m_whisperProc->kill();
        m_whisperProc->waitForFinished(3000);
        delete m_whisperProc;
    }
    if (m_piperProc) {
        m_piperProc->kill();
        m_piperProc->waitForFinished(3000);
        delete m_piperProc;
    }
}

// ---------------------------------------------------------------------------
// Binary path resolution
// ---------------------------------------------------------------------------

QString VoiceService::whisperBinaryPath() const
{
    // Check WALIAI_WHISPER_CLI env override first
    QString envPath = qEnvironmentVariable("WALIAI_WHISPER_CLI");
    if (!envPath.isEmpty() && QFileInfo::exists(envPath)) {
        return envPath;
    }

    // Check next to our own binary
    QString appDir = QCoreApplication::applicationDirPath();
    QString candidate = appDir + "/whisper-cli";
    if (QFileInfo::exists(candidate)) return candidate;

    candidate = appDir + "/main";  // older whisper.cpp naming
    if (QFileInfo::exists(candidate)) return candidate;

    // Check PATH
    QString inPath = QStandardPaths::findExecutable("whisper-cli");
    if (!inPath.isEmpty()) return inPath;

    inPath = QStandardPaths::findExecutable("whisper");
    if (!inPath.isEmpty()) return inPath;

    return {};
}

QString VoiceService::piperBinaryPath() const
{
    // Check WALIAI_PIPER_CLI env override first
    QString envPath = qEnvironmentVariable("WALIAI_PIPER_CLI");
    if (!envPath.isEmpty() && QFileInfo::exists(envPath)) {
        return envPath;
    }

    // Check next to our own binary
    QString appDir = QCoreApplication::applicationDirPath();
    QString candidate = appDir + "/piper";
    if (QFileInfo::exists(candidate)) return candidate;

    // Check PATH
    QString inPath = QStandardPaths::findExecutable("piper");
    if (!inPath.isEmpty()) return inPath;

    return {};
}

// ---------------------------------------------------------------------------
// STT initialisation
// ---------------------------------------------------------------------------

bool VoiceService::initSTT()
{
    m_whisperModel = m_locator.whisperModelPath();
    if (m_whisperModel.isEmpty() || !m_locator.whisperModelAvailable()) {
        Logger::instance().warning("No Whisper model found. STT will be disabled.");
        m_sttReady = false;
        return false;
    }

    QString binary = whisperBinaryPath();
    if (binary.isEmpty()) {
        Logger::instance().warning(
            "whisper-cli binary not found. Place it next to the WaliAI "
            "binary or set WALIAI_WHISPER_CLI. STT disabled.");
        m_sttReady = false;
        return false;
    }

    m_sttReady = true;
    Logger::instance().log("STT initialised – model: " + m_whisperModel
                           + "  binary: " + binary);
    return true;
}

// ---------------------------------------------------------------------------
// TTS initialisation
// ---------------------------------------------------------------------------

bool VoiceService::initTTS()
{
    m_piperModel = m_locator.piperModelPath();
    if (m_piperModel.isEmpty() || !m_locator.piperModelAvailable()) {
        Logger::instance().warning("No Piper voice model found. TTS will be disabled.");
        m_ttsReady = false;
        return false;
    }

    m_piperConfig = m_locator.piperConfigPath();

    QString binary = piperBinaryPath();
    if (binary.isEmpty()) {
        Logger::instance().warning(
            "piper binary not found. Place it next to the WaliAI binary "
            "or set WALIAI_PIPER_CLI. TTS disabled.");
        m_ttsReady = false;
        return false;
    }

    m_ttsReady = true;
    Logger::instance().log("TTS initialised – model: " + m_piperModel
                           + "  binary: " + binary
                           + "  profile: " + m_activeProfileName);
    return true;
}

bool VoiceService::isSTTReady() const { return m_sttReady; }
bool VoiceService::isTTSReady() const { return m_ttsReady; }

// ---------------------------------------------------------------------------
// WAV file helpers
// ---------------------------------------------------------------------------

void VoiceService::writeWavHeader(QIODevice *device, qint32 dataSize,
                                  int sampleRate, int channels, int bitsPerSample)
{
    QDataStream out(device);
    out.setByteOrder(QDataStream::LittleEndian);

    // RIFF header
    out.writeRawData("RIFF", 4);
    out << qint32(36 + dataSize);           // file size - 8
    out.writeRawData("WAVE", 4);

    // fmt sub-chunk
    out.writeRawData("fmt ", 4);
    out << qint32(16);                      // sub-chunk size (PCM)
    out << qint16(1);                       // audio format: PCM
    out << qint16(channels);
    out << qint32(sampleRate);
    out << qint32(sampleRate * channels * bitsPerSample / 8); // byte rate
    out << qint16(channels * bitsPerSample / 8);              // block align
    out << qint16(bitsPerSample);

    // data sub-chunk
    out.writeRawData("data", 4);
    out << qint32(dataSize);
}

QString VoiceService::writePcmToWav(const QByteArray &pcmData)
{
    if (!m_tempDir.isValid()) {
        Logger::instance().warning("VoiceService temp directory is invalid.");
        return {};
    }

    QString wavPath = m_tempDir.filePath("input_" +
        QString::number(QDateTime::currentMSecsSinceEpoch()) + ".wav");

    QFile file(wavPath);
    if (!file.open(QIODevice::WriteOnly)) {
        Logger::instance().warning("Failed to create temp WAV: " + wavPath);
        return {};
    }

    writeWavHeader(&file, pcmData.size(), kSampleRate, kChannels, kBitsPerSample);
    file.write(pcmData);
    file.close();

    return wavPath;
}

// ---------------------------------------------------------------------------
// Speech-to-Text
// ---------------------------------------------------------------------------

QString VoiceService::transcribe(const QByteArray &pcmData)
{
    if (!m_sttReady) {
        emit sttError("STT not initialised. Call initSTT() first.");
        return {};
    }

    // Write PCM data to a temporary WAV file
    QString wavPath = writePcmToWav(pcmData);
    if (wavPath.isEmpty()) {
        emit sttError("Failed to write audio to temporary file.");
        return {};
    }

    return transcribeFile(wavPath);
}

QString VoiceService::transcribeFile(const QString &wavPath)
{
    if (!m_sttReady) {
        emit sttError("STT not initialised.");
        return {};
    }

    QString binary = whisperBinaryPath();
    if (binary.isEmpty()) {
        emit sttError("whisper-cli binary not found.");
        return {};
    }

    // Build command: whisper-cli -m <model> -f <wav> --no-timestamps -nt
    QStringList args;
    args << "-m" << m_whisperModel;
    args << "-f" << wavPath;
    args << "-l" << m_whisperLang;
    args << "--no-timestamps";
    args << "-nt";  // no token timestamps

    Logger::instance().log("Whisper STT: " + binary + " " + args.join(" "));

    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.start(binary, args);

    if (!proc.waitForStarted(5000)) {
        QString err = "Failed to start whisper process: " + proc.errorString();
        Logger::instance().warning(err);
        emit sttError(err);
        return {};
    }

    // Wait up to 60 seconds for transcription
    if (!proc.waitForFinished(60000)) {
        proc.kill();
        QString err = "Whisper transcription timed out.";
        Logger::instance().warning(err);
        emit sttError(err);
        return {};
    }

    if (proc.exitCode() != 0) {
        QString err = "Whisper exited with code " + QString::number(proc.exitCode())
                      + ": " + QString::fromUtf8(proc.readAll());
        Logger::instance().warning(err);
        emit sttError(err);
        return {};
    }

    QString result = QString::fromUtf8(proc.readAll()).trimmed();
    Logger::instance().log("Whisper transcription: " + result.left(200));
    return result;
}

void VoiceService::transcribeAsync(const QByteArray &pcmData)
{
    if (!m_sttReady) {
        emit sttError("STT not initialised.");
        return;
    }

    // Write PCM to WAV
    QString wavPath = writePcmToWav(pcmData);
    if (wavPath.isEmpty()) {
        emit sttError("Failed to write audio to temp file.");
        return;
    }

    QString binary = whisperBinaryPath();
    if (binary.isEmpty()) {
        emit sttError("whisper-cli binary not found.");
        return;
    }

    // Clean up any previous async process
    if (m_whisperProc) {
        m_whisperProc->kill();
        m_whisperProc->waitForFinished(1000);
        delete m_whisperProc;
    }

    m_whisperProc = new QProcess(this);
    connect(m_whisperProc,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &VoiceService::onWhisperFinished);

    QStringList args;
    args << "-m" << m_whisperModel;
    args << "-f" << wavPath;
    args << "-l" << m_whisperLang;
    args << "--no-timestamps";
    args << "-nt";

    Logger::instance().log("Whisper async STT: " + binary + " " + args.join(" "));
    m_whisperProc->start(binary, args);
}

void VoiceService::onWhisperFinished(int exitCode, QProcess::ExitStatus status)
{
    if (!m_whisperProc) return;

    if (status != QProcess::NormalExit || exitCode != 0) {
        QString err = "Whisper process failed (exit=" + QString::number(exitCode)
                      + "): " + QString::fromUtf8(m_whisperProc->readAllStandardError());
        Logger::instance().warning(err);
        emit sttError(err);
    } else {
        QString text = QString::fromUtf8(m_whisperProc->readAllStandardOutput()).trimmed();
        Logger::instance().log("Whisper async result: " + text.left(200));
        emit transcriptionReady(text);
    }

    m_whisperProc->deleteLater();
    m_whisperProc = nullptr;
}

// ---------------------------------------------------------------------------
// Text-to-Speech
// ---------------------------------------------------------------------------

QByteArray VoiceService::synthesize(const QString &text)
{
    QString filePath = synthesizeToFile(text);
    if (filePath.isEmpty()) return {};

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit ttsError("Failed to read synthesised audio file.");
        return {};
    }

    QByteArray data = file.readAll();
    file.close();
    return data;
}

QString VoiceService::synthesizeToFile(const QString &text)
{
    if (!m_ttsReady) {
        emit ttsError("TTS not initialised. Call initTTS() first.");
        return {};
    }

    QString binary = piperBinaryPath();
    if (binary.isEmpty()) {
        emit ttsError("piper binary not found.");
        return {};
    }

    if (!m_tempDir.isValid()) {
        emit ttsError("Temp directory invalid.");
        return {};
    }

    QString outPath = m_tempDir.filePath("tts_" +
        QString::number(QDateTime::currentMSecsSinceEpoch()) + ".wav");

    // piper --model <model> --config <config> --output_file <out> + profile args
    QStringList args;
    args << "--model" << m_piperModel;
    if (!m_piperConfig.isEmpty()) {
        args << "--config" << m_piperConfig;
    }
    args << "--output_file" << outPath;
    appendProfileArgs(args);

    Logger::instance().log("Piper TTS [" + m_activeProfileName + "]: "
                           + binary + " " + args.join(" "));

    QProcess proc;
    proc.start(binary, args);

    if (!proc.waitForStarted(5000)) {
        QString err = "Failed to start piper process: " + proc.errorString();
        Logger::instance().warning(err);
        emit ttsError(err);
        return {};
    }

    // Write text to stdin, then close to signal EOF
    proc.write(text.toUtf8());
    proc.closeWriteChannel();

    if (!proc.waitForFinished(30000)) {
        proc.kill();
        QString err = "Piper synthesis timed out.";
        Logger::instance().warning(err);
        emit ttsError(err);
        return {};
    }

    if (proc.exitCode() != 0) {
        QString err = "Piper exited with code " + QString::number(proc.exitCode())
                      + ": " + QString::fromUtf8(proc.readAllStandardError());
        Logger::instance().warning(err);
        emit ttsError(err);
        return {};
    }

    if (!QFileInfo::exists(outPath)) {
        emit ttsError("Piper did not produce output file.");
        return {};
    }

    Logger::instance().log("Piper TTS output: " + outPath);
    return outPath;
}

void VoiceService::synthesizeAsync(const QString &text)
{
    if (!m_ttsReady) {
        emit ttsError("TTS not initialised.");
        return;
    }

    QString binary = piperBinaryPath();
    if (binary.isEmpty()) {
        emit ttsError("piper binary not found.");
        return;
    }

    // Clean up previous process
    if (m_piperProc) {
        m_piperProc->kill();
        m_piperProc->waitForFinished(1000);
        delete m_piperProc;
    }

    m_pendingSynthFile = m_tempDir.filePath("tts_" +
        QString::number(QDateTime::currentMSecsSinceEpoch()) + ".wav");

    m_piperProc = new QProcess(this);
    connect(m_piperProc,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &VoiceService::onPiperFinished);

    QStringList args;
    args << "--model" << m_piperModel;
    if (!m_piperConfig.isEmpty()) {
        args << "--config" << m_piperConfig;
    }
    args << "--output_file" << m_pendingSynthFile;
    appendProfileArgs(args);

    Logger::instance().log("Piper async TTS [" + m_activeProfileName + "]: "
                           + binary + " " + args.join(" "));
    m_piperProc->start(binary, args);

    if (m_piperProc->waitForStarted(5000)) {
        m_piperProc->write(text.toUtf8());
        m_piperProc->closeWriteChannel();
    } else {
        emit ttsError("Failed to start piper process.");
        delete m_piperProc;
        m_piperProc = nullptr;
    }
}

void VoiceService::onPiperFinished(int exitCode, QProcess::ExitStatus status)
{
    if (!m_piperProc) return;

    if (status != QProcess::NormalExit || exitCode != 0) {
        QString err = "Piper process failed (exit=" + QString::number(exitCode)
                      + "): " + QString::fromUtf8(m_piperProc->readAllStandardError());
        Logger::instance().warning(err);
        emit ttsError(err);
    } else {
        // Read the output WAV file and emit
        QFile file(m_pendingSynthFile);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray wavData = file.readAll();
            file.close();
            Logger::instance().log("Piper async TTS complete: "
                                   + QString::number(wavData.size()) + " bytes");
            emit audioReady(wavData);
        } else {
            emit ttsError("Failed to read synthesised audio file.");
        }
    }

    m_piperProc->deleteLater();
    m_piperProc = nullptr;
}

// ---------------------------------------------------------------------------
// Live recording
// ---------------------------------------------------------------------------

void VoiceService::startListening()
{
    if (m_listening) return;

    if (!m_sttReady) {
        emit sttError("Cannot listen: STT not initialised.");
        return;
    }

    m_recordBuffer.clear();
    m_listening = true;
    m_silenceTimer.start();

    Logger::instance().log("Voice listening started. Waiting for audio input...");
    emit listeningStarted();
}

void VoiceService::stopListening()
{
    if (!m_listening) return;

    m_silenceTimer.stop();
    m_listening = false;

    Logger::instance().log("Voice listening stopped. Captured "
                           + QString::number(m_recordBuffer.size()) + " bytes.");
    emit listeningStopped();

    // If we have audio data, trigger async transcription
    if (!m_recordBuffer.isEmpty() && m_sttReady) {
        transcribeAsync(m_recordBuffer);
    }
}

bool VoiceService::isListening() const { return m_listening; }

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

void VoiceService::setWhisperLanguage(const QString &lang)
{
    m_whisperLang = lang;
    Logger::instance().log("Whisper language set to: " + lang);
}

// ---------------------------------------------------------------------------
// Voice profiles
// ---------------------------------------------------------------------------

QMap<QString, VoiceProfile> VoiceService::availableProfiles()
{
    QMap<QString, VoiceProfile> profiles;

    // Breeze — calm, neutral, conversational (default)
    // Inspired by ChatGPT's "Breeze" voice: relaxed pacing, low
    // expressiveness variance, natural cadence. Ideal for long interactions.
    profiles["breeze"] = VoiceProfile{
        "Breeze",
        "Calm, neutral tone for everyday conversation",
        1.15,   // slightly slower than default → relaxed pacing
        0.667,  // moderate expressiveness
        0.80,   // natural phoneme width → balanced cadence
        200     // 0.2s pause between sentences
    };

    // Ember — warm, expressive, friendly
    profiles["ember"] = VoiceProfile{
        "Ember",
        "Warm and expressive with a friendly lilt",
        1.00,   // normal speed
        0.667,  // default expressiveness
        0.80,   // default phoneme randomness
        250     // default pause
    };

    // Cove — slow, deep, thoughtful
    profiles["cove"] = VoiceProfile{
        "Cove",
        "Deep and deliberate, ideal for explanations",
        1.30,   // noticeably slower
        0.35,   // very steady
        0.50,   // tight phoneme width → precise articulation
        400     // longer pauses for weight
    };

    // Juniper — fast, energetic, upbeat
    profiles["juniper"] = VoiceProfile{
        "Juniper",
        "Quick and energetic for rapid-fire interaction",
        0.85,   // faster than default
        0.70,   // slightly more expressive
        0.90,   // wider phoneme variation
        180     // shorter sentence gaps
    };

    // Sky — neutral, clear, professional
    profiles["sky"] = VoiceProfile{
        "Sky",
        "Clear and professional, suited for reading aloud",
        1.05,   // barely slower
        0.50,   // moderate steadiness
        0.70,   // balanced cadence
        280     // normal pauses
    };

    return profiles;
}

void VoiceService::setVoiceProfile(const QString &profileName)
{
    auto profiles = availableProfiles();
    QString key = profileName.toLower();

    if (!profiles.contains(key)) {
        Logger::instance().warning("Unknown voice profile '" + profileName
                                   + "'. Falling back to 'breeze'.");
        key = "breeze";
    }

    m_activeProfile = profiles[key];
    m_activeProfileName = key;
    Logger::instance().log("Voice profile set: " + m_activeProfile.name
                           + " – " + m_activeProfile.description);
}

QString VoiceService::currentProfileName() const
{
    return m_activeProfileName;
}

void VoiceService::speak(const QString &text)
{
    synthesizeAsync(text);
}

void VoiceService::appendProfileArgs(QStringList &args) const
{
    // Piper CLI speech parameter flags
    args << "--length_scale"
         << QString::number(m_activeProfile.lengthScale, 'f', 3);
    args << "--noise_scale"
         << QString::number(m_activeProfile.noiseScale, 'f', 3);
    args << "--noise_w"
         << QString::number(m_activeProfile.noiseW, 'f', 3);
    args << "--sentence_silence"
         << QString::number(m_activeProfile.sentenceSilenceMs / 1000.0, 'f', 3);
}
