/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#include "mir/graphics/renderable.h"
#include "mir/graphics/buffer.h"
#include "mir/graphics/android/sync_fence.h"
#include "mir/graphics/android/native_buffer.h"
#include "hwc_layerlist.h"

#include <cstring>

namespace mg=mir::graphics;
namespace mga=mir::graphics::android;
namespace geom=mir::geometry;

namespace
{
std::shared_ptr<hwc_display_contents_1_t> generate_hwc_list(size_t needed_size)
{
    /* hwc layer list uses hwLayers[0] at the end of the struct */
    auto struct_size = sizeof(hwc_display_contents_1_t) + sizeof(hwc_layer_1_t)*(needed_size);
    auto new_hwc_representation = std::shared_ptr<hwc_display_contents_1_t>(
        static_cast<hwc_display_contents_1_t*>( ::operator new(struct_size)));

    new_hwc_representation->numHwLayers = needed_size;
    new_hwc_representation->retireFenceFd = -1;

    //as far as HWC is concerned, two things can change. If the size of the list, or the position
    //of the surfaces in the list change, then we must toggle this flag to on. 
    //TODO: toggle this flag on/off appropriately. 
    new_hwc_representation->flags = HWC_GEOMETRY_CHANGED;

    //aosp exynos hwc in particular, checks that these fields are non-null in hwc1.1, although
    //these fields are deprecated in hwc1.1 and later.
    static int fake_egl_values = 0;
    new_hwc_representation->dpy = &fake_egl_values;
    new_hwc_representation->sur = &fake_egl_values;

    return new_hwc_representation;
}
}

void mga::LayerListBase::update_representation(
    size_t needed_size, std::list<std::shared_ptr<mg::Renderable>> const& renderlist)
{
    any_buffer_updated = false;
    if (hwc_representation->numHwLayers != needed_size)
    {
        hwc_representation = generate_hwc_list(needed_size);
    }

    if (layers.size() == needed_size)
    {
        auto layers_it = layers.begin();
        for(auto renderable : renderlist)
        {
            layers_it->set_render_parameters(
                renderable->screen_position(), renderable->alpha_enabled());
            layers_it->set_buffer(*renderable->buffer());
            any_buffer_updated |= layers_it->needs_hwc_commit(); 
            layers_it++;
        }
    }
    else
    {
        any_buffer_updated = true;
        std::list<HWCLayer> new_layers;
        auto i = 0u;
        for(auto const& renderable : renderlist)
        {
            new_layers.emplace_back(
                mga::HWCLayer(
                    mga::LayerType::gl_rendered,
                    renderable->screen_position(),
                    renderable->alpha_enabled(),
                    hwc_representation, i++));
            new_layers.back().set_buffer(*renderable->buffer());
        }

        for(; i < needed_size; i++)
        {
            new_layers.emplace_back(mga::HWCLayer(hwc_representation, i));
        }
        layers = std::move(new_layers);
    }
}

bool mga::LayerListBase::list_has_changed()
{
    return any_buffer_updated;
}

std::weak_ptr<hwc_display_contents_1_t> mga::LayerListBase::native_list()
{
    return hwc_representation;
}

mga::NativeFence mga::LayerListBase::retirement_fence()
{
    return hwc_representation->retireFenceFd;
}

mga::LayerListBase::LayerListBase(size_t initial_list_size)
    : hwc_representation{generate_hwc_list(initial_list_size)}
{
    update_representation(initial_list_size, {});
}

mga::LayerList::LayerList()
    : LayerListBase{1}
{
    layers.back().set_layer_type(mga::LayerType::skip);
}
