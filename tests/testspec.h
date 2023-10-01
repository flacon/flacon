/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2023
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

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
