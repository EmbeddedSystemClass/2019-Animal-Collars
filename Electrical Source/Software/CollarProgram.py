##############################################
#CollarProgram.py
#User Program to set up and control the 2019
#animal tracking collar system
#Commands and other information can be found in
#the associated Technical user manual
##############################################
import tkinter as tk
from tkinter import ttk
from tkinter import scrolledtext
import datetime
import math

import time
import os
import serial
import re

import sys
import glob

#Defines
LARGE_FONT = ("Verdana", 12)
SMALL_FONT = ("Courier New", 10)


# App container
#=======================================================================================================================
class CollarProgram(tk.Tk):
    def __init__(self, *args, **kwargs):
        tk.Tk.__init__(self, *args, **kwargs)

        tk.Tk.iconbitmap(self, default="GV_icon.ico")
        tk.Tk.wm_title(self, "2019 Wildlife Tracking Collar Interface")

        container = tk.Frame(self)
        container.pack(side='top', fill='both', expand=True)
        container.grid_rowconfigure(0, weight=1)
        container.grid_columnconfigure(0, weight=1)

        self.frames = {}

        for F in (StartPage, PageProgram, PageDataRx, PagePowerCalc):
            frame = F(container, self)
            self.frames[F] = frame
            frame.grid(row=0, column=0, sticky="nsew")

        self.show_frame(StartPage)

    def show_frame(self, cont):
        frame = self.frames[cont]
        frame.tkraise()
#=======================================================================================================================
class StartPage(tk.Frame):
    def __init__(self, parent, controller):
        tk.Frame.__init__(self, parent)
        label = ttk.Label(self, text="Main Menu", font=LARGE_FONT)
        label.grid(row=0, column=2)

        button1 = ttk.Button(self, text="Direct Collar Connection",
                             command=lambda: controller.show_frame(PageProgram))
        button1.grid(row=1, column=0)
        button2 = ttk.Button(self, text="Wireless Collar Connection",
                             command=lambda: controller.show_frame(PageDataRx))
        button2.grid(row=1, column=2)
        button3 = ttk.Button(self, text="Power Calculator", command=self.ShowPowerCalc)
                             #command=lambda: controller.show_frame(PagePowerCalc))

        button3.grid(row=1, column=4)
        button3 = ttk.Button(self, text="Help", command=self.ShowHelp)
        # command=lambda: controller.show_frame(PagePowerCalc))

        button3.grid(row=1, column=5)

    def ShowPowerCalc(self):
        #os.system("start " + PowerCalc.xlsx)
        #subprocess.run(['open', PowerCalc], check=True)
        os.startfile("PowerCalc.xlsx")
        return
    def ShowHelp(self):
        #os.system("start " + PowerCalc.xlsx)
        #subprocess.run(['open', PowerCalc], check=True)
        os.startfile("FieldManual.pdf")
        return
