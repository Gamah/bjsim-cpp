from PyQt5.QtWidgets import QMainWindow, QApplication, QWidget, QAction, QTableWidget,QTableWidgetItem,QVBoxLayout
from PyQt5.QtGui import QIcon
from PyQt5.QtCore import pyqtSlot
import json
import sys

with open('out.json') as f:
    data = json.load(f)

data = data['Alice']
print(data)

class TableView(QTableWidget):
    def __init__(self, data, *args):
        QTableWidget.__init__(self, *args)
        self.data = data
        self.setData()
        self.resizeColumnsToContents()
        self.resizeRowsToContents()
 
    def setData(self): 
        horHeaders = []
        for n, key in enumerate(sorted(self.data.keys())):
            horHeaders.append(key)
            for m, item in enumerate(self.data[key]):
                print(m, item)
                newitem = QTableWidgetItem(m)
                self.setItem(m, n, key)
        self.setHorizontalHeaderLabels(horHeaders)
 
def main(args):
    app = QApplication(args)
    table = TableView(data, 15, 10)
    table.show()
    sys.exit(app.exec_())
 
if __name__=="__main__":
    main(sys.argv)
