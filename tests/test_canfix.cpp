/*
 * GoogleTest suite for canfix.cpp.
 *
 * Covers:
 *   1. CFParameter FCB bit-field pack/unpack (setMetaData/getMetaData,
 *      setFlags/getFlags)
 *   2. CanFix::checkParameterEnable — EEPROM bitmask logic
 *   3. CanFix::sendParam — CAN frame encoding
 *   4. CanFix::handleFrame — ID-range routing to callbacks
 *   5. CanFix::getNodeNumber — EEPROM fallback to device ID
 */
#include <gtest/gtest.h>
#include "mocks/can.h"
#include "mocks/EEPROM.h"
#include "../canfix.h"

// ── Test fixture with fresh mock state for each test ──────────────────────────

class CanFixTest : public ::testing::Test {
protected:
    void SetUp() override {
        g_can  = MockCanState{};
        EEPROM.reset();
    }
};

// ── CFParameter FCB field tests ───────────────────────────────────────────────

TEST(CFParameterTest, SetAndGetMetadata_Zero) {
    CFParameter p;
    p.fcb = 0xFF;
    p.setMetaData(0);
    EXPECT_EQ(p.getMetaData(), 0);
    // Lower nibble must be preserved
    EXPECT_EQ(p.fcb & 0x0F, 0x0F);
}

TEST(CFParameterTest, SetAndGetMetadata_NonZero) {
    CFParameter p;
    p.fcb = 0x00;
    p.setMetaData(0x0A);
    EXPECT_EQ(p.getMetaData(), 0x0A);
}

TEST(CFParameterTest, MetaDataMaxValue) {
    CFParameter p;
    p.fcb = 0x00;
    p.setMetaData(0x0F);
    EXPECT_EQ(p.getMetaData(), 0x0F);
}

TEST(CFParameterTest, SetAndGetFlags_Zero) {
    CFParameter p;
    p.fcb = 0xFF;
    p.setFlags(0);
    EXPECT_EQ(p.getFlags(), 0);
    // Upper nibble must be preserved
    EXPECT_EQ((p.fcb >> 4) & 0x0F, 0x0F);
}

TEST(CFParameterTest, SetAndGetFlags_NonZero) {
    CFParameter p;
    p.fcb = 0x00;
    p.setFlags(0x07);
    EXPECT_EQ(p.getFlags(), 0x07);
}

TEST(CFParameterTest, FlagsOnlyStoreLowNibble) {
    CFParameter p;
    p.fcb = 0x00;
    p.setFlags(0xFF);  // only low 4 bits should stick
    EXPECT_EQ(p.getFlags(), 0x0F);
}

TEST(CFParameterTest, MetaAndFlagsAreIndependent) {
    CFParameter p;
    p.fcb = 0x00;
    p.setMetaData(0x05);
    p.setFlags(0x03);
    EXPECT_EQ(p.getMetaData(), 0x05);
    EXPECT_EQ(p.getFlags(),    0x03);
    EXPECT_EQ(p.fcb, (uint8_t)((0x05 << 4) | 0x03));
}

// ── CanFix::checkParameterEnable ──────────────────────────────────────────────

class CheckParamEnableTest : public CanFixTest {
protected:
    // device id=1, pin=0; constructor calls EEPROM which is freshly reset
    CanFix cf{0, 1};
};

TEST_F(CheckParamEnableTest, AllEnabledByDefault) {
    // EEPROM.reset() fills with 0xFF; bit=1 means disabled, so
    // a fresh EEPROM has all parameters *disabled*.
    // After reset to 0x00 they are enabled.
    EEPROM.reset();
    for (int addr = 0; addr < EEPROM_SIZE; addr++) {
        EEPROM.write(addr, 0x00);
    }
    // Parameter 256 → byte 32, bit 0
    EXPECT_NE(cf.checkParameterEnable(256), 0);
}

TEST_F(CheckParamEnableTest, DisabledParameterReturnZero) {
    // Disable parameter 256: id=256 → index=32, offset=0
    EEPROM.write(32, 0x01);  // set bit 0
    EXPECT_EQ(cf.checkParameterEnable(256), 0);
}

TEST_F(CheckParamEnableTest, EnabledParameterReturnNonZero) {
    EEPROM.write(32, 0x00);  // clear bit 0
    EXPECT_NE(cf.checkParameterEnable(256), 0);
}

TEST_F(CheckParamEnableTest, ParameterAtBitOffset7) {
    // Parameter 263: index=32, offset=7
    EEPROM.write(32, 0x00);
    EXPECT_NE(cf.checkParameterEnable(263), 0);
    EEPROM.write(32, 0x80);  // set bit 7
    EXPECT_EQ(cf.checkParameterEnable(263), 0);
}

TEST_F(CheckParamEnableTest, ParameterAtNextByte) {
    // Parameter 264: index=33, offset=0
    EEPROM.write(33, 0x00);
    EXPECT_NE(cf.checkParameterEnable(264), 0);
    EEPROM.write(33, 0x01);
    EXPECT_EQ(cf.checkParameterEnable(264), 0);
}

// ── CanFix::sendParam — CAN frame encoding ────────────────────────────────────

class SendParamTest : public CanFixTest {
protected:
    CanFix cf{0, 42};  // device id = 42
};

