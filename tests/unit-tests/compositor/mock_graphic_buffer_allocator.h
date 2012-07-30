/*
 * Copyright © 2012 Canonical Ltd.
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
 * Authored by: Thomas Voss <thomas.voss@canonical.com>
 */

#ifndef MIR_COMPOSITOR_MOCK_GRAPHIC_BUFFER_ALLOCATOR_H_
#define MIR_COMPOSITOR_MOCK_GRAPHIC_BUFFER_ALLOCATOR_H_

#include "mir/compositor/graphic_buffer_allocator.h"
#include "mir/compositor/buffer.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mir_test/gmock_fixes.h"
#include "mock_buffer.h"

namespace mir
{
namespace compositor
{
const geometry::Width width {1024};
const geometry::Height height {768};
const geometry::Stride stride {geom::dim_cast<geom::Stride>(width)};
const PixelFormat pixel_format {PixelFormat::rgba_8888};

struct MockGraphicBufferAllocator : GraphicBufferAllocator
{
 public:
    MOCK_METHOD3(alloc_buffer, std::shared_ptr<Buffer> (geometry::Width, geometry::Height, PixelFormat));
    MOCK_METHOD1(free_buffer, void(std::shared_ptr<Buffer>));
};

}
}

#endif // MIR_COMPOSITOR_MOCK_GRAPHIC_BUFFER_ALLOCATOR_H_
