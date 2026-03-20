# encoding:utf-8

import sys_infor
import sys
import os
# from ui_init import top_window_init
from PyQt5.QtWidgets import QApplication
from PyQt5.QtWidgets import QLabel

from ui import gimbal_control
from ui_init import gimbal_window_init
if __name__ == '__main__':

    app = QApplication(sys.argv)
    my_show = gimbal_window_init.Gimbal_Window_Init()
    my_show.Qlable_show = QLabel('APP初始化完成。。。')

    my_show.show()

    sys_infor.sys_set_infor('top_ui',my_show)

    sys_infor.sys_status_show_temporary('请连接设备')


    sys.exit(app.exec_())
