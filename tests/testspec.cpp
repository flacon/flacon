#include "testspec.h"
#include <yaml-cpp/yaml.h>
#include <QDir>
#include <QDebug>
#include <QTest>
#include "types.h"

static inline QString calcPath(const QString &p1, const QString &p2)
{
    return (p1.isEmpty() ? "" : p1 + ".") + p2;
}

static inline void mustExists(const TestSpec::Node *node)
{
    if (!node->exists()) {
        throw FlaconError("The spec file does not contain the \"" + node->path() + "\" tag!");
    }
}

/**************************************
 *
 **************************************/
TestSpec::Node::Node(YAML::Node yaml) :
    mYaml(yaml)
{
}

const TestSpec::Node TestSpec::Node::operator[](const QString &key) const
{
    Node res  = mYaml[key.toStdString()];
    res.mPath = calcPath(mPath, key);
    return res;
}

TestSpec::Node TestSpec::Node::operator[](const QString &key)
{
    Node res  = mYaml[key.toStdString()];
    res.mPath = calcPath(mPath, key);
    return res;
}

bool TestSpec::Node::isString() const
{
    return mYaml.IsScalar();
}

QString TestSpec::Node::toString() const
{
    mustExists(this);
    return toString("");
}

QString TestSpec::Node::toString(const QString &defaultValue) const
{
    if (mYaml.IsScalar()) {
        return QString::fromStdString(mYaml.Scalar());
    }
    return defaultValue;
}

QStringList TestSpec::Node::toStringList() const
{
    mustExists(this);
    return toStringList({});
}

QStringList TestSpec::Node::toStringList(const QStringList &defaultValue) const
{
    QStringList res;

    if (mYaml.IsScalar()) {
        res << QString::fromStdString(mYaml.Scalar());
        return res;
    }

    if (!mYaml.IsSequence()) {
        return defaultValue;
    }

    YAML::const_iterator it = mYaml.begin();
    while (it != mYaml.end()) {
        res << QString::fromStdString(it->Scalar());
        ++it;
    }

    return res;
}

static QFileInfo expandFilePath(const QString &dir, const QString &file)
{
    QFileInfo res(file);
    if (res.filePath().isEmpty() || res.isAbsolute()) {
        return res;
    }

    return QFileInfo(QDir(dir), res.filePath());
}

QFileInfo TestSpec::Node::toFileInfo(const QString &dir) const
{
    return expandFilePath(dir, toString());
}

QFileInfoList TestSpec::Node::toFileInfoList(const QString &dir) const
{
    QFileInfoList res;
    for (const QString &s : toStringList()) {
        res << expandFilePath(dir, s);
    }
    return res;
}

/**************************************
 *
 **************************************/
TestSpec::TestSpec(const QString &dir, const QString &file)
{
    QFileInfo fi(dir + "/" + file);
    mDir = fi.dir();

    try {
        mRoot = YAML::LoadFile(fi.filePath().toStdString());
    }
    catch (const YAML::Exception &err) {
        throw FlaconError(err.what());
    }
}

/**************************************
 *
 **************************************/
QString TestSpec::findFile(const QString &pattern) const
{
    QFileInfoList files = mDir.entryInfoList(QStringList(pattern), QDir::Files);
    if (files.count() < 1) {
        throw FlaconError(QString("%1 file not found").arg(pattern));
    }

    if (files.count() > 1) {
        throw FlaconError(QString("Multipy %1 files found").arg(pattern));
    }

    return files.first().filePath();
}

/**************************************
 *
 **************************************/
char *toString(const QFileInfo &value)
{
    return QTest::toString(value.filePath());
}
