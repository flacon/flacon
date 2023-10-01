#ifndef TESTSPEC_H
#define TESTSPEC_H

#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include <yaml-cpp/node/node.h>

class TestSpec
{

public:
    class Node;
    using NodeList = QList<Node>;

    class Node
    {
        friend TestSpec;

    public:
        Node()                  = default;
        Node(const Node &other) = default;
        Node &operator=(const Node &other) = default;

        const Node operator[](const QString &key) const;
        Node       operator[](const QString &key);
        Node       operator[](size_t index) const;

        bool isString() const;

        QString path() const { return mPath; }

        QString       toString() const;
        QString       toString(const QString &defaultValue) const;
        QStringList   toStringList() const;
        QStringList   toStringList(const QStringList &defaultValue) const;
        QFileInfo     toFileInfo(const QString &dir) const;
        QFileInfoList toFileInfoList(const QString &dir) const;
        NodeList      toArray() const;

        std::size_t size() const { return mYaml.size(); }
        bool        exists() const { return mYaml.IsDefined(); }

    private:
        Node(YAML::Node yaml);
        YAML::Node mYaml;
        QString    mPath;
    };

public:
    TestSpec(const QString &dir, const QString &file = "spec.yaml");

    static bool exists(const QString &dir, const QString &file = "spec.yaml");

    const Node operator[](const QString &key) const { return mRoot[key]; }
    Node       operator[](const QString &key) { return mRoot[key]; }

    QString findFile(const QString &pattern) const;

private:
    Node mRoot;
    QDir mDir;
};

char *toString(const QFileInfo &value);

#endif // TESTSPEC_H
