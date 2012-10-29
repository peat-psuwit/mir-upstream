/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voss <thomas.voss@canonical.com>
 */

#ifndef MIR_FRONTEND_SERVICES_SURFACE_FACTORY_H_
#define MIR_FRONTEND_SERVICES_SURFACE_FACTORY_H_

#include <memory>

namespace mir
{
namespace surfaces
{

class Surface;
class SurfaceCreationParameters;

}
namespace frontend
{
namespace services
{

namespace ms = mir::surfaces;

class SurfaceFactory
{
 public:
    virtual ~SurfaceFactory() {}

    virtual int create_surface(const ms::SurfaceCreationParameters& params) = 0;
    virtual void destroy_surface(int surface_id) = 0;

 protected:
    SurfaceFactory() = default;
    SurfaceFactory(const SurfaceFactory&) = delete;
    SurfaceFactory& operator=(const SurfaceFactory&) = delete;
};

}
}
}

#endif // MIR_FRONTEND_SERVICES_SURFACE_FACTORY_H_