TEST_F(SendParamTest, FrameIdMatchesParamType) {
    CFParameter p;
    p.type   = 0x0100;  // ID 256
    p.index  = 0;
    p.fcb    = 0;
    p.length = 0;
    cf.sendParam(p);
    EXPECT_EQ(g_can.last_written.id, 0x0100u);
}

TEST_F(SendParamTest, FrameData0IsNodeNumber) {
    // getNodeNumber() falls back to deviceid only when EEPROM[EE_NODE]==0x00.
    // Write 0x00 explicitly to trigger the fallback to deviceid=42.
    EEPROM.write(EE_NODE, 0x00);
    CFParameter p;
    p.type = 0x0100; p.index = 0; p.fcb = 0; p.length = 0;
    cf.sendParam(p);
    EXPECT_EQ(g_can.last_written.data[0], 42u);
}

TEST_F(SendParamTest, FrameData1IsIndex) {
    CFParameter p;
    p.type = 0x0100; p.index = 7; p.fcb = 0; p.length = 0;
    cf.sendParam(p);
    EXPECT_EQ(g_can.last_written.data[1], 7u);
}

TEST_F(SendParamTest, FrameData2IsFcb) {
    CFParameter p;
    p.type = 0x0100; p.index = 0; p.fcb = 0xAB; p.length = 0;
    cf.sendParam(p);
    EXPECT_EQ(g_can.last_written.data[2], 0xABu);
}

TEST_F(SendParamTest, FrameLengthIsPayloadPlusThree) {
    CFParameter p;
    p.type = 0x0100; p.index = 0; p.fcb = 0;
    p.length = 3;
    p.data[0] = 0xDE; p.data[1] = 0xAD; p.data[2] = 0xBE;
    cf.sendParam(p);
    EXPECT_EQ(g_can.last_written.length, 6u);
    EXPECT_EQ(g_can.last_written.data[3], 0xDEu);
    EXPECT_EQ(g_can.last_written.data[4], 0xADu);
    EXPECT_EQ(g_can.last_written.data[5], 0xBEu);
}

// ── CanFix::handleFrame — ID routing ─────────────────────────────────────────

static bool       s_param_called;
static CFParameter s_last_param;
static bool       s_alarm_called;

static void param_cb(CFParameter p) {
    s_param_called = true;
    s_last_param   = p;
}
static void alarm_cb(byte id, word type, byte *data, byte length) {
    (void)id; (void)type; (void)data; (void)length;
    s_alarm_called = true;
}

class HandleFrameTest : public CanFixTest {
protected:
    CanFix cf{0, 1};

    void SetUp() override {
        CanFixTest::SetUp();
        s_param_called = false;
        s_alarm_called = false;
        cf.set_param_callback(param_cb);
        cf.set_alarm_callback(alarm_cb);
    }
};

TEST_F(HandleFrameTest, ParameterFrameFiresParamCallback) {
    // ID in range 256–0x6DF → parameter callback
    CanFrame frame;
    frame.id      = 0x0100;  // 256
    frame.data[0] = 1;   // node
    frame.data[1] = 0;   // index
    frame.data[2] = 0;   // fcb
    frame.data[3] = 0xAB;
    frame.length  = 4;

    g_can.rx_status  = 0x40;  // buffer 0 has data
    g_can.rx_frame[0] = frame;
    cf.exec();

    EXPECT_TRUE(s_param_called);
    EXPECT_EQ(s_last_param.type,  0x0100u);
    EXPECT_EQ(s_last_param.node,  1u);
    EXPECT_EQ(s_last_param.data[0], 0xABu);
}

TEST_F(HandleFrameTest, AlarmFrameFiresAlarmCallback) {
    CanFrame frame;
    frame.id     = 0x0001;  // ID < 256 → alarm
    frame.data[0] = 0; frame.data[1] = 0;
    frame.length  = 2;

    g_can.rx_status   = 0x40;
    g_can.rx_frame[0] = frame;
    cf.exec();

    EXPECT_TRUE(s_alarm_called);
    EXPECT_FALSE(s_param_called);
}

TEST_F(HandleFrameTest, IdZeroIgnored) {
    CanFrame frame;
    frame.id = 0x0000;
    frame.length = 0;

    g_can.rx_status   = 0x40;
    g_can.rx_frame[0] = frame;
    cf.exec();

    EXPECT_FALSE(s_param_called);
    EXPECT_FALSE(s_alarm_called);
}

// ── CanFix::getNodeNumber — EEPROM fallback ───────────────────────────────────

TEST_F(CanFixTest, GetNodeNumberFallsBackToDeviceIdWhenEepromZero) {
    EEPROM.write(EE_NODE, 0x00);
    CanFix cf{0, 77};
    // Verify via sendParam: data[0] should be 77
    CFParameter p; p.type = 256; p.index = 0; p.fcb = 0; p.length = 0;
    cf.sendParam(p);
    EXPECT_EQ(g_can.last_written.data[0], 77u);
}

TEST_F(CanFixTest, GetNodeNumberReadsFromEepromWhenNonZero) {
    EEPROM.write(EE_NODE, 42);
    CanFix cf{0, 99};  // device id = 99 but EEPROM overrides
    CFParameter p; p.type = 256; p.index = 0; p.fcb = 0; p.length = 0;
    cf.sendParam(p);
    EXPECT_EQ(g_can.last_written.data[0], 42u);
}
