#include <unity.h>
#include "state_machine.h"

void setUp(void) {
    g_state.current_state = State::IDLE;
    g_state.error_code = ErrorCode::NONE;
}

void tearDown(void) {}

// --- Valid transitions ---

void test_idle_to_playing() {
    TEST_ASSERT_TRUE(tryTransition(State::IDLE, State::PLAYING));
}

void test_idle_to_homing() {
    TEST_ASSERT_TRUE(tryTransition(State::IDLE, State::HOMING));
}

void test_idle_to_moving() {
    TEST_ASSERT_TRUE(tryTransition(State::IDLE, State::MOVING));
}

void test_playing_to_paused() {
    TEST_ASSERT_TRUE(tryTransition(State::PLAYING, State::PAUSED));
}

void test_playing_to_idle() {
    TEST_ASSERT_TRUE(tryTransition(State::PLAYING, State::IDLE));
}

void test_paused_to_playing() {
    TEST_ASSERT_TRUE(tryTransition(State::PAUSED, State::PLAYING));
}

void test_paused_to_idle() {
    TEST_ASSERT_TRUE(tryTransition(State::PAUSED, State::IDLE));
}

void test_homing_to_idle() {
    TEST_ASSERT_TRUE(tryTransition(State::HOMING, State::IDLE));
}

void test_moving_to_idle() {
    TEST_ASSERT_TRUE(tryTransition(State::MOVING, State::IDLE));
}

void test_moving_to_moving() {
    TEST_ASSERT_TRUE(tryTransition(State::MOVING, State::MOVING));
}

void test_error_to_idle() {
    TEST_ASSERT_TRUE(tryTransition(State::ERROR, State::IDLE));
}

// Any state to ERROR
void test_any_to_error() {
    TEST_ASSERT_TRUE(tryTransition(State::IDLE, State::ERROR));
    TEST_ASSERT_TRUE(tryTransition(State::PLAYING, State::ERROR));
    TEST_ASSERT_TRUE(tryTransition(State::PAUSED, State::ERROR));
    TEST_ASSERT_TRUE(tryTransition(State::HOMING, State::ERROR));
    TEST_ASSERT_TRUE(tryTransition(State::MOVING, State::ERROR));
}

// --- Invalid transitions ---

void test_idle_to_paused_invalid() {
    TEST_ASSERT_FALSE(tryTransition(State::IDLE, State::PAUSED));
}

void test_idle_to_idle_invalid() {
    TEST_ASSERT_FALSE(tryTransition(State::IDLE, State::IDLE));
}

void test_playing_to_homing_invalid() {
    TEST_ASSERT_FALSE(tryTransition(State::PLAYING, State::HOMING));
}

void test_playing_to_moving_invalid() {
    TEST_ASSERT_FALSE(tryTransition(State::PLAYING, State::MOVING));
}

void test_paused_to_homing_invalid() {
    TEST_ASSERT_FALSE(tryTransition(State::PAUSED, State::HOMING));
}

void test_paused_to_moving_invalid() {
    TEST_ASSERT_FALSE(tryTransition(State::PAUSED, State::MOVING));
}

void test_homing_to_playing_invalid() {
    TEST_ASSERT_FALSE(tryTransition(State::HOMING, State::PLAYING));
}

void test_error_to_playing_invalid() {
    TEST_ASSERT_FALSE(tryTransition(State::ERROR, State::PLAYING));
}

void test_error_to_error_invalid() {
    TEST_ASSERT_FALSE(tryTransition(State::ERROR, State::ERROR));
}

// --- changeState integration ---

void test_change_state_valid() {
    g_state.current_state = State::IDLE;
    TEST_ASSERT_TRUE(changeState(State::PLAYING));
    TEST_ASSERT_EQUAL(State::PLAYING, g_state.current_state);
}

void test_change_state_invalid() {
    g_state.current_state = State::ERROR;
    TEST_ASSERT_FALSE(changeState(State::PLAYING));
    TEST_ASSERT_EQUAL(State::ERROR, g_state.current_state);
}

void test_change_state_to_error_sets_code() {
    g_state.current_state = State::PLAYING;
    TEST_ASSERT_TRUE(changeState(State::ERROR, ErrorCode::LIMIT_HIT));
    TEST_ASSERT_EQUAL(State::ERROR, g_state.current_state);
    TEST_ASSERT_EQUAL(ErrorCode::LIMIT_HIT, g_state.error_code);
}

void test_change_state_clear_error() {
    g_state.current_state = State::ERROR;
    g_state.error_code = ErrorCode::MOTOR_STALL;
    TEST_ASSERT_TRUE(changeState(State::IDLE));
    TEST_ASSERT_EQUAL(State::IDLE, g_state.current_state);
    TEST_ASSERT_EQUAL(ErrorCode::NONE, g_state.error_code);
}

int main() {
    UNITY_BEGIN();

    // Valid transitions
    RUN_TEST(test_idle_to_playing);
    RUN_TEST(test_idle_to_homing);
    RUN_TEST(test_idle_to_moving);
    RUN_TEST(test_playing_to_paused);
    RUN_TEST(test_playing_to_idle);
    RUN_TEST(test_paused_to_playing);
    RUN_TEST(test_paused_to_idle);
    RUN_TEST(test_homing_to_idle);
    RUN_TEST(test_moving_to_idle);
    RUN_TEST(test_moving_to_moving);
    RUN_TEST(test_error_to_idle);
    RUN_TEST(test_any_to_error);

    // Invalid transitions
    RUN_TEST(test_idle_to_paused_invalid);
    RUN_TEST(test_idle_to_idle_invalid);
    RUN_TEST(test_playing_to_homing_invalid);
    RUN_TEST(test_playing_to_moving_invalid);
    RUN_TEST(test_paused_to_homing_invalid);
    RUN_TEST(test_paused_to_moving_invalid);
    RUN_TEST(test_homing_to_playing_invalid);
    RUN_TEST(test_error_to_playing_invalid);
    RUN_TEST(test_error_to_error_invalid);

    // Integration
    RUN_TEST(test_change_state_valid);
    RUN_TEST(test_change_state_invalid);
    RUN_TEST(test_change_state_to_error_sets_code);
    RUN_TEST(test_change_state_clear_error);

    return UNITY_END();
}
