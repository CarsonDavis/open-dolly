#include <unity.h>
#include <cstring>
#include "trajectory.h"
#include "config.h"

void setUp(void) {
    // Reset trajectory state
    if (g_trajectory.points) {
        free(g_trajectory.points);
        g_trajectory.points = nullptr;
    }
    if (g_trajectory.events) {
        free(g_trajectory.events);
        g_trajectory.events = nullptr;
    }
    g_trajectory.point_count = 0;
    g_trajectory.event_count = 0;
    g_trajectory.duration_ms = 0;
    g_trajectory.loop = false;
}

void tearDown(void) {
    if (g_trajectory.points) {
        free(g_trajectory.points);
        g_trajectory.points = nullptr;
    }
    if (g_trajectory.events) {
        free(g_trajectory.events);
        g_trajectory.events = nullptr;
    }
}

// --- Parsing tests ---

void test_parse_valid_single_axis() {
    // slider-only style trajectory (but we test with all axes present since
    // native test uses the default 4-axis config)
    const char* json = R"({
        "points": [
            {"t": 0,    "slide": 0.0,   "pan": 0.0, "tilt": 0.0, "roll": 0.0},
            {"t": 1000, "slide": 100.0, "pan": 0.0, "tilt": 0.0, "roll": 0.0},
            {"t": 2000, "slide": 200.0, "pan": 0.0, "tilt": 0.0, "roll": 0.0}
        ],
        "loop": false
    })";

    TEST_ASSERT_TRUE(parseTrajectoryJson(json, strlen(json)));
    TEST_ASSERT_EQUAL(3, g_trajectory.point_count);
    TEST_ASSERT_EQUAL(2000, g_trajectory.duration_ms);
    TEST_ASSERT_FALSE(g_trajectory.loop);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, g_trajectory.points[0].axes[AXIS_SLIDE]);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, g_trajectory.points[1].axes[AXIS_SLIDE]);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 200.0f, g_trajectory.points[2].axes[AXIS_SLIDE]);
}

void test_parse_multi_axis() {
    const char* json = R"({
        "points": [
            {"t": 0,     "slide": 0.0,   "pan": 0.0,  "tilt": 0.0,  "roll": 0.0},
            {"t": 5000,  "slide": 250.0, "pan": 45.0, "tilt": -10.0, "roll": 0.0},
            {"t": 10000, "slide": 500.0, "pan": 90.0, "tilt": 0.0,  "roll": 5.0}
        ],
        "loop": false
    })";

    TEST_ASSERT_TRUE(parseTrajectoryJson(json, strlen(json)));
    TEST_ASSERT_EQUAL(3, g_trajectory.point_count);
    TEST_ASSERT_EQUAL(10000, g_trajectory.duration_ms);

    // Check pan axis value at point 1
    int pan_idx = -1;
    for (int i = 0; i < g_axis_count; i++) {
        if (strcmp(g_axis_config[i].name, "pan") == 0) { pan_idx = i; break; }
    }
    TEST_ASSERT_NOT_EQUAL(-1, pan_idx);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 45.0f, g_trajectory.points[1].axes[pan_idx]);
}

void test_parse_with_loop() {
    const char* json = R"({
        "points": [
            {"t": 0,    "slide": 0.0, "pan": 0.0, "tilt": 0.0, "roll": 0.0},
            {"t": 1000, "slide": 100.0, "pan": 0.0, "tilt": 0.0, "roll": 0.0}
        ],
        "loop": true
    })";

    TEST_ASSERT_TRUE(parseTrajectoryJson(json, strlen(json)));
    TEST_ASSERT_TRUE(g_trajectory.loop);
}

void test_parse_with_events() {
    const char* json = R"({
        "points": [
            {"t": 0,     "slide": 0.0, "pan": 0.0, "tilt": 0.0, "roll": 0.0},
            {"t": 10000, "slide": 500.0, "pan": 0.0, "tilt": 0.0, "roll": 0.0}
        ],
        "events": [
            {"t": 5000, "type": "shutter"},
            {"t": 8000, "type": "record_start"}
        ],
        "loop": false
    })";

    TEST_ASSERT_TRUE(parseTrajectoryJson(json, strlen(json)));
    TEST_ASSERT_EQUAL(2, g_trajectory.event_count);
    TEST_ASSERT_EQUAL(5000, g_trajectory.events[0].t_ms);
    TEST_ASSERT_EQUAL(0, g_trajectory.events[0].type); // shutter = 0
    TEST_ASSERT_EQUAL(8000, g_trajectory.events[1].t_ms);
    TEST_ASSERT_EQUAL(1, g_trajectory.events[1].type); // record_start = 1
}

// --- Validation failure tests ---

void test_parse_non_ascending_timestamps() {
    const char* json = R"({
        "points": [
            {"t": 1000, "slide": 100.0, "pan": 0.0, "tilt": 0.0, "roll": 0.0},
            {"t": 500,  "slide": 50.0,  "pan": 0.0, "tilt": 0.0, "roll": 0.0}
        ],
        "loop": false
    })";
    TEST_ASSERT_FALSE(parseTrajectoryJson(json, strlen(json)));
}

