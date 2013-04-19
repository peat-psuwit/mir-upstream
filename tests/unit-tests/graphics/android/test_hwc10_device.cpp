/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#include "src/server/graphics/android/hwc10_device.h"
#include "mir_test_doubles/mock_display_support_provider.h"
#include "mir_test_doubles/mock_hwc_composer_device_1.h"
#include "mir_test_doubles/mock_hwc_organizer.h"
#include "mir_test_doubles/mock_android_buffer.h"
#include <gtest/gtest.h>

namespace mga=mir::graphics::android;
namespace mtd=mir::test::doubles;
namespace geom=mir::geometry;

class HWC10Device : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        using namespace testing;
        test_size = geom::Size{geom::Width{88}, geom::Height{4}};
        test_pf = geom::PixelFormat::abgr_8888;
        test_numfb = 558u;
        mock_device = std::make_shared<testing::NiceMock<mtd::MockHWCComposerDevice1>>();
        mock_organizer = std::make_shared<mtd::MockHWCOrganizer>();
        mock_fbdev = std::make_shared<mtd::MockDisplaySupportProvider>();
        ON_CALL(*mock_fbdev, display_size())
            .WillByDefault(Return(test_size)); 
        ON_CALL(*mock_fbdev, display_format())
            .WillByDefault(Return(test_pf)); 
        ON_CALL(*mock_fbdev, number_of_framebuffers_available())
            .WillByDefault(Return(test_numfb)); 
    }

    geom::PixelFormat test_pf;
    geom::Size test_size;
    unsigned int test_numfb;
    std::shared_ptr<mtd::MockHWCOrganizer> mock_organizer;
    std::shared_ptr<mtd::MockHWCComposerDevice1> mock_device;
    std::shared_ptr<mtd::MockDisplaySupportProvider> mock_fbdev;
};

TEST_F(HWC10Device, hwc10_gets_size_from_fb_dev)
{
    EXPECT_CALL(*mock_fbdev, display_size())
        .Times(1);
    mga::HWC10Device device(mock_device, mock_organizer, mock_fbdev);

    auto size = device.display_size();
    EXPECT_EQ(test_size, size);
}

TEST_F(HWC10Device, hwc10_gets_format_from_fb_dev)
{
    EXPECT_CALL(*mock_fbdev, display_format())
        .Times(1);
    mga::HWC10Device device(mock_device, mock_organizer, mock_fbdev);

    auto pf = device.display_format();
    EXPECT_EQ(test_pf, pf);
}

TEST_F(HWC10Device, hwc10_gets_numfb_from_fb_dev)
{
    EXPECT_CALL(*mock_fbdev, number_of_framebuffers_available())
        .Times(1);
    mga::HWC10Device device(mock_device, mock_organizer, mock_fbdev);

    auto numfb = device.number_of_framebuffers_available();
    EXPECT_EQ(test_numfb, numfb);
}

TEST_F(HWC10Device, hwc10_commit_frame)
{
    using namespace testing;

    EGLDisplay dpy;
    EGLSurface sur;
    EXPECT_CALL(*mock_device, set_interface(mock_device.get(), 1, _))
        .Times(1);

    mga::HWC10Device device(mock_device, mock_organizer, mock_fbdev);

    device.commit_frame(dpy, sur);

    Mock::VerifyAndClearExpectations(mock_device.get()); 
    EXPECT_EQ(dpy, mock_device->display0_content.dpy);
    EXPECT_EQ(sur, mock_device->display0_content.sur);
    EXPECT_EQ(-1, mock_device->display0_content.retireFenceFd);
    EXPECT_EQ(0u, mock_device->display0_content.flags);
    EXPECT_EQ(0u, mock_device->display0_content.numHwLayers);
    EXPECT_EQ(nullptr, mock_device->display0_content.hwLayers);
}
