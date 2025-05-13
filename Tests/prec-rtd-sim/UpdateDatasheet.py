import datetime
from lecore.Doc.ModbusTable import *


if __name__ == "__main__":
    print(f"=== START === " + datetime.datetime.now().strftime("%y/%m/%d %H:%M:%S ==="))
    err = 0

    docx_update_datasheet("PrecRtdSim_Modbus.json", "../../Documents/PRecRtdSim-datasheet.docx")

    print(f"=== END === " + datetime.datetime.now().strftime("%y/%m/%d %H:%M:%S ==="))
    print('=== RESULT === {} === '.format('SUCCESS' if err == 0 else '{} ERRORS'.format(err)))