#=======================================================================================================================
class PageProgram(tk.Frame):
    # =====================================
    def __init__(self, parent, controller):
        tk.Frame.__init__(self, parent)
        tk.Frame.__init__(self, parent)

        # Labels
        ttk.Label(self, text="Program Device", font=LARGE_FONT).grid(row=0, column=0)
        ttk.Label(self, text="COM Port", font=LARGE_FONT).grid(row=1, column=0)
        ttk.Label(self, text="Serial Num", font=LARGE_FONT).grid(row=3, column=0)
        ttk.Label(self, text="Dongle SN", font=LARGE_FONT).grid(row=5, column=0)
        ttk.Label(self, text="Messages", font=LARGE_FONT).grid(row=14, column=0)

        ttk.Label(self, text="Fixes Per Day", font=LARGE_FONT).grid(row=0, column=3)
        ttk.Label(self, text="Hours on (24Hr time UTC)", font=LARGE_FONT).grid(row=2, column=3)
        #ttk.Label(self, text="Fix Accuracy (meters)", font=LARGE_FONT).grid(row=2, column=4)
        ttk.Label(self, text="Days on (M,W,R...)", font=LARGE_FONT).grid(row=7, column=3)

        ttk.Label(self, text="Time Window Start", font=SMALL_FONT).grid(row=3, column=3)
        ttk.Label(self, text="Time Window End", font=SMALL_FONT).grid(row=5, column=3)

        # Buttons
        button1 = ttk.Button(self, text="Connect To Collar",
                             command=lambda: self.NewComConnection())
        button1.grid(row=2, column=1)
        button2 = ttk.Button(self, text="Get SN Of Collar",
                             command=self.GetSN)
        button2.grid(row=4, column=1)
        button3 = ttk.Button(self, text="Home",
                             command=lambda: controller.show_frame(StartPage))
        button3.grid(row=0, column=4)

        button5 = ttk.Button(self, text="Program Collar With Selected Settings",
                             command=self.ProgramDevice)
        button5.grid(row=6, column=4)
        button8 = ttk.Button(self, text="Program Collar as Small Collar", command=self.ProgramSmallDevice)
        button8.grid(row=7, column=4)

        #button6 = ttk.Button(self, text="Get Configuration On Device",
        #                     command=lambda: self.GetConfig())
        #button6.grid(row=5, column=4)
        button7 = tk.Button(self, text="Clear Collar Storage", bg="red",
                             command=self.MemInitialize)
        button7.grid(row=15, column=4)
        button7 = ttk.Button(self, text="Download Data From Collar",
                             command=self.DownloadFixes)
        button7.grid(row=11, column=4)

        # Entries
        self.comPortEntry = tk.Entry(self)
        self.snEntry = tk.Entry(self)
        self.rxEntry = tk.Entry(self)

        self.comPortEntry.grid(row=2, column=0)
        self.snEntry.grid(row=4, column=0)
        self.rxEntry.grid(row=6, column=0)

        self.snEntry.insert(0, '')
        self.snEntry.configure(state='readonly')

        self.fixAmt = ttk.Spinbox(self, from_=2.0, to=144.0, increment=1.0, wrap=True, state='readonly')
        self.fixAmt.grid(row=1, column=3)
        self.fixAmt.set(6)

        #self.fixAcc = ttk.Spinbox(self, from_=15.0, to=100.0, increment=5, wrap=True)
        #self.fixAcc.grid(row=4, column=4)
        #self.fixAcc.set(50)

        self.vhfSrt = ttk.Spinbox(self, from_=0.0, to=24.0, increment=1.0, wrap=True, state='readonly')
        self.vhfSrt.grid(row=4, column=3)
        self.vhfSrt.set(10)
        self.vhfStp = ttk.Spinbox(self, from_=0.0, to=24.0, increment=1.0, wrap=True, state='readonly')
        self.vhfStp.grid(row=6, column=3)
        self.vhfStp.set(14)

        self.sunB = tk.IntVar()
        tk.Checkbutton(self, text="S", variable=self.sunB, font=SMALL_FONT).grid(row=8, column=3)

        self.monB = tk.IntVar()
        tk.Checkbutton(self, text="M", variable=self.monB, font=SMALL_FONT).grid(row=9, column=3)

        self.teusB = tk.IntVar()
        tk.Checkbutton(self, text="T", variable=self.teusB, font=SMALL_FONT).grid(row=10, column=3)

        self.wedB = tk.IntVar()
        tk.Checkbutton(self, text="W", variable=self.wedB, font=SMALL_FONT).grid(row=11, column=3)

        self.thurB = tk.IntVar()
        tk.Checkbutton(self, text="R", variable=self.thurB, font=SMALL_FONT).grid(row=12, column=3)

        self.friB = tk.IntVar()
        tk.Checkbutton(self, text="F", variable=self.friB, font=SMALL_FONT).grid(row=13, column=3)

        self.satB = tk.IntVar()
        tk.Checkbutton(self, text="S", variable=self.satB, font=SMALL_FONT).grid(row=14, column=3)

        self.sunB.set(0)
        self.monB.set(0)
        self.teusB.set(0)
        self.wedB.set(0)
        self.thurB.set(1)
        self.friB.set(0)
        self.satB.set(0)



        self.fixE = tk.Entry(self)
        # self.fixE.grid(row=1, column=3)

        self.hourE = tk.Entry(self)
        # self.hourE.grid(row=3, column=3)

        self.dayE = tk.Entry(self)
        # self.dayE.grid(row=5, column=3)

        # Text Box
        self.msgBox = scrolledtext.ScrolledText(self, height=10, width=40)
        self.msgBox.grid(row=15, column=0)

        # Serial Port
        self.ser = serial.Serial(
            port=None,
            # port='\\\\.\\COM3',
            baudrate=9600,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS
        )

        self.txData = b""
    def DownloadFixes(self):

        if self.ser.isOpen():
            self.msgBox.insert(tk.END, "Downloading Available Data\n")
            self.msgBox.see("end")
        else:
            self.msgBox.insert(tk.END, "Please connect to COMPORT first\n")
            self.msgBox.see("end")
            return

        # Ask collar for data
        self.msgBox.insert(tk.END, "Requesting Data\n")
        self.msgBox.see("end")
        self.update()

        self.ser.write(b'DTA?')

        # self.ser.timeout = 1
        tmpString = bytearray(b'')
        response = []

        self.ser.timeout = 10

        tmpString = self.ser.read_until(b'?')

        #if tmpString[-1] != b'?':
        #    self.msgBox.insert(tk.END, "Data error, try again\n")
        #    self.msgBox.see("end")
        #    self.update()
        #    return


        tmpString = tmpString[:-1].decode()

        time.sleep(1)

        self.msgBox.insert(tk.END, "Data received, getting SN from collar\n")
        self.msgBox.see("end")
        self.update()

        self.GetSN()

        now = datetime.datetime.now()

        path = self.snEntry.get() + "_" + now.strftime('%m_%d_%y_%H_%M') + ".txt"

        f = open(path, "w+")

        f.write("Hour,Min,Day,Month,Year,Northing Deg,Northing Min,Direction,Easting Deg,Easting Min,Direction\n")
        f.write(tmpString)
        f.close()

        pre, ext = os.path.splitext(path)
        os.rename(path, pre + ".csv")

        self.msgBox.insert(tk.END, "Data saved to: " + pre + ".csv\n")
        self.msgBox.insert(tk.END, "Download Done\n Collar Disconnected\n")
        self.msgBox.see("end")
        self.update()

    def MemInitialize(self):
        serialData = "ERS?"
        if self.ser.isOpen():
            self.ser.write(str.encode(serialData))
            self.msgBox.insert(tk.END, "Wiping Device\n")
            self.msgBox.see("end")
        else:
            self.msgBox.insert(tk.END, "Please connect to COMPORT first\n")
            self.msgBox.see("end")
            return
        self.ser.write(str.encode(serialData))
        self.msgBox.insert(tk.END, "Device Storage Deleted\n")
        self.msgBox.see("end")
        self.update()

    # =====================================
    def FormatProgramString(self):
        # print(self.fixAmt.get() + self.vhfSrt.get() + self.vhfStp.get())

        temp = int(self.fixAmt.get())
        temp = (1 / (temp / 24))
        mins, hours = math.modf(temp)

        # Convert fixes per day to time between fix
        mins = mins * 60
        mins = int(math.ceil(mins / 1.0)) * 1
        if (mins >= 60.0):
            mins = 0
            hours = hours + 1

        hours = int(hours)
        mins = int(mins)

        startHr = int(self.vhfSrt.get())
        endHr = int(self.vhfStp.get())

        day = int(0)

        day = day + (self.monB.get() << 0) + (self.teusB.get() << 1)
        day = day + (self.wedB.get() << 2) + (self.thurB.get() << 3)
        day = day + (self.friB.get() << 4) + (self.satB.get() << 5)
        day = day + (self.sunB.get() << 6)

        self.txData = '%0*d' % (2, hours) + '%0*d' % (2, mins) + '%0*d' % (2, startHr) + '%0*d' % (2, endHr)
        self.txData = str.encode(self.txData + chr(day))

    # Function to connect program to collar  comport
    # =====================================
    def NewComConnection(self):
        # ser = serial.Serial(
        #
        #     port = '\\\\.\\COM' + self.comPortEntry.get(),
        #     #port='\\\\.\\COM3',
        #     baudrate=9600,
        #     parity=serial.PARITY_NONE,
        #     stopbits=serial.STOPBITS_ONE,
        #     bytesize=serial.EIGHTBITS
        # )
        self.ser.port = '\\\\.\\COM' + self.comPortEntry.get()

        # global serial_object
        # serial_object = self.ser.port
        # Connect()

        tmpString = ''




        #Clear SN entry
        self.snEntry.configure(state="normal")
        self.snEntry.delete(0, 'end')
        self.snEntry.configure(state='readonly')
        # Check to see if port is open or if port exists
        if self.ser.isOpen():
            try:
                self.ser.close()
            except IOError:
                self.msgBox.insert(tk.END, "Could not close port: " + self.comPortEntry.get() + '\n')
                self.msgBox.see("end")
                return
            self.msgBox.insert(tk.END, "Comport was open, closing now\n")

        # Attempt to open port with current params
        try:
            self.ser.open()
        except IOError:
            self.msgBox.insert(tk.END, "Could not open port: " + self.comPortEntry.get() + '\n')
            self.msgBox.insert(tk.END, "List of Ports ready to connect:\n" )
            self.msgBox.insert(tk.END, str(serial_ports()) + '\n')
            self.msgBox.see("end")

            #print(serial_ports())

            return
        if self.ser.isOpen():
            self.msgBox.insert(tk.END, "Comport " + self.comPortEntry.get() + " opened successfully.\n")
            self.msgBox.insert(tk.END, "Attempting device handshake\n")
            self.msgBox.see("end")


        # Flush port before use
        self.ser.flushInput()
        self.ser.flushOutput()
        self.update()

        # Send ACK to device
        self.ser.write(str.encode("ACK?"))
        self.msgBox.insert(tk.END, "Sent\n")
        self.msgBox.see("end")
        self.update()

        # Read for response from device with timeout
        self.ser.timeout = 1
        tmpString = self.ser.read(4)

        if not tmpString:  # No response
            self.msgBox.insert(tk.END, "No response from device\n")
            self.msgBox.see("end")
            return

        elif tmpString != b'ACK?':
            self.msgBox.insert(tk.END, "Incorrect response from device\n")
            self.msgBox.see("end")
            return

        else:
            self.msgBox.insert(tk.END, "Handshake completed successfully\n")
            self.msgBox.see("end")

        now = datetime.datetime.now()

    # =====================================
    def ProgramDevice(self):

        if int(self.vhfSrt.get()) >= int(self.vhfStp.get()):
            self.msgBox.insert(tk.END, "Please Enter Valid VHF Time\n")
            self.msgBox.see("end")
            return

        if len(self.rxEntry.get()) == 0:
            self.msgBox.insert(tk.END, "Please enter valid dongle SN\n")
            self.msgBox.see("end")
            return






        # Set up the XBee device
        # must be done first due to timeout constraints
        # sending CFG command with the dongle sn
        serialData = "CFG?" + self.rxEntry.get() + "?"
        if self.ser.isOpen():
            self.ser.write(str.encode(serialData))
            self.msgBox.insert(tk.END, "Configuring XBee\n")
            self.msgBox.see("end")
        else:
            self.msgBox.insert(tk.END, "Please connect to COMPORT first\n")
            self.msgBox.see("end")
            return

        self.update()


        # Waits for response string from the collar.
        self.ser.timeout = 3;
        tmpStr = self.ser.read(50)  # Should timeout
        tmpStr = tmpStr.decode()
        self.ser.timeout = 0;

        # Using regex to remove O and K from the return string
        myex = re.compile(r"(O|K)")
        tmpStr = re.sub(myex, '', tmpStr)


        # Compares the input dongle SN with the configured one
        if tmpStr != self.rxEntry.get():
            self.msgBox.insert(tk.END, "XBee configuration ERROR, set target to " + tmpStr + "\n")
            self.msgBox.see("end")
            return
        else:
            self.msgBox.insert(tk.END, "XBee set to " + tmpStr + "\n")
            self.msgBox.see("end")

        self.update()

        time.sleep(1)  # Ensure Xbee timeout

        # Get Device SN
        self.GetSN()

        # Set time on RTC first
        now = datetime.datetime.now()
        print(now)

        # Set up filepointer
        self.msgBox.insert(tk.END, "Creating Config File " + "\n")
        self.msgBox.see("end")
        self.update()
        f = open(self.snEntry.get() + ".txt", "w+")

        # File header
        f.write("Collar configuration file for SN: " + self.snEntry.get() + "\n\n")
        f.write("Collar configured at " + str(now) + "\n")

        f.write("Dongle Target SN: " + tmpStr + "\n")

        # String takes form of:
        # second minute hour day month year weekday
        serialData = "TST?" + now.strftime('%S%M%H%d%m%y%w') + "?"

        if self.ser.isOpen():
            self.ser.write(str.encode(serialData))
        else:
            self.msgBox.insert(tk.END, "Please connect to COMPORT first\n")
            self.msgBox.see("end")
            return

        self.ser.timeout = 1

        self.FormatProgramString()

        time.sleep(2)

        serialData = b"PGM?" + self.txData + b"?"

        self.ser.write(serialData)

        time.sleep(1)

       # serialData = b"ACC" + bytes(int(self.fixAcc.get())) + b"?"


        #self.ser.write(serialData)
        #print(serialData)

        f.write("Fixes per day: " + self.fixAmt.get() + "\n")
        f.write("VHF Hours: " + self.vhfSrt.get() + ":" + self.vhfStp.get() + "\n")
        f.write("VHF Days: ")

        if self.monB.get():
            f.write("Monday ")
        if self.teusB.get():
            f.write("Teusday ")
        if self.wedB.get():
            f.write("Wednesday ")
        if self.thurB.get():
            f.write("Thursday ")
        if self.friB.get():
            f.write("Friday ")
        if self.satB.get():
            f.write("Saturday ")
        if self.sunB.get():
            f.write("Sunday ")
        f.write("\n END OF FILE")

        f.close()

        serialData = "GPG?"
        self.ser.write(str.encode(serialData))

        print(self.ser.read(50))

        self.msgBox.insert(tk.END, "Program File created, programing complete\n")
        self.msgBox.see("end")



    # =====================================
    def ProgramSmallDevice(self):

        # Set up the XBee device
        # must be done first due to timeout constraints
        # sending CFG command with the dongle sn
        if self.ser.isOpen():
            self.msgBox.insert(tk.END, "Configuring Small Collar\n")
            self.msgBox.see("end")
        else:
            self.msgBox.insert(tk.END, "Please connect to COMPORT first\n")
            self.msgBox.see("end")
            return

        self.update()

        # Set time on RTC first
        now = datetime.datetime.now()
        print(now)

        # Set up filepointer
        self.msgBox.insert(tk.END, "Creating Config File " + "\n")
        self.msgBox.see("end")
        self.update()
        f = open(str(now) + ".txt", "w+")

        # File header
        f.write("Collar configuration file for SN: " + str(now) + "\n\n")
        f.write("Collar configured at " + str(now) + "\n")

        # String takes form of:
        # second minute hour day month year weekday
        serialData = "TST?" + now.strftime('%S%M%H%d%m%y%w') + "?"

        if self.ser.isOpen():
            self.ser.write(str.encode(serialData))
        else:
            self.msgBox.insert(tk.END, "Please connect to COMPORT first\n")
            self.msgBox.see("end")
            return

        self.ser.timeout = 1

        self.FormatProgramString()

        time.sleep(2)

        serialData = b"PGM?" + self.txData + b"?"

        self.ser.write(serialData)

        time.sleep(1)

        f.write("Fixes per day: " + self.fixAmt.get() + "\n")

        f.write("\n END OF FILE")

        f.close()

        serialData = "GPG?"
        self.ser.write(str.encode(serialData))

        print(self.ser.read(50))

        self.msgBox.insert(tk.END, "Program File created, programing complete\n")
        self.msgBox.see("end")



    # =====================================
    def GetSN(self):
        if self.ser.isOpen():
            self.ser.write(str.encode("GSN?"))
            self.msgBox.see("end")
        else:
            self.msgBox.insert(tk.END, "Please connect to COMPORT first\n")
            self.msgBox.see("end")
            return

        self.ser.timeout = 2

        self.ser.read(2)
        tmpStr = self.ser.read(14)

        self.snEntry.configure(state="normal")
        self.snEntry.delete(0, 'end')
        tmpStr = tmpStr.decode()
        self.snEntry.insert(0, tmpStr)
        self.snEntry.configure(state='readonly')


