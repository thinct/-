import shutil
import tkinter.messagebox
import os  

try:
    while True:    
        if 'Calibration_HandEye' in os.popen('tasklist /FI "IMAGENAME eq Calibration_HandEye.exe"').read():
            os.system('TASKKILL /F /IM Calibration_HandEye.exe')
        else:
            break;
        
    path = shutil.copy(r'D:\WorkCode\QtTempCodes\Calibration_HandEye\release\Calibration_HandEye.exe'\
    , r'D:\Others\DengJin\SMTMainWindow_20190828_x64\Calibration_HandEye.exe')
    
    os.system(r'D:\Others\DengJin\SMTMainWindow_20190828_x64\Calibration_HandEye.exe')
    
except:
    tkinter.Tk().withdraw();
    tkinter.messagebox.showerror('错误','出错了')
    
    
exit()