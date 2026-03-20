# -*- coding: utf-8 -*-

from ui.gimbal_control import Ui_MainWindow

from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtCore import *
import time
import struct
import sys_infor
from protocol.connection.CANConnection import CANConnection
from protocol.sdk.CmdCombine import combine

class Gimbal_Window_Init(QMainWindow,Ui_MainWindow):
    def __init__(self):
        super(Gimbal_Window_Init, self).__init__()
        self.setupUi(self)
        self.device = {}
        self.check_data = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'A', 'B',
                           'C', 'D', 'E', 'F']

        self.init = False

        self.pitch_speed_value = 30.0
        self.roll_speed_value = 30.0
        self.yaw_speed_value = 30.0
        self.pitch_pos_value = 0.0
        self.roll_pos_value = 0.0
        self.yaw_pos_value = 0.0


        # 下面设置每一个控件的槽函数连接
        self.btn_connect.clicked.connect(self.device_open)

        self.setSpeed.clicked.connect(self.gimbal_setSpeed)
        self.setPos.clicked.connect(self.gimbal_setPos)

        self.btn_up.pressed.connect(self.gimbal_up_pressed)
        self.btn_up.released.connect(self.gimbal_up_released)
        self.btn_down.pressed.connect(self.gimbal_down_pressed)
        self.btn_down.released.connect(self.gimbal_down_released)
        self.btn_left.pressed.connect(self.gimbal_left_pressed)
        self.btn_left.released.connect(self.gimbal_left_released)
        self.btn_right.pressed.connect(self.gimbal_right_pressed)
        self.btn_right.released.connect(self.gimbal_right_released)

        self.btn_af.pressed.connect(self.gimbal_af_pressed)
        self.btn_af.released.connect(self.gimbal_af_released)
        self.btn_capture.clicked.connect(self.gimbal_picture)
        self.btn_startMovie.clicked.connect(self.gimbal_startMovie)
        self.btn_endMovie.clicked.connect(self.gimbal_endMovie)

        self.sp_speed_pitch.setValue(self.pitch_speed_value)
        self.sp_speed_roll.setValue(self.roll_speed_value)
        self.sp_speed_yaw.setValue(self.yaw_speed_value)
        self.sp_pos_pitch.setValue(self.pitch_pos_value)
        self.sp_pos_roll.setValue(self.roll_pos_value)
        self.sp_pos_yaw.setValue(self.yaw_pos_value)

        self.timer_timeout = 20 #50HZ定时器
        self.timer_up = QTimer(self)
        self.timer_down = QTimer(self)
        self.timer_left = QTimer(self)
        self.timer_right = QTimer(self)

        self.timer_up.timeout.connect(self.timer_up_cb)
        self.timer_down.timeout.connect(self.timer_down_cb)
        self.timer_left.timeout.connect(self.timer_left_cb)
        self.timer_right.timeout.connect(self.timer_right_cb)



    def upgrade_ui(self):
        self.sp_attitude_pitch.setValue(sys_infor.gimbal_control_attitude_pitch)
        self.sp_attitude_roll.setValue(sys_infor.gimbal_control_attitude_roll)
        self.sp_attitude_yaw.setValue(sys_infor.gimbal_control_attitude_yaw)
        self.sp_joint_pitch.setValue(sys_infor.gimbal_control_joint_pitch)
        self.sp_joint_roll.setValue(sys_infor.gimbal_control_joint_roll)
        self.sp_joint_yaw.setValue(sys_infor.gimbal_control_joint_yaw)

    def input_check(self,input_data):

        length = len(input_data)
        if length < 1 or length > 3:
            return -1

        for str in input_data:
            if str not in self.check_data:
                return -1
        return 0

    def device_open(self):

        print("Device open")

        # 获取输入内容并连接
        self.device["type"]    = "CAN"
        self.device["name"] = self.device_type.currentText()
        self.device["id"]   = self.device_id.currentText()
        self.device["send_id"] = "223"
        self.device["rev_id"]  = "222"

        print(self.device)


        try:
            self.device["connect"]  = CANConnection(int(self.device["send_id"],16),
                                                    int(self.device["rev_id"],16),
                                                    self.device["name"],
                                                    can_index = int(self.device["id"]))

        except sys_infor.CAN_OPEN_FAILED:
            sys_infor.sys_set_infor("connect_status", "failed")

        except sys_infor.CAN_CLEAN_FAILED:
            sys_infor.sys_set_infor("connect_status", "clear_failed")

        except sys_infor.CAN_START_FAILED:
            sys_infor.sys_set_infor("connect_status", "start_failed")

        else:
            sys_infor.sys_set_infor("connect_status", "success")

        # self.close()

        status = sys_infor.sys_get_infor("connect_status")

        if status == "success":
            sys_infor.sys_set_infor("device", self.device)
            sys_infor.sys_status_show_normal("设备信息：CAN,  TX: 0x{0}  RX:0x{1}".format(self.device["send_id"],self.device["rev_id"]))

        elif status == "failed":
            sys_infor.sys_status_show_normal("设备打开失败...")
        elif status == "clear_failed":
            sys_infor.sys_status_show_normal("设备清除buff失败...")
        elif status == "start_failed":
            sys_infor.sys_status_show_normal("设备启动失败...")


        self.label_connect.setText('设备已连接')
        sys_infor.sys_status_show_temporary('设备已连接')

        # 使能手持云台推送
        cmd = combine(cmd_type='03', cmd_set='0E', cmd_id='07', data='01')
        print('cmd-->{}'.format(cmd))
        self.device["connect"] .send_cmd(cmd)

        self.backedUpgrade = BackendUpgradeUI()
        self.backedUpgrade.upgrade_signal.connect(self.upgrade_ui)
        self.backedUpgrade.start()

        self.init = True

    def setSpeedControl(self, yaw, roll, pitch,ctrl_byte = 0x80):
        hex_data = struct.pack('<3hB',yaw,roll,pitch,ctrl_byte)
        pack_data = ['{:02X}'.format(i) for i in hex_data]
        cmd_data = ':'.join(pack_data)

        cmd = combine(cmd_type='03', cmd_set='0E', cmd_id='01', data=cmd_data)
        print('cmd---data {}'.format(cmd))
        self.device["connect"] .send_cmd(cmd)

    def setPosControl(self, yaw, roll, pitch,ctrl_byte=0x01,time_for_action=0x14):
        hex_data = struct.pack('<3h2B',yaw,roll,pitch,ctrl_byte,time_for_action)
        pack_data = ['{:02X}'.format(i) for i in hex_data]
        cmd_data = ':'.join(pack_data)

        cmd = combine(cmd_type='03', cmd_set='0E', cmd_id='00', data=cmd_data)
        print('cmd---data {}'.format(cmd))
        self.device["connect"].send_cmd(cmd)

    def cameraControl(self, ctrl_byte=0x00):
        hex_data = struct.pack('<B',ctrl_byte)
        pack_data = ['{:02X}'.format(i) for i in hex_data]
        cmd_data = ':'.join(pack_data)
        print('cmd---data {}'.format(cmd_data))

        cmd = combine(cmd_type='03', cmd_set='0D', cmd_id='00', data=cmd_data)
        self.device["connect"].send_cmd(cmd)

    def gimbal_setSpeed(self):

        if self.init:
            print('===>setSpeed')
            self.pitch_speed_value = self.sp_speed_pitch.value()
            self.roll_speed_value = self.sp_speed_roll.value()
            self.yaw_speed_value = self.sp_speed_yaw.value()


    def gimbal_setPos(self):
        if self.init:
            print('===>setPos')
            self.pitch_pos_value = self.sp_pos_pitch.value()
            self.roll_pos_value = self.sp_pos_roll.value()
            self.yaw_pos_value = self.sp_pos_yaw.value()

            self.setPosControl(int(self.yaw_pos_value * 10), int(self.roll_pos_value * 10), int(self.pitch_pos_value * 10))

    def gimbal_up_pressed(self):
        if self.init:
            print("===>up pressed")
            self.timer_up.start(self.timer_timeout)

    def gimbal_up_released(self):
        if self.init:
            print("===>up released")
            self.timer_up.stop()
            self.setSpeedControl(0, 0, 0)

    def gimbal_down_pressed(self):
        if self.init:
            print("===>down pressed")
            self.timer_down.start(self.timer_timeout)

    def gimbal_down_released(self):
        if self.init:
            print("===>down released")
            self.timer_down.stop()
            self.setSpeedControl(0, 0, 0)

    def gimbal_right_pressed(self):
        if self.init:
            print("===>right pressed")
            self.timer_right.start(self.timer_timeout)

    def gimbal_right_released(self):
        if self.init:
            print("===>right released")
            self.timer_right.stop()
            self.setSpeedControl(0, 0, 0)

    def gimbal_left_pressed(self):
        if self.init:
            print("===>left pressed")
            self.timer_left.start(self.timer_timeout)

    def gimbal_left_released(self):
        if self.init:
            print("===>left released")
            self.timer_left.stop()
            self.setSpeedControl(0, 0, 0)

    def gimbal_af_pressed(self):
        if self.init:
            self.cameraControl(ctrl_byte=0x05)

    def gimbal_af_released(self):
        if self.init:
            self.cameraControl(ctrl_byte=0x0B)

    def gimbal_picture(self):
        if self.init:
            self.cameraControl(ctrl_byte=0x01)

    def gimbal_startMovie(self):
        if self.init:
            self.cameraControl(ctrl_byte=0x03)

    def gimbal_endMovie(self):
        if self.init:
            self.cameraControl(ctrl_byte=0x04)

    def timer_up_cb(self):
        self.setSpeedControl(0, 0, int(self.pitch_speed_value * 10))

    def timer_down_cb(self):
        self.setSpeedControl(0, 0, int(-self.pitch_speed_value * 10))

    def timer_left_cb(self):
        self.setSpeedControl(int(-self.yaw_speed_value * 10), 0, 0)

    def timer_right_cb(self):
        self.setSpeedControl(int(self.yaw_speed_value * 10), 0, 0)

class BackendUpgradeUI(QThread):
    upgrade_signal = pyqtSignal()

    def run(self):
        while True:
            self.upgrade_signal.emit()
            time.sleep(1)