# =======================================================================================================================
class PageDataRx(tk.Frame):
    def __init__(self, parent, controller):
        tk.Frame.__init__(self, parent)
        tk.Frame.__init__(self, parent)

        # Labels
        ttk.Label(self, text="Receive Data", font=LARGE_FONT).grid(row=0, column=0)
        ttk.Label(self, text="COM Port", font=LARGE_FONT).grid(row=1, column=0)
        ttk.Label(self, text="Dongle SN", font=LARGE_FONT).grid(row=3, column=0)
        ttk.Label(self, text="Target SN", font=LARGE_FONT).grid(row=5, column=0)
        ttk.Label(self, text="Messages", font=LARGE_FONT).grid(row=7, column=0)

        # label = tk.Label(self, text="Receive Data", font=LARGE_FONT)
        # label.pack(pady=10, padx=10)

        # Buttons
        button1 = ttk.Button(self, text="Home",
                             command=lambda: controller.show_frame(StartPage))
        button1.grid(row=0, column=2)
        button2 = ttk.Button(self, text="Get Dongle SN",
                             command=self.GetDongleSN)
        button2.grid(row=4, column=1)

        button3 = ttk.Button(self, text="Set Dongle Target",
                             command=self.SetDongleTx)
        button3.grid(row=6, column=1)

        #button4 = ttk.Button(self, text="Connect and Download",
        #                     command=self.ConnectDownload)
        #button4.grid(row=2, column=1)

        self.connButton = tk.IntVar()
        tk.Checkbutton(self, text="Connect and Download", variable=self.connButton,  command=self.ConnectDownload).grid(row=2, column=1)


        # Entries
        self.comPortEntry = tk.Entry(self)
        self.snEntry = tk.Entry(self)

        self.comPortEntry.grid(row=2, column=0)
        self.snEntry.grid(row=4, column=0)

        self.snEntry.insert(0, '')
        self.snEntry.configure(state='readonly')

        self.txEntry = tk.Entry(self)
        self.txEntry.grid(row=6, column=0)

        # Text Box
        self.msgBox = scrolledtext.ScrolledText(self, height=10, width=40)
        self.msgBox.grid(row=7, column=0)

        # Serial Port
        self.ser = serial.Serial(
            port=None,
            # port='\\\\.\\COM3',
            baudrate=9600,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS
        )

    def GetDongleSN(self):

        self.ser.port = '\\\\.\\COM' + self.comPortEntry.get()

        # global serial_object
        # serial_object = self.ser.port
        # Connect()

        tmpString = ''
        # Check to see if port is open or if port exists
        if self.ser.isOpen():
            try:
                self.ser.close()
            except IOError:
                self.msgBox.insert(tk.END, "Could not close port: " + self.comPortEntry.get() + '\n')
                self.msgBox.see("end")
                return
            self.msgBox.insert(tk.END, "Comport was open, closing now\n")
            self.msgBox.see("end")

        # Attempt to open port with current params
        try:
            self.ser.open()
        except IOError:
            self.msgBox.insert(tk.END, "Could not open port: " + self.comPortEntry.get() + '\n')
            self.msgBox.insert(tk.END, "List of Ports ready to connect:\n")
            self.msgBox.insert(tk.END, str(serial_ports()) + '\n')
            self.msgBox.see("end")
            return
        if self.ser.isOpen():
            self.msgBox.insert(tk.END, "Comport " + self.comPortEntry.get() + " opened succesfully.\n")
            self.msgBox.see("end")
            self.msgBox.insert(tk.END, "Attempting device handshake\n")
            self.msgBox.see("end")

        # Flush port before use
        self.ser.flushInput()
        self.ser.flushOutput()
        self.update()

        # Send +++ to device
        time.sleep(1)
        self.ser.write(str.encode("+++"))
        self.msgBox.insert(tk.END, "Sent\n")
        self.msgBox.see("end")
        self.update()

        # Wait for rx
        self.ser.timeout = 2
        tmpString = self.ser.read(2)
        print(tmpString)
        if tmpString == b'OK':
            self.msgBox.insert(tk.END, "Programming mode entered\n")
            self.msgBox.see("end")
        else:
            self.msgBox.insert(tk.END, "Could not connect to device\n")
            self.msgBox.see("end")
            return

        # Get SN Upper
        self.msgBox.insert(tk.END, "Getting the address from dongle\n")
        self.msgBox.see("end")
        self.update()
        self.ser.write(b'ATSH\r')
        tmpa = self.ser.read(8)
        self.ser.write(b'ATSL\r')
        tmpb = self.ser.read(8)
        tmpString = tmpa[1:7] + tmpb[0:8]

        self.snEntry.configure(state='normal')
        self.snEntry.delete(0, 'end')
        self.snEntry.insert(0, tmpString.decode())
        self.snEntry.configure(state='readonly')
        print(tmpString.decode())

    def GetDongleDN(self):

        # Flush port before use
        self.ser.flushInput()
        self.ser.flushOutput()
        self.update()

        # Send +++ to device
        time.sleep(1)
        self.ser.write(str.encode("+++"))
        self.msgBox.insert(tk.END, "Sent\n")
        self.update()

        # Wait for rx
        self.ser.timeout = 2
        tmpString = self.ser.read(2)
        print(tmpString)
        if tmpString == b'OK':
            self.msgBox.insert(tk.END, "Programming mode entered\n")
            self.msgBox.see("end")
        else:
            self.msgBox.insert(tk.END, "Could not connect to device\n")
            self.msgBox.see("end")
            return

        # Get SN Upper
        self.msgBox.insert(tk.END, "Getting the address from dongle\n")
        self.msgBox.see("end")
        self.update()
        self.ser.write(b'ATDH\r')
        tmpa = self.ser.read(8)
        self.ser.write(b'ATDL\r')
        tmpb = self.ser.read(8)
        tmpString = tmpa[1:7] + tmpb[0:8]

        self.txEntry.delete(0, 'end')
        self.txEntry.insert(0, tmpString.decode())
        print(tmpString.decode())

    def ConnectDownload(self):

        self.ser.port = '\\\\.\\COM' + self.comPortEntry.get()

        # global serial_object
        # serial_object = self.ser.port
        # Connect()
        if self.connButton.get()!= 1:
            return

        tmpString = ''
        # Check to see if port is open or if port exists
        if self.ser.isOpen():
            try:
                self.ser.close()
            except IOError:
                self.msgBox.insert(tk.END, "Could not close port: " + self.comPortEntry.get() + '\n')
                self.msgBox.see("end")
                return
            self.msgBox.insert(tk.END, "Comport was open, closing now\n")
            self.msgBox.see("end")

        # Attempt to open port with current params
        try:
            self.ser.open()
        except IOError:
            self.msgBox.insert(tk.END, "Could not open port: " + self.comPortEntry.get() + '\n')
            self.msgBox.insert(tk.END, "List of Ports ready to connect:\n")
            self.msgBox.insert(tk.END, str(serial_ports()) + '\n')
            self.msgBox.see("end")
            self.msgBox.see("end")
            return
        if self.ser.isOpen():
            self.msgBox.insert(tk.END, "Comport " + self.comPortEntry.get() + " opened succesfully.\n")
            self.msgBox.insert(tk.END, "Attempting device handshake\n")
            self.msgBox.see("end")

        # Flush port before use
        self.ser.flushInput()
        self.ser.flushOutput()
        self.update()

        self.ser.timeout = 0

        # Send '?' and look for ack
        while self.connButton.get() == 1 and tmpString != b'?':
            for x in range(10):
                self.ser.write(b'?')
                self.msgBox.insert(tk.END, "Attempt " + str(x) + "\n")
                self.msgBox.see("end")
                self.update()
                time.sleep(0.2)
                tmpString = self.ser.read(1)
                if tmpString == b'?':
                    break
        if tmpString == b'?':
            self.msgBox.insert(tk.END, "Connection Made\n")
            self.msgBox.see("end")
            self.ser.timeout = 1;
            self.ser.read(5)# clears spam chars
        else:
            self.msgBox.insert(tk.END, "Connection Failed\n")
            self.msgBox.see("end")
            return

        # Ask collar for data
        self.msgBox.insert(tk.END, "Requesting Data\n")
        self.msgBox.see("end")
        self.update()

        self.ser.write(b'DTA?')

        # self.ser.timeout = 1
        tmpString = bytearray(b'')
        response = []

        self.ser.timeout = 10

        tmpString = self.ser.read_until(b'?')

        print(tmpString)


        # tmpString = self.ser.read(100)
        tmpString = tmpString.decode()

        time.sleep(1)

        self.GetDongleDN()

        now = datetime.datetime.now()

        path = self.txEntry.get() + "_" + now.strftime('%m_%d_%y_%H_%M') + ".txt"

        f = open(path, "w+")

        f.write("Hour,Min,Day,Month,Year,Northing Deg,Northing Min,Direction,Easting Deg,Easting Min,Direction\n")
        f.write(tmpString)
        f.close()

        pre, ext = os.path.splitext(path)
        os.rename(path, pre + ".csv")

        self.msgBox.insert(tk.END, "Data saved to: " + pre + ".csv\n")
        self.msgBox.see("end")

    def SetDongleTx(self):
        self.ser.port = '\\\\.\\COM' + self.comPortEntry.get()

        # global serial_object
        # serial_object = self.ser.port
        # Connect()

        tmpString = ''
        # Check to see if port is open or if port exists
        if self.ser.isOpen():
            try:
                self.ser.close()
            except IOError:
                self.msgBox.insert(tk.END, "Could not close port: " + self.comPortEntry.get() + '\n')
                self.msgBox.see("end")
                return
            self.msgBox.insert(tk.END, "Comport was open, closing now\n")
            self.msgBox.see("end")

        # Attempt to open port with current params
        try:
            self.ser.open()
        except IOError:
            self.msgBox.insert(tk.END, "Could not open port: " + self.comPortEntry.get() + '\n')
            self.msgBox.insert(tk.END, "List of Ports ready to connect:\n")
            self.msgBox.insert(tk.END, str(serial_ports()) + '\n')
            self.msgBox.see("end")
            return
        if self.ser.isOpen():
            self.msgBox.insert(tk.END, "Comport " + self.comPortEntry.get() + " opened succesfully.\n")
            self.msgBox.insert(tk.END, "Attempting device handshake\n")
            self.msgBox.see("end")

        # Flush port before use
        self.ser.flushInput()
        self.ser.flushOutput()
        self.update()

        # Send +++ to device
        time.sleep(1)
        self.ser.write(str.encode("+++"))
        self.msgBox.insert(tk.END, "Sent\n")
        self.msgBox.see("end")
        self.update()

        # Wait for rx
        self.ser.timeout = 2
        tmpString = self.ser.read(2)
        print(tmpString)
        if tmpString == b'OK':
            self.msgBox.insert(tk.END, "Programming mode entered\n")
            self.msgBox.see("end")
        else:
            self.msgBox.insert(tk.END, "Could not connect to device\n")
            self.msgBox.see("end")
            return
        tmpString = self.txEntry.get()
        dh = tmpString[0:6]
        dl = tmpString[6:]

        self.ser.timeout = 2

        self.msgBox.insert(tk.END, "Setting DH to " + dh + "\n")
        self.msgBox.see("end")
        self.update()

        # setting DH
        self.ser.write(str.encode("ATDH" + dh + '\r'))
        self.ser.read(3)
        self.ser.write(b'ATAC\r')
        self.ser.read(3)
        self.ser.write(b'ATDH\r')
        tmp = self.ser.read(8)
        tmp = tmp.decode()
        self.msgBox.insert(tk.END, "DH set to " + tmp + "\n")
        self.msgBox.see("end")
        self.update()

        # Setting DL
        self.msgBox.insert(tk.END, "Setting DL to " + dl + "\n")
        self.msgBox.see("end")
        self.update()

        self.ser.write(str.encode("ATDL" + dl + '\r'))
        self.ser.read(3)
        self.ser.write(b'ATAC\r')
        self.ser.read(3)
        self.ser.write(b'ATDL\r')
        tmp = self.ser.read(8)
        tmp = tmp.decode()
        self.msgBox.insert(tk.END, "DL set to " + tmp + "\n")
        self.msgBox.see("end")
        self.update()

        # Save to memory
        self.ser.write(str.encode("ATWR" + '\r'))
        tmp = self.ser.read(3)
        tmp = tmp.decode()
        self.msgBox.insert(tk.END, tmp + "\n")
        self.msgBox.see("end")

        print(dh)
        print(dl)


# =======================================================================================================================
class PagePowerCalc(tk.Frame):
    def __init__(self, parent, controller):
        tk.Frame.__init__(self, parent)
        tk.Frame.__init__(self, parent)
        label = tk.Label(self, text="Power Calculations", font=LARGE_FONT)
        label.pack(pady=10, padx=10)

        button1 = ttk.Button(self, text="Home",
                             command=lambda: controller.show_frame(StartPage))
        button1.pack()


# =======================================================================================================================

def serial_ports():
    """ Lists serial port names

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
            #result.append('\n')
        except (OSError, serial.SerialException):
            pass
    return result





app = CollarProgram()
# app.update()
# after(500, app.update())
app.mainloop()
