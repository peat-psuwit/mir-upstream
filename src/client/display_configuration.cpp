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

#include "display_configuration.h"

#include <cstring>

namespace mcl = mir::client;
namespace mp = mir::protobuf;

void mcl::delete_config_storage(MirDisplayConfiguration* config)
{
    if (!config)
        return;

    for(auto i=0u; i< config->num_displays; i++)
    {
        if (config->displays[i].modes)
            ::operator delete(config->displays[i].modes);
        if (config->displays[i].output_formats)
            ::operator delete(config->displays[i].output_formats);
    }
    if (config->displays)
        ::operator delete(config->displays);

    ::operator delete(config);
}

namespace
{
void fill_display_output(MirDisplayOutput& output, mp::DisplayOutput const& msg)
{
    output.card_id = msg.card_id();
    output.output_id = msg.output_id();

    for(auto i=0u; i < output.num_modes; i++)
    {
        auto mode = msg.mode(i);
        output.modes[i].horizontal_resolution = mode.horizontal_resolution(); 
        output.modes[i].vertical_resolution = mode.vertical_resolution(); 
        output.modes[i].refresh_rate = mode.refresh_rate();
    }
    output.current_mode = msg.current_mode();

    for(auto i=0u; i < output.num_output_formats; i++)
    {
        output.output_formats[i] = static_cast<MirPixelFormat>(msg.pixel_format(i));
    }
    output.current_output_format = msg.current_format();

    output.position_x = msg.position_x();
    output.position_y = msg.position_y();
    output.connected = msg.connected();
    output.used = msg.used();
    output.physical_width_mm = msg.physical_width_mm();
    output.physical_height_mm = msg.physical_height_mm();
}

}


mcl::DisplayConfiguration::DisplayConfiguration()
    : notify_change([]{})
{
}

mcl::DisplayConfiguration::~DisplayConfiguration()
{
}

void mcl::DisplayConfiguration::update_configuration(mp::Connection const& connection_msg)
{
    std::unique_lock<std::mutex> lk(guard);
    outputs.clear();
    for(auto i=0; i < connection_msg.display_output_size(); i++)
    {
        auto const& msg_output = connection_msg.display_output(i);
        auto output = std::make_shared<mcl::DisplayOutput>(msg_output.mode_size(), msg_output.pixel_format_size());
        fill_display_output(*output, msg_output);
        outputs.push_back(output);
    }
}

void mcl::DisplayConfiguration::update_configuration(mp::DisplayConfiguration const& msg)
{
    {
        std::unique_lock<std::mutex> lk(guard);
        outputs.clear();
        for(auto i=0; i < msg.display_output_size(); i++)
        {
            auto const& msg_output = msg.display_output(i);
            auto output = std::make_shared<mcl::DisplayOutput>(msg_output.mode_size(), msg_output.pixel_format_size());
            fill_display_output(*output, msg_output);
            outputs.push_back(output);
        }
    }

    notify_change();
}

//user is responsible for freeing the returned value
MirDisplayConfiguration* mcl::DisplayConfiguration::copy_to_client() const
{
    std::unique_lock<std::mutex> lk(guard);
    auto new_config = static_cast<MirDisplayConfiguration*>(::operator new(sizeof(MirDisplayConfiguration)));
    new_config->num_displays = outputs.size();
    new_config->displays = static_cast<MirDisplayOutput*>(::operator new(sizeof(MirDisplayOutput)*new_config->num_displays));

    auto i=0u;
    for( auto const& out : outputs)
    {
        auto new_info = &new_config->displays[i++];
        MirDisplayOutput* output = out.get();
        std::memcpy(new_info, output, sizeof(MirDisplayOutput)); 

        auto format_size = sizeof(MirPixelFormat)* new_info->num_output_formats;
        new_info->output_formats = static_cast<MirPixelFormat*>(::operator new(format_size));
        std::memcpy(new_info->output_formats, output->output_formats, format_size);

        auto mode_size = sizeof(MirDisplayMode)* new_info->num_modes;
        new_info->modes = static_cast<MirDisplayMode*>(::operator new(mode_size));
        std::memcpy(new_info->modes, output->modes, mode_size);
    }
    return new_config;
}

void mcl::DisplayConfiguration::set_display_change_handler(std::function<void()> const& fn)
{
    std::unique_lock<std::mutex> lk(guard);
    notify_change = fn;
}
