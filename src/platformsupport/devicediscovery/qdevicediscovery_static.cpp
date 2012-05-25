/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdevicediscovery_p.h"

#include <QStringList>
#include <QCoreApplication>
#include <QObject>
#include <QHash>
#include <QDir>
#include <QtCore/private/qcore_unix_p.h>

#include <linux/input.h>
#include <fcntl.h>

//#define QT_QPA_DEVICE_DISCOVERY_DEBUG

#ifdef QT_QPA_DEVICE_DISCOVERY_DEBUG
#include <QtDebug>
#endif

#define LONG_BITS (sizeof(int) * 8 )
#define LONG_FIELD_SIZE(bits) ((bits / LONG_BITS) + 1)

static bool testBit(long bit, const long *field)
{
    return (field[bit / LONG_BITS] >> bit % LONG_BITS) & 1;
}

QT_BEGIN_NAMESPACE

QDeviceDiscovery *QDeviceDiscovery::create(QDeviceTypes types, QObject *parent)
{
    return new QDeviceDiscovery(types, parent);
}

QDeviceDiscovery::QDeviceDiscovery(QDeviceTypes types, QObject *parent) :
    QObject(parent),
    m_types(types)
{
#ifdef QT_QPA_DEVICE_DISCOVERY_DEBUG
    qWarning() << "New DeviceDiscovery created for type" << types;
#endif
}

QDeviceDiscovery::~QDeviceDiscovery()
{
}

QStringList QDeviceDiscovery::scanConnectedDevices()
{
    QStringList devices;

    // check for input devices
    QDir dir(QString::fromLatin1(QT_EVDEV_DEVICE_PATH));
    dir.setFilter(QDir::System);

    foreach (const QString &deviceFile, dir.entryList()) {
        QString absoluteFilePath = dir.absolutePath() + QString::fromLatin1("/") + deviceFile;
        if (checkDeviceType(absoluteFilePath))
            devices << absoluteFilePath;
    }

    // check for drm devices
    dir.setPath(QString::fromLatin1(QT_DRM_DEVICE_PATH));
    foreach (const QString &deviceFile, dir.entryList()) {
        QString absoluteFilePath = dir.absolutePath() + QString::fromLatin1("/") + deviceFile;
        if (checkDeviceType(absoluteFilePath))
            devices << absoluteFilePath;
    }

#ifdef QT_QPA_DEVICE_DISCOVERY_DEBUG
    qWarning() << "DeviceDiscovery found matching devices" << devices;
#endif

    return devices;
}

bool QDeviceDiscovery::checkDeviceType(const QString &device)
{
    bool ret = false;
    int fd = QT_OPEN(device.toLocal8Bit().constData(), O_RDONLY | O_NDELAY, 0);
    if (!fd) {
#ifdef QT_QPA_DEVICE_DISCOVERY_DEBUG
        qWarning() << "DeviceDiscovery cannot open device" << device;
#endif
        return false;
    }

    long bitsKey[LONG_FIELD_SIZE(KEY_CNT)];
    if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(bitsKey)), bitsKey) >= 0 ) {
        if (!ret && (m_types & Device_Keyboard)) {
            if (testBit(KEY_Q, bitsKey)) {
#ifdef QT_QPA_DEVICE_DISCOVERY_DEBUG
                qWarning() << "DeviceDiscovery found keyboard at" << device;
#endif
                ret = true;
            }
        }

        if (!ret && (m_types & Device_Mouse)) {
            long bitsRel[LONG_FIELD_SIZE(REL_CNT)];
            if (ioctl(fd, EVIOCGBIT(EV_REL, sizeof(bitsRel)), bitsRel) >= 0 ) {
                if (testBit(REL_X, bitsRel) && testBit(REL_Y, bitsRel) && testBit(BTN_MOUSE, bitsKey)) {
#ifdef QT_QPA_DEVICE_DISCOVERY_DEBUG
                    qWarning() << "DeviceDiscovery found mouse at" << device;
#endif
                    ret = true;
                }
            }
        }

        if (!ret && (m_types & (Device_Touchpad | Device_Touchscreen))) {
            long bitsAbs[LONG_FIELD_SIZE(ABS_CNT)];
            if (ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(bitsAbs)), bitsAbs) >= 0 ) {
                if (testBit(ABS_X, bitsAbs) && testBit(ABS_Y, bitsAbs)) {
                    if ((m_types & Device_Touchpad) && testBit(BTN_TOOL_FINGER, bitsKey)) {
#ifdef QT_QPA_DEVICE_DISCOVERY_DEBUG
                        qWarning() << "DeviceDiscovery found touchpad at" << device;
#endif
                        ret = true;
                    } else if ((m_types & Device_Touchscreen) && testBit(BTN_TOUCH, bitsKey)) {
#ifdef QT_QPA_DEVICE_DISCOVERY_DEBUG
                        qWarning() << "DeviceDiscovery found touchscreen at" << device;
#endif
                        ret = true;
                    }
                }
            }
        }
    }

    if (!ret && (m_types & Device_DRM) && device.contains(QString::fromLatin1(QT_DRM_DEVICE_PREFIX)))
        ret = true;

    QT_CLOSE(fd);
    return ret;
}

QT_END_NAMESPACE
