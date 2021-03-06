/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWINDOWSVULKANINSTANCE_H
#define QWINDOWSVULKANINSTANCE_H

#if defined(VULKAN_H_) && !defined(VK_USE_PLATFORM_WIN32_KHR)
#error "vulkan.h included without Win32 WSI"
#endif

#define VK_USE_PLATFORM_WIN32_KHR

#include <QtGui/private/qbasicvulkanplatforminstance_p.h>
#include <QtCore/qlibrary.h>

QT_BEGIN_NAMESPACE

class QWindowsVulkanInstance : public QBasicPlatformVulkanInstance
{
    Q_DISABLE_COPY_MOVE(QWindowsVulkanInstance)
public:
    QWindowsVulkanInstance(QVulkanInstance *instance);

    void createOrAdoptInstance() override;
    bool supportsPresent(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, QWindow *window) override;

    VkSurfaceKHR createSurface(HWND win);

private:
    QVulkanInstance *m_instance;
    PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR m_getPhysDevPresSupport;
    PFN_vkCreateWin32SurfaceKHR m_createSurface;
};

QT_END_NAMESPACE

#endif // QWINDOWSVULKANINSTANCE_H
