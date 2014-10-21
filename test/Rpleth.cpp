#include <memory>
#include <modules/rpleth/RplethModule.hpp>
#include <modules/rpleth/rplethprotocol.hpp>
#include "helper/TestHelper.hpp"
#include "helper/FakeWiegandReader.hpp"

class RplethTest : public TestHelper
{
public:
    virtual bool run_module(zmqpp::socket *pipe)
    {
        boost::property_tree::ptree cfg, module_cfg;

        module_cfg.add("port", "4242");
        module_cfg.add("reader", "WIEGAND1");
        module_cfg.add("stream_mode", "true");

        cfg.add_child("module_config", module_cfg);
        return test_run_module<RplethModule>(&ctx_, pipe, cfg);
    }

    RplethTest() :
            w1(ctx_, "WIEGAND1"),
            w1_actor(std::bind(&FakeWiegandReader::run, &w1, std::placeholders::_1))
    {
    }

    virtual ~RplethTest()
    {
    }

    RplethPacket extract_packet(const std::string &data)
    {
        ciruclar_buf_.write(reinterpret_cast<const unsigned char *>(data.c_str()), data.size()); // write those to circular buffer
        RplethPacket rpleth_packet = RplethProtocol::decodeCommand(ciruclar_buf_, true);
        return rpleth_packet;
    }

    /**
    * Simulate client buffer.
    */
    CircularBuffer ciruclar_buf_;

    zmqpp::socket connect_to_rpleth()
    {
        zmqpp::message msg;
        std::string connection_identity, data;

        zmqpp::socket client(ctx_, zmqpp::socket_type::stream);
        client.connect("tcp://127.0.0.1:4242");

        client.receive(msg);
        assert(msg.parts() == 2);
        msg >> connection_identity;
        assert(msg.size(1) == 0);
        return std::move(client);
    }

    /**
    * Check that we can read a rpleth from the socket and check that its valid.
    */
    void check_rpleth_card_msg(zmqpp::socket &source, const std::vector<uint8_t> card_binary)
    {
        zmqpp::message msg;
        std::string connection_identity, data;

        source.receive(msg);
        msg >> connection_identity;
        msg >> data; // data we would read from socket
        RplethPacket rpleth_packet = extract_packet(data);

        ASSERT_TRUE(rpleth_packet.isGood);
        ASSERT_EQ(card_binary.size(), rpleth_packet.dataLen);
        ASSERT_EQ(card_binary.size(), rpleth_packet.data.size());

        ASSERT_EQ(card_binary, rpleth_packet.data);
    }

    FakeWiegandReader w1;
    // to run the fake reader in a thread.
    zmqpp::actor w1_actor;
};

TEST(Rpleth, TestConvertCard)
{
    // we just need some conf so the module load.
    boost::property_tree::ptree cfg, module_cfg;

    module_cfg.add("port", "4242");
    module_cfg.add("reader", "WIEGAND1");
    module_cfg.add("stream_mode", "true");
    cfg.add_child("module_config", module_cfg);
    zmqpp::context_t ctx;
    zmqpp::socket pipe(ctx, zmqpp::socket_type::pair);
    RplethModule module(ctx, &pipe, cfg);

    std::vector<uint8_t> card_binary = {0xff, 0xff, 0xff, 0xff};
    ASSERT_EQ(card_binary, module.card_convert_from_text("ff:ff:ff:ff"));

    card_binary = {0x32, 0x12, 0x14, 0xae, 0xbc};
    ASSERT_EQ(card_binary, module.card_convert_from_text("32:12:14:ae:bc"));

    card_binary = {0x00, 0x00, 0x00, 0x00,};
    ASSERT_EQ(card_binary, module.card_convert_from_text("00:00:00:00"));
}

/**
* Check that a rpleth receive cards that are read.
*/
TEST_F(RplethTest, TestReceiveStreamCardsSimple)
{
    zmqpp::socket client = connect_to_rpleth();

    // fake wiegand reader activity.
    bus_push_.send(zmqpp::message() << "S_WIEGAND1" << "ff:ab:cd:ef:12");
    check_rpleth_card_msg(client, {0xff, 0xab, 0xcd, 0xef, 0x12});
}

TEST_F(RplethTest, TestReceiveStreamCards2)
{
    zmqpp::message msg;
    std::string connection_identity, data;
    zmqpp::socket client = connect_to_rpleth();

    // fake wiegand reader activity.
    bus_push_.send(zmqpp::message() << "S_WIEGAND1" << "ff:ab:cd:ef:12");
    check_rpleth_card_msg(client, {0xff, 0xab, 0xcd, 0xef, 0x12});

    bus_push_.send(zmqpp::message() << "S_WIEGAND1" << "11:22:33:44");
    check_rpleth_card_msg(client, {0x11, 0x22, 0x33, 0x44});

    std::cout << "toto" << std::endl;

    bus_push_.send(zmqpp::message() << "S_INGNORED_READER" << "11:22:33:44");
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
    // nothing to read anymore
    ASSERT_FALSE(client.receive(msg, true));
}