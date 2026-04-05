#pragma once

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QProcess>
#include <QBuffer>
#include <QTimer>
#include <QTemporaryDir>
#include <QMap>

class ModelLocator;

/// Voice profile describing TTS behaviour (speed, pitch, style).
/// Piper supports these via --length_scale (speed), --noise_scale
/// (expressiveness), and --noise_w (phoneme width / cadence).
struct VoiceProfile {
    QString name;              // human-readable label
    QString description;       // short tagline
    double  lengthScale;       // >1 = slower, <1 = faster  (default 1.0)
    double  noiseScale;        // expressiveness / variation (default 0.667)
    double  noiseW;            // phoneme-width randomness   (default 0.8)
    int     sentenceSilenceMs; // pause between sentences     (default 250)
};

/// VoiceService wraps Whisper.cpp (STT) and Piper (TTS) as external CLI
/// processes. Audio is captured via Qt multimedia, written to temporary WAV
/// files, and piped through the whisper/piper binaries for transcription
/// and synthesis respectively.
class VoiceService : public QObject
{
    Q_OBJECT

public:
    explicit VoiceService(ModelLocator &locator, QObject *parent = nullptr);
    ~VoiceService();

    // ---------- Initialisation ----------
    bool initSTT();
    bool initTTS();
    bool isSTTReady() const;
    bool isTTSReady() const;

    // ---------- Speech-to-Text (Whisper) ----------
    /// Transcribe raw 16-bit 16 kHz mono PCM data synchronously.
    QString transcribe(const QByteArray &pcmData);

    /// Transcribe asynchronously; emits transcriptionReady on completion.
    void transcribeAsync(const QByteArray &pcmData);

    /// Transcribe a WAV file already on disk.
    QString transcribeFile(const QString &wavPath);

    // ---------- Text-to-Speech (Piper) ----------
    /// Synthesise text to WAV bytes synchronously.
    QByteArray synthesize(const QString &text);

    /// Synthesise asynchronously; emits audioReady on completion.
    void synthesizeAsync(const QString &text);

    /// Synthesise text and write to a WAV file; returns file path.
    QString synthesizeToFile(const QString &text);

    /// Convenience: speak text aloud (non-blocking TTS).
    void speak(const QString &text);

    // ---------- Live recording ----------
    void startListening();
    void stopListening();
    bool isListening() const;

    // ---------- Configuration ----------
    void setWhisperLanguage(const QString &lang);

    // ---------- Voice profiles ----------
    /// Returns all built-in voice profiles.
    static QMap<QString, VoiceProfile> availableProfiles();

    /// Set the active TTS voice profile by name (e.g. "breeze").
    void setVoiceProfile(const QString &profileName);

    /// Currently active profile name.
    QString currentProfileName() const;

signals:
    void transcriptionReady(const QString &text);
    void audioReady(const QByteArray &wavData);
    void sttError(const QString &error);
    void ttsError(const QString &error);
    void listeningStarted();
    void listeningStopped();

private slots:
    void onWhisperFinished(int exitCode, QProcess::ExitStatus status);
    void onPiperFinished(int exitCode, QProcess::ExitStatus status);

private:
    // WAV file helpers
    QString writePcmToWav(const QByteArray &pcmData);
    static void writeWavHeader(QIODevice *device, qint32 dataSize,
                               int sampleRate, int channels, int bitsPerSample);

    // Binary path resolution
    QString whisperBinaryPath() const;
    QString piperBinaryPath() const;

    ModelLocator &m_locator;

    // STT state
    bool m_sttReady = false;
    QString m_whisperModel;
    QString m_whisperLang = "en";
    QProcess *m_whisperProc = nullptr;
    QString m_pendingTranscribeOutput;

    // TTS state
    bool m_ttsReady = false;
    QString m_piperModel;
    QString m_piperConfig;
    QProcess *m_piperProc = nullptr;
    QString m_pendingSynthFile;

    // Active voice profile (defaults to "breeze")
    VoiceProfile m_activeProfile;
    QString m_activeProfileName = "breeze";

    /// Appends profile-specific Piper CLI args.
    void appendProfileArgs(QStringList &args) const;

    // Recording state
    bool m_listening = false;
    QByteArray m_recordBuffer;
    QTimer m_silenceTimer;

    // Temp directory for WAV files
    QTemporaryDir m_tempDir;

    // Audio constants
    static constexpr int kSampleRate   = 16000;
    static constexpr int kChannels     = 1;
    static constexpr int kBitsPerSample = 16;
    static constexpr int kSilenceTimeoutMs = 2000;
};
