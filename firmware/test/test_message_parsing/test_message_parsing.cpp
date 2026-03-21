#include <unity.h>
#include <cstring>
#include <ArduinoJson.h>
#include "commands.h"
#include "config.h"

// Test that WebSocket JSON messages parse correctly into Command structs.
// This tests the parsing logic independent of the web server.

static bool parseCommand(const char* json, Command& cmd) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, json);
    if (err) return false;

    const char* cmd_str = doc["cmd"];
    if (!cmd_str) return false;

    memset(&cmd, 0, sizeof(cmd));

    if (strcmp(cmd_str, "play") == 0) {
        cmd.type = CommandType::PLAY;
        const char* id = doc["trajectory_id"] | "";
        strlcpy(cmd.data.play.trajectory_id, id, 16);
    }
    else if (strcmp(cmd_str, "pause") == 0)  { cmd.type = CommandType::PAUSE; }
    else if (strcmp(cmd_str, "resume") == 0) { cmd.type = CommandType::RESUME; }
    else if (strcmp(cmd_str, "stop") == 0)   { cmd.type = CommandType::STOP; }
    else if (strcmp(cmd_str, "jog") == 0) {
        cmd.type = CommandType::JOG;
        for (int i = 0; i < g_axis_count; i++) {
            cmd.data.jog.axes[i] = doc[g_axis_config[i].name] | 0.0f;
        }
    }
    else if (strcmp(cmd_str, "move_to") == 0) {
        cmd.type = CommandType::MOVE_TO;
        cmd.data.move_to.duration_ms = doc["duration_ms"] | 1000;
        for (int i = 0; i < g_axis_count; i++) {
            cmd.data.move_to.axes[i] = doc[g_axis_config[i].name] | 0.0f;
        }
    }
    else if (strcmp(cmd_str, "home") == 0) {
        cmd.type = CommandType::HOME;
        JsonArray axes = doc["axes"];
        cmd.data.home.axis_mask = 0;
        for (JsonVariant v : axes) {
            const char* name = v.as<const char*>();
            if (!name) continue;
            for (int i = 0; i < g_axis_count; i++) {
                if (strcmp(name, g_axis_config[i].name) == 0) {
                    cmd.data.home.axis_mask |= (1 << i);
                }
            }
        }
    }
    else if (strcmp(cmd_str, "scrub") == 0) {
        cmd.type = CommandType::SCRUB;
        cmd.data.scrub.t_ms = doc["t"] | 0;
    }
    else {
        return false;
    }

    return true;
}

void setUp(void) {}
void tearDown(void) {}

// --- Play ---

void test_parse_play() {
    Command cmd;
    TEST_ASSERT_TRUE(parseCommand(
        R"({"cmd":"play","trajectory_id":"abc123"})", cmd));
    TEST_ASSERT_EQUAL(CommandType::PLAY, cmd.type);
    TEST_ASSERT_EQUAL_STRING("abc123", cmd.data.play.trajectory_id);
}

// --- Pause / Resume / Stop ---

void test_parse_pause() {
    Command cmd;
    TEST_ASSERT_TRUE(parseCommand(R"({"cmd":"pause"})", cmd));
    TEST_ASSERT_EQUAL(CommandType::PAUSE, cmd.type);
}

void test_parse_resume() {
    Command cmd;
    TEST_ASSERT_TRUE(parseCommand(R"({"cmd":"resume"})", cmd));
    TEST_ASSERT_EQUAL(CommandType::RESUME, cmd.type);
}

void test_parse_stop() {
    Command cmd;
    TEST_ASSERT_TRUE(parseCommand(R"({"cmd":"stop"})", cmd));
    TEST_ASSERT_EQUAL(CommandType::STOP, cmd.type);
}

// --- Jog ---

void test_parse_jog() {
    Command cmd;
    TEST_ASSERT_TRUE(parseCommand(
        R"({"cmd":"jog","pan":2.5,"tilt":-1.0})", cmd));
    TEST_ASSERT_EQUAL(CommandType::JOG, cmd.type);

    // Find pan and tilt indices
    for (int i = 0; i < g_axis_count; i++) {
        if (strcmp(g_axis_config[i].name, "pan") == 0) {
            TEST_ASSERT_FLOAT_WITHIN(0.01f, 2.5f, cmd.data.jog.axes[i]);
        }
        if (strcmp(g_axis_config[i].name, "tilt") == 0) {
            TEST_ASSERT_FLOAT_WITHIN(0.01f, -1.0f, cmd.data.jog.axes[i]);
        }
    }
}

