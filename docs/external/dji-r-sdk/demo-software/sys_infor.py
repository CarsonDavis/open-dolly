# encoding:utf-8

from PyQt5.QtWidgets import QApplication
# 存储所有信息的字典
sys_infor = {}
gimbal_control_attitude_pitch = 0
gimbal_control_attitude_roll = 0
gimbal_control_attitude_yaw = 0
gimbal_control_joint_pitch = 0
gimbal_control_joint_roll = 0
gimbal_control_joint_yaw = 0
# 设置和获取系统信息
def sys_set_infor(key,value):
    global sys_infor
    sys_infor[key] = value

def sys_get_infor(key):
    global sys_infor
    if key in sys_infor.keys():
        return sys_infor[key]
    else:
        return False

#状态栏显示 (正常显示)
def sys_status_show_normal(str):
    global sys_infor
    if 'top_ui' not in sys_infor.keys():
        return -1

    top_ui = sys_infor['top_ui']

    top_ui.Qlable_show.clear()
    top_ui.Qlable_show.setText(str)
    top_ui.statusBar().addPermanentWidget(top_ui.Qlable_show)

# 状态栏显示 (临时显示)
def sys_status_show_temporary(str):
    global sys_infor
    if 'top_ui' not in sys_infor.keys():
        return -1
    top_ui = sys_infor['top_ui']
    top_ui.statusBar().showMessage(str)
    return 0

# 设置信息输出地址
def sys_infor_out_set(Qtextbrowser):
    global sys_infor
    sys_infor['text_out'] = Qtextbrowser

# 信息输出 (显示到输出信息框中)
def sys_infor_out(str):
    global sys_infor
    if 'text_out' not in sys_infor.keys():
        return -1
    text_out = sys_infor['text_out']
    text_out.append(str)
    QApplication.processEvents()

# 将收到的应答包由 列表转为 字符串 方便输出
def sys_infor_answer_list_str(data):
    answer_str = ""
    for str in data:
        answer_str += str + " "
    return answer_str

#UART 底层异常类型
class UART_OPEN_TIMEOUT(Exception):    #串口打开超时
    pass
class UART_OPEN_FAILED(Exception):   #串口打开失败
    pass
class UART_READ_FAILED(Exception):   #串口读取数据失败
    pass

#CAN 底层异常类型
class CAN_OPEN_FAILED(Exception):   #CAN打开失败
    pass
class CAN_CLEAN_FAILED(Exception): #CAN清空buff失败
    pass
class CAN_START_FAILED(Exception): #CAN启动失败
    pass
class CAN_READ_FAILED(Exception):   #CAN读取数据失败
    pass


# 底层公共异常
class ANSWER_TIMEOUT(Exception): #应答超时
    pass
class WRITE_FAILED(Exception): #发送失败
    pass