/*
    Copyright (C) 2014-2015 Islog

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

#pragma once

#include <string>
#include <zmqpp/zmqpp.hpp>

namespace Leosac
{
    namespace Hardware
    {

        /**
        * Facade object for a Wiegand Reader device.
        *
        * You can control a reader through this class by acting on its led and buzzer.
        *
        * ### Wiegand Reader specifications:
        * A wiegand reader device shall have 2 inputs GPIO configured and an optional greenled and buzzer
        * devices.
        *
        * ### Command specifications:
        * A wiegand device shall accept 4 commands:
        *    + `BEEP` to activate the buzzer device for a given amount of time.
        *    + `BEEP_ON` to activate the buzzer.
        *    + `BEEP_OFF` to deactivate the buzzer.
        *    + `GREEN_LED` to execute a command on the associated led device.
        *
        * Command (Frame 1)        | Frame 2             | Frame 3          | Description
        * -------------------------|---------------------|------------------|--------------
        * BEEP                     | Duration (msec)     |                  | Make the buzzer beep for the given duration
        * BEEP_ON                  |                     |                  | Turn the buzzer on (for an undefined time span)
        * BEEP_OFF                 |                     |                  | Turn the buzzer off
        * GREEN_LED                | A valid LED command | Parameters for command | Forward a LED command (and its parameters) to the green led
        *
        * @note A GREEN_LED command shall return a response as-if we sent a message to a LED device.
        * @see FLED for a list of led command.
        */
        class FWiegandReader
        {
        public:
            /**
            * Construct a facade to a wiegand reader; this facade will connect to the reader
            * @param ctx ZMQ context
            * @param reader_name name of targeted wiegand device
            */
            FWiegandReader(zmqpp::context &ctx, const std::string &reader_ame);

            FWiegandReader(const FWiegandReader &) = delete;

            FWiegandReader &operator=(const FWiegandReader &) = delete;

            ~FWiegandReader() = default;

            /**
            * Turn the reader's green led on.
            */
            bool greenLedOn();

            /**
            * Turn the reader's green led off.
            */
            bool greenLedOff();

            /**
            * Make the reader's green led blink.
            */
            bool greenLedBlink(int64_t duration = 1000, int64_t speed = 300);

            /**
            * Beep for a given duration
            */
            bool beep(int64_t duration = 1000);

            /**
            * Turn the buzzer on.
            */
            bool buzzerOn();

            /**
            * Turn the buzzer off.
            */
            bool buzzerOff();

            /**
            * Returns the device's name.
            */
            const std::string &name() const;

        private:

            /**
            * Send a message to the `backend_` wiegand reader and wait for a response.
            * The response shall be either "OK" or "KO"
            */
            bool send_to_backend(zmqpp::message &m);

            /**
            * A socket to talk to the backend wiegand reader.
            */
            zmqpp::socket backend_;

            std::string name_;
        };

    }
}