void test_parse_jog_partial() {
    // Jog only one axis — others should be 0
    Command cmd;
    TEST_ASSERT_TRUE(parseCommand(
        R"({"cmd":"jog","slide":5.0})", cmd));
    TEST_ASSERT_EQUAL(CommandType::JOG, cmd.type);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 5.0f, cmd.data.jog.axes[AXIS_SLIDE]);

    // All other axes should be 0
    for (int i = 0; i < g_axis_count; i++) {
        if (i != AXIS_SLIDE) {
            TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, cmd.data.jog.axes[i]);
        }
    }
}

// --- Move To ---

void test_parse_move_to() {
    Command cmd;
    TEST_ASSERT_TRUE(parseCommand(
        R"({"cmd":"move_to","pan":45.0,"tilt":-10.0,"roll":0.0,"slide":500.0,"duration_ms":2000})",
        cmd));
    TEST_ASSERT_EQUAL(CommandType::MOVE_TO, cmd.type);
    TEST_ASSERT_EQUAL(2000, cmd.data.move_to.duration_ms);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 500.0f, cmd.data.move_to.axes[AXIS_SLIDE]);
}

void test_parse_move_to_default_duration() {
    Command cmd;
    TEST_ASSERT_TRUE(parseCommand(
        R"({"cmd":"move_to","slide":100.0})", cmd));
    TEST_ASSERT_EQUAL(1000, cmd.data.move_to.duration_ms);
}

// --- Home ---

void test_parse_home() {
    Command cmd;
    TEST_ASSERT_TRUE(parseCommand(
        R"({"cmd":"home","axes":["slide"]})", cmd));
    TEST_ASSERT_EQUAL(CommandType::HOME, cmd.type);
    TEST_ASSERT_TRUE(cmd.data.home.axis_mask & (1 << AXIS_SLIDE));
}

void test_parse_home_multiple_axes() {
    Command cmd;
    TEST_ASSERT_TRUE(parseCommand(
        R"({"cmd":"home","axes":["slide","pan"]})", cmd));
    TEST_ASSERT_TRUE(cmd.data.home.axis_mask & (1 << AXIS_SLIDE));

    int pan_idx = -1;
    for (int i = 0; i < g_axis_count; i++) {
        if (strcmp(g_axis_config[i].name, "pan") == 0) { pan_idx = i; break; }
    }
    if (pan_idx >= 0) {
        TEST_ASSERT_TRUE(cmd.data.home.axis_mask & (1 << pan_idx));
    }
}

// --- Scrub ---

void test_parse_scrub() {
    Command cmd;
    TEST_ASSERT_TRUE(parseCommand(
        R"({"cmd":"scrub","t":15000})", cmd));
    TEST_ASSERT_EQUAL(CommandType::SCRUB, cmd.type);
    TEST_ASSERT_EQUAL(15000, cmd.data.scrub.t_ms);
}

// --- Error cases ---

void test_parse_invalid_json() {
    Command cmd;
    TEST_ASSERT_FALSE(parseCommand("not json", cmd));
}

void test_parse_missing_cmd() {
    Command cmd;
    TEST_ASSERT_FALSE(parseCommand(R"({"foo":"bar"})", cmd));
}

void test_parse_unknown_cmd() {
    Command cmd;
    TEST_ASSERT_FALSE(parseCommand(R"({"cmd":"explode"})", cmd));
}

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_parse_play);
    RUN_TEST(test_parse_pause);
    RUN_TEST(test_parse_resume);
    RUN_TEST(test_parse_stop);
    RUN_TEST(test_parse_jog);
    RUN_TEST(test_parse_jog_partial);
    RUN_TEST(test_parse_move_to);
    RUN_TEST(test_parse_move_to_default_duration);
    RUN_TEST(test_parse_home);
    RUN_TEST(test_parse_home_multiple_axes);
    RUN_TEST(test_parse_scrub);
    RUN_TEST(test_parse_invalid_json);
    RUN_TEST(test_parse_missing_cmd);
    RUN_TEST(test_parse_unknown_cmd);

    return UNITY_END();
}