void test_parse_empty_points() {
    const char* json = R"({"points": [], "loop": false})";
    TEST_ASSERT_FALSE(parseTrajectoryJson(json, strlen(json)));
}

void test_parse_no_points_field() {
    const char* json = R"({"loop": false})";
    TEST_ASSERT_FALSE(parseTrajectoryJson(json, strlen(json)));
}

void test_parse_invalid_json() {
    const char* json = "not json at all";
    TEST_ASSERT_FALSE(parseTrajectoryJson(json, strlen(json)));
}

void test_parse_out_of_range_value() {
    // slide max is 1000.0 in default config
    const char* json = R"({
        "points": [
            {"t": 0,    "slide": 0.0,    "pan": 0.0, "tilt": 0.0, "roll": 0.0},
            {"t": 1000, "slide": 9999.0, "pan": 0.0, "tilt": 0.0, "roll": 0.0}
        ],
        "loop": false
    })";
    TEST_ASSERT_FALSE(parseTrajectoryJson(json, strlen(json)));
}

// --- Interpolation tests ---

void test_interpolate_midpoint() {
    // Manually set up a simple 2-point trajectory
    g_trajectory.point_count = 2;
    g_trajectory.points = (TrajectoryPoint*)malloc(2 * sizeof(TrajectoryPoint));

    g_trajectory.points[0].t_ms = 0;
    memset(g_trajectory.points[0].axes, 0, sizeof(float) * MAX_AXES);
    g_trajectory.points[0].axes[AXIS_SLIDE] = 0.0f;

    g_trajectory.points[1].t_ms = 1000;
    memset(g_trajectory.points[1].axes, 0, sizeof(float) * MAX_AXES);
    g_trajectory.points[1].axes[AXIS_SLIDE] = 100.0f;

    float result[MAX_AXES];
    interpolateAtTime(500, result);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 50.0f, result[AXIS_SLIDE]);
}

void test_interpolate_at_start() {
    g_trajectory.point_count = 2;
    g_trajectory.points = (TrajectoryPoint*)malloc(2 * sizeof(TrajectoryPoint));

    g_trajectory.points[0].t_ms = 0;
    memset(g_trajectory.points[0].axes, 0, sizeof(float) * MAX_AXES);
    g_trajectory.points[0].axes[AXIS_SLIDE] = 10.0f;

    g_trajectory.points[1].t_ms = 1000;
    memset(g_trajectory.points[1].axes, 0, sizeof(float) * MAX_AXES);
    g_trajectory.points[1].axes[AXIS_SLIDE] = 110.0f;

    float result[MAX_AXES];
    interpolateAtTime(0, result);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 10.0f, result[AXIS_SLIDE]);
}

void test_interpolate_at_end() {
    g_trajectory.point_count = 2;
    g_trajectory.points = (TrajectoryPoint*)malloc(2 * sizeof(TrajectoryPoint));

    g_trajectory.points[0].t_ms = 0;
    memset(g_trajectory.points[0].axes, 0, sizeof(float) * MAX_AXES);

    g_trajectory.points[1].t_ms = 1000;
    memset(g_trajectory.points[1].axes, 0, sizeof(float) * MAX_AXES);
    g_trajectory.points[1].axes[AXIS_SLIDE] = 100.0f;

    float result[MAX_AXES];
    interpolateAtTime(1000, result);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, result[AXIS_SLIDE]);
}

void test_interpolate_three_segments() {
    g_trajectory.point_count = 4;
    g_trajectory.points = (TrajectoryPoint*)malloc(4 * sizeof(TrajectoryPoint));

    for (int i = 0; i < 4; i++) {
        g_trajectory.points[i].t_ms = i * 1000;
        memset(g_trajectory.points[i].axes, 0, sizeof(float) * MAX_AXES);
        g_trajectory.points[i].axes[AXIS_SLIDE] = (float)(i * 100);
    }

    float result[MAX_AXES];

    // In second segment (t=1500, between points 1 and 2)
    interpolateAtTime(1500, result);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 150.0f, result[AXIS_SLIDE]);

    // In third segment (t=2500, between points 2 and 3)
    interpolateAtTime(2500, result);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 250.0f, result[AXIS_SLIDE]);
}

int main() {
    UNITY_BEGIN();

    // Parsing
    RUN_TEST(test_parse_valid_single_axis);
    RUN_TEST(test_parse_multi_axis);
    RUN_TEST(test_parse_with_loop);
    RUN_TEST(test_parse_with_events);

    // Validation failures
    RUN_TEST(test_parse_non_ascending_timestamps);
    RUN_TEST(test_parse_empty_points);
    RUN_TEST(test_parse_no_points_field);
    RUN_TEST(test_parse_invalid_json);
    RUN_TEST(test_parse_out_of_range_value);

    // Interpolation
    RUN_TEST(test_interpolate_midpoint);
    RUN_TEST(test_interpolate_at_start);
    RUN_TEST(test_interpolate_at_end);
    RUN_TEST(test_interpolate_three_segments);

    return UNITY_END();
}
