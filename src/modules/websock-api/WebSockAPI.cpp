/*
    Copyright (C) 2014-2016 Islog

    This file is part of Leosac.

    Leosac is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Leosac is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "WebSockAPI.hpp"
#include "WSServer.hpp"
#include "core/CoreAPI.hpp"
#include "core/CoreUtils.hpp"
#include "core/auth/Token.hpp"
#include "core/auth/User.hpp"
#include <boost/filesystem.hpp>
#include <tools/XmlPropertyTree.hpp>
#include <tools/db/database.hpp>

using namespace Leosac;
using namespace Leosac::Module;
using namespace Leosac::Module::WebSockAPI;

WebSockAPIModule::WebSockAPIModule(zmqpp::context &ctx, zmqpp::socket *pipe,
                                   const boost::property_tree::ptree &cfg,
                                   CoreUtilsPtr utils)
    : BaseModule(ctx, pipe, cfg, utils)
{
    port_ = cfg.get<uint16_t>("module_config.port", 8976);
}

void WebSockAPIModule::run()
{
    WSServer srv(*this, core_utils()->database());
    std::thread thread(std::bind(&WSServer::run, &srv, port_));

    while (is_running_)
    {
        reactor_.poll();
    }
    INFO("WEBSOCK WILL START STOPPING...");
    srv.start_shutdown();
    thread.join();
}

CoreUtilsPtr WebSockAPIModule::core_utils()
{
    return utils_;
}

SessionAborted::SessionAborted()
    : LEOSACException("Websocket session has been aborted.")
{
}

SessionAborted::SessionAborted(Auth::TokenPtr token)
    : LEOSACException(build_msg(token))
{
}

std::string SessionAborted::build_msg(Auth::TokenPtr token)
{
    std::stringstream ss;
    ss << "Websocket session has been aborted.";

    if (!token)
        ss << "No associated token found.";
    else
    {
        ss << " Token " << token->token();
        if (token->owner())
            ss << ", owned by user " << token->owner()->id();
        else
            ss << ", with no owner ";
        ss << " expired on " << token->expiration();
    }
    return ss.str();
}

EntityNotFound::EntityNotFound(const std::string &id, const std::string &type)
    : LEOSACException(build_msg(id, type))
    , entity_id_(id)
    , entity_type_(type)
{
}

std::string EntityNotFound::build_msg(const std::string &id, const std::string &type)
{
    std::stringstream ss;
    ss << "Cannot find `" << type << "` with id `" << id << "`.";

    return ss.str();
}

const std::string &EntityNotFound::entity_id() const
{
    return entity_id_;
}

const std::string &EntityNotFound::entity_type() const
{
    return entity_type_;
}
