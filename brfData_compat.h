/* brfData_compat.h — Compatibility shim for building without Qt.
 * Include this BEFORE brfData.h when BRF_NO_QT is defined.
 * It provides minimal stubs so the core BRF headers compile. */

#ifndef BRFDATA_COMPAT_H
#define BRFDATA_COMPAT_H

#ifdef BRF_NO_QT

// brfData.h includes <QFileInfo> and <QString> but doesn't actually
// need them for the sync tool — the fileName field is a std::string
// and QFileInfo is never used in the core logic.
// We provide trivial shims so brfData.h compiles.

#include <string>

// Minimal QString stand-in — just enough for brfData.h to compile.
// The core BRF code (brfMesh, brfBody, etc.) never uses QString.
class QString {
public:
    QString() {}
    QString(const char* s) : _s(s ? s : "") {}
    QString(const std::string& s) : _s(s) {}
    std::string toStdString() const { return _s; }
    const char* toLatin1Data() const { return _s.c_str(); }
    bool isEmpty() const { return _s.empty(); }
    int length() const { return (int)_s.size(); }
    static QString fromStdString(const std::string& s) { return QString(s); }
private:
    std::string _s;
};

// Minimal QFileInfo stand-in — only default-constructible.
class QFileInfo {
public:
    QFileInfo() {}
};

#endif // BRF_NO_QT
#endif // BRFDATA_COMPAT_H
