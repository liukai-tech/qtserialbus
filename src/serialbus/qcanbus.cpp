/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtSerialBus module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qserialbusplugininterface.h"
#include "qcanbus.h"

#include <QtCore/qobject.h>
#include <QtCore/qpluginloader.h>
#include <QtCore/qdebug.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qglobalstatic.h>

#include <private/qfactoryloader_p.h>
#include <private/qlibrary_p.h>

#define QSerialBusPluginInterface_iid "org.qt-project.Qt.QSerialBusPluginInterface"

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, qFactoryLoader,
    (QSerialBusPluginInterface_iid, QLatin1String("/serialbuses")))

typedef QHash<QByteArray, QSerialBusBackendFactory*> QSerialBusPluginsHash;
Q_GLOBAL_STATIC(QSerialBusPluginsHash, qSerialBusPlugins)

static QCanBus *globalInstance = Q_NULLPTR;

static void loadPlugins()
{
    const QList<QJsonObject> meta = qFactoryLoader()->metaData();
    for (int i = 0; i < meta.count(); i++) {
        if (QSerialBusPluginInterface *plugin = qobject_cast<QSerialBusPluginInterface*>(qFactoryLoader->instance(i)))
            plugin->registerBus();
    }
}

/*!
    \class QCanBus
    \inmodule QtSerialBus
    \since 5.6

    \brief The QCanBus class handles registration and creation of bus backends.

    QCanBus loads Qt CAN Bus plugins at runtime. The ownership of serial bus backends is
    transferred to the loader.
*/

/*!
    Returns a pointer to the QCanBus class. The object is loaded if necessary. QCanBus
    uses the singleton design pattern.
 */
QCanBus *QCanBus::instance()
{
    if (!globalInstance)
        globalInstance = new QCanBus();
    return globalInstance;
}

/*!
    Registers a backend for the identifier specified by \a identifier, which must be unique.
    The \a factory will be asked to create instances of the backend.
 */
void QCanBus::registerBackend(const QByteArray &identifier, QSerialBusBackendFactory *factory)
{
    if (!qSerialBusPlugins()->contains(identifier))
        qSerialBusPlugins()->insert(identifier, factory);
}

/*!
    Returns a list of identifiers for all loaded plugins.
 */
QList<QByteArray> QCanBus::plugins()
{
    return qSerialBusPlugins()->keys();
}

/*!
    Returns a list of available backend names for \a identifier, or an empty list if no suitable
    \a identifier can be found.
*/

QStringList QCanBus::availableBackends(const QByteArray &identifier) const
{
    if (QSerialBusBackendFactory *factory = qSerialBusPlugins()->value(identifier))
        return factory->availableBackends();
    return QStringList();
}

/*!
    Creates a CAN bus device. \a plugin is the name of the plugin as returned by the \l plugins()
    method. \a identifier is the typ of the device inside the plugin. A single plugin may contain more
    than one device type. \a name is the network interface name.

    Ownership of the returned backend is transferred to the caller.
    Returns \c null if no suitable device can be found.
 */
QCanBusDevice *QCanBus::createDevice(const QByteArray &plugin,
                                     const QString &identifier,
                                     const QString &name) const
{
    if (QSerialBusBackendFactory *factory = qSerialBusPlugins()->value(plugin))
        return factory->createDevice(identifier, name);
    return Q_NULLPTR;
}

QCanBus::QCanBus(QObject *parent) :
    QObject(parent)
{
    loadPlugins();
}

/*!
    \class QSerialBusBackendFactory
    \inmodule QtSerialBus
    \since 5.6

    \brief The QSerialBusBackendFactory class instantiates instances of serial bus backends.

    This interface must be implemented in order to register a serial bus backend.
 */

/*!
    \fn QSerialBusBackendFactory::createBackend(const QString &busBackend, const QString &name) const

    Instantiates a backend. \a busBackend defines the backend in the plugin to be created. \a name
    is the network interface name.

    If the factory cannot create a backend, it should return 0.
*/

/*!
    \fn QSerialBusBackendFactory::availableBackends() const

    Lists the available backends.
*/

/*!
    \internal
 */
QSerialBusBackendFactory::~QSerialBusBackendFactory()
{
}

QT_END_NAMESPACE
