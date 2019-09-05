import shutil
import tkinter.messagebox

try:
    path = shutil.copy(r'D:\WorkCode\QtTempCodes\Calibration_HandEye\release\Calibration_HandEye.exe'\
    , r'D:\Others\DengJin\SMTMainWindow_20190828_x64\Calibration_HandEye.exe')
except:
    tkinter.Tk().withdraw();
    tkinter.messagebox.showerror('错误','出错了')