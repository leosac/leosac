#include <gtest/gtest.h>
#include <core/auth/AuthSourceBuilder.hpp>
#include "core/auth/Interfaces/IAuthSourceMapper.hpp"
#include "modules/auth/auth-file/FileAuthSourceMapper.hpp"

using namespace Leosac::Auth;
using namespace Leosac::Module::Auth;

namespace Leosac
{
    namespace Test
    {
        /**
        * Test the AuthSourceBuilder ability to create AuthSource object
        * from messages.
        */
        class AuthSourceBuilderTest : public ::testing::Test
        {
        public:
            AuthSourceBuilderTest()
            {
                msg1_ << "S_MY_WIEGAND_1";
                msg1_ << "SIMPLE_WIEGAND";
                msg1_ << "af:bc:12:42";
                msg1_ << 32;
            }

            ~AuthSourceBuilderTest()
            {
            }

            AuthSourceBuilder builder_;
            /**
            * This looks like a message sent by MY_WIEGAND_1 with SIMPLE_WIEGAND data.
            */
            zmqpp::message msg1_;
        };

        TEST_F(AuthSourceBuilderTest, ExtractSourceName)
        {
            std::string out;

            ASSERT_TRUE(builder_.extract_source_name("S_MY_DEVICE", &out));
            ASSERT_EQ(out, "MY_DEVICE");

            ASSERT_TRUE(builder_.extract_source_name("S___MY_DEVICE", &out));
            ASSERT_EQ(out, "__MY_DEVICE");

            ASSERT_FALSE(builder_.extract_source_name("MY_DEVICE", &out));
            ASSERT_FALSE(builder_.extract_source_name("", &out));
            ASSERT_FALSE(builder_.extract_source_name("S_", &out));
            ASSERT_FALSE(builder_.extract_source_name("D", &out));
        }

        TEST_F(AuthSourceBuilderTest, SimpleBuild)
        {
            IAuthenticationSourcePtr auth_source = builder_.create(&msg1_);
            ASSERT_TRUE(auth_source.get());

            ASSERT_EQ(auth_source->name(), "MY_WIEGAND_1");
        }
    }
}