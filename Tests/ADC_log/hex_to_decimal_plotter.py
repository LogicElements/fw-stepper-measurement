import matplotlib.pyplot as plt
import numpy as np
import tkinter as tk
from tkinter import filedialog, messagebox
import os
import struct

class HexDataPlotter:
    def __init__(self):
        self.root = tk.Tk()
        self.root.title("HEX to Decimal Plotter")
        self.root.geometry("1200x900")
        
        # Data storage for six datasets
        self.hex_data1 = []
        self.decimal_data1 = []
        self.hex_data2 = []
        self.decimal_data2 = []
        self.hex_data3 = []
        self.decimal_data3 = []
        self.hex_data4 = []
        self.decimal_data4 = []
        self.hex_data5 = []
        self.decimal_data5 = []
        self.hex_data6 = []
        self.decimal_data6 = []
        self.hex_data7 = []
        self.decimal_data7 = []
        
        # Setup proper window closing protocol
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
        
        self.setup_ui()
        
        # Automaticky předvyplníme názvy souborů
        self.auto_fill_file_paths()
        
    def setup_ui(self):
        # Main frame
        main_frame = tk.Frame(self.root, padx=20, pady=20)
        main_frame.pack(fill=tk.BOTH, expand=True)
        
        # Title
        title_label = tk.Label(main_frame, text="HEX to Decimal Data Plotter", 
                              font=("Arial", 16, "bold"))
        title_label.pack(pady=(0, 20))
        
        # File selection frame - Dataset 1
        file_frame1 = tk.Frame(main_frame)
        file_frame1.pack(fill=tk.X, pady=(0, 10))
        
        tk.Label(file_frame1, text="Soubor 1:").pack(side=tk.LEFT, padx=(0, 8))
        self.file_path_var1 = tk.StringVar()
        file_entry1 = tk.Entry(file_frame1, textvariable=self.file_path_var1, width=45)
        file_entry1.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 8))
        
        browse_btn1 = tk.Button(file_frame1, text="Procházet", command=lambda: self.browse_file(1))
        browse_btn1.pack(side=tk.LEFT, padx=(0, 8))
        load_btn1 = tk.Button(file_frame1, text="Načíst 1", command=lambda: self.load_and_convert(1))
        load_btn1.pack(side=tk.LEFT)

        # File selection frame - Dataset 2
        file_frame2 = tk.Frame(main_frame)
        file_frame2.pack(fill=tk.X, pady=(0, 10))
        
        tk.Label(file_frame2, text="Soubor 2:").pack(side=tk.LEFT, padx=(0, 8))
        self.file_path_var2 = tk.StringVar()
        file_entry2 = tk.Entry(file_frame2, textvariable=self.file_path_var2, width=45)
        file_entry2.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 8))
        
        browse_btn2 = tk.Button(file_frame2, text="Procházet", command=lambda: self.browse_file(2))
        browse_btn2.pack(side=tk.LEFT, padx=(0, 8))
        load_btn2 = tk.Button(file_frame2, text="Načíst 2", command=lambda: self.load_and_convert(2))
        load_btn2.pack(side=tk.LEFT)

        # File selection frame - Dataset 3
        file_frame3 = tk.Frame(main_frame)
        file_frame3.pack(fill=tk.X, pady=(0, 10))
        
        tk.Label(file_frame3, text="Soubor 3:").pack(side=tk.LEFT, padx=(0, 8))
        self.file_path_var3 = tk.StringVar()
        file_entry3 = tk.Entry(file_frame3, textvariable=self.file_path_var3, width=45)
        file_entry3.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 8))
        
        browse_btn3 = tk.Button(file_frame3, text="Procházet", command=lambda: self.browse_file(3))
        browse_btn3.pack(side=tk.LEFT, padx=(0, 8))
        load_btn3 = tk.Button(file_frame3, text="Načíst 3", command=lambda: self.load_and_convert(3))
        load_btn3.pack(side=tk.LEFT)

        # File selection frame - Dataset 4 (Počítání kroků)
        file_frame4 = tk.Frame(main_frame)
        file_frame4.pack(fill=tk.X, pady=(0, 10))
        
        tk.Label(file_frame4, text="Soubor 4 (Kroky):").pack(side=tk.LEFT, padx=(0, 8))
        self.file_path_var4 = tk.StringVar()
        file_entry4 = tk.Entry(file_frame4, textvariable=self.file_path_var4, width=45)
        file_entry4.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 8))
        
        browse_btn4 = tk.Button(file_frame4, text="Procházet", command=lambda: self.browse_file(4))
        browse_btn4.pack(side=tk.LEFT, padx=(0, 8))
        load_btn4 = tk.Button(file_frame4, text="Načíst 4", command=lambda: self.load_and_convert(4))
        load_btn4.pack(side=tk.LEFT)

        # File selection frame - Dataset 5
        file_frame5 = tk.Frame(main_frame)
        file_frame5.pack(fill=tk.X, pady=(0, 10))
        
        tk.Label(file_frame5, text="Soubor 5:").pack(side=tk.LEFT, padx=(0, 8))
        self.file_path_var5 = tk.StringVar()
        file_entry5 = tk.Entry(file_frame5, textvariable=self.file_path_var5, width=45)
        file_entry5.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 8))
        
        browse_btn5 = tk.Button(file_frame5, text="Procházet", command=lambda: self.browse_file(5))
        browse_btn5.pack(side=tk.LEFT, padx=(0, 8))
        load_btn5 = tk.Button(file_frame5, text="Načíst 5", command=lambda: self.load_and_convert(5))
        load_btn5.pack(side=tk.LEFT)

        # File selection frame - Dataset 6
        file_frame6 = tk.Frame(main_frame)
        file_frame6.pack(fill=tk.X, pady=(0, 20))
        
        tk.Label(file_frame6, text="Soubor 6:").pack(side=tk.LEFT, padx=(0, 8))
        self.file_path_var6 = tk.StringVar()
        file_entry6 = tk.Entry(file_frame6, textvariable=self.file_path_var6, width=45)
        file_entry6.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 8))
        
        browse_btn6 = tk.Button(file_frame6, text="Procházet", command=lambda: self.browse_file(6))
        browse_btn6.pack(side=tk.LEFT, padx=(0, 8))
        load_btn6 = tk.Button(file_frame6, text="Načíst 6", command=lambda: self.load_and_convert(6))
        load_btn6.pack(side=tk.LEFT)
        
        # File selection frame - Dataset 7
        file_frame7 = tk.Frame(main_frame)
        file_frame7.pack(fill=tk.X, pady=(0, 20))
        
        tk.Label(file_frame7, text="Soubor 7:").pack(side=tk.LEFT, padx=(0, 8))
        self.file_path_var7 = tk.StringVar()
        file_entry7 = tk.Entry(file_frame7, textvariable=self.file_path_var7, width=45)
        file_entry7.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 8))
        
        browse_btn7 = tk.Button(file_frame7, text="Procházet", command=lambda: self.browse_file(7))
        browse_btn7.pack(side=tk.LEFT, padx=(0, 8))
        load_btn7 = tk.Button(file_frame7, text="Načíst 7", command=lambda: self.load_and_convert(7))
        load_btn7.pack(side=tk.LEFT)
        
        # Options frame
        options_frame = tk.Frame(main_frame)
        options_frame.pack(fill=tk.X, pady=(0, 20))
        
        # Data format options
        format_label = tk.Label(options_frame, text="Formát dat:")
        format_label.pack(anchor=tk.W)
        
        self.format_var = tk.StringVar(value="space")
        format_frame = tk.Frame(options_frame)
        format_frame.pack(fill=tk.X, pady=(0, 10))
        
        tk.Radiobutton(format_frame, text="Oddělené mezerami", variable=self.format_var, 
                      value="space", command=self.on_format_change).pack(side=tk.LEFT)
        tk.Radiobutton(format_frame, text="Oddělené čárkami", variable=self.format_var, 
                      value="comma", command=self.on_format_change).pack(side=tk.LEFT)
        tk.Radiobutton(format_frame, text="Jeden HEX na řádek", variable=self.format_var, 
                      value="line", command=self.on_format_change).pack(side=tk.LEFT)
        tk.Radiobutton(format_frame, text="16-bit + 2 fixní byty (00 08)", variable=self.format_var, 
                      value="16bit_fixed", command=self.on_format_change).pack(side=tk.LEFT)
        
        # Data type options
        type_label = tk.Label(options_frame, text="Typ dat:")
        type_label.pack(anchor=tk.W)
        
        self.data_type_var = tk.StringVar(value="unsigned16")
        type_frame = tk.Frame(options_frame)
        type_frame.pack(fill=tk.X, pady=(0, 10))
        
        tk.Radiobutton(type_frame, text="8-bit bez znaménka (0-255)", variable=self.data_type_var, 
                      value="unsigned", command=self.toggle_byte_order).pack(side=tk.LEFT)
        tk.Radiobutton(type_frame, text="8-bit se znaménkem (-128 až 127)", variable=self.data_type_var, 
                      value="signed", command=self.toggle_byte_order).pack(side=tk.LEFT)
        tk.Radiobutton(type_frame, text="16-bit bez znaménka (0-65535)", variable=self.data_type_var, 
                      value="unsigned16", command=self.toggle_byte_order).pack(side=tk.LEFT)
        tk.Radiobutton(type_frame, text="16-bit se znaménkem (-32768 až 32767)", variable=self.data_type_var, 
                      value="signed16", command=self.toggle_byte_order).pack(side=tk.LEFT)
        tk.Radiobutton(type_frame, text="32-bit bez znaménka (0-4294967295)", variable=self.data_type_var, 
                      value="unsigned32", command=self.toggle_byte_order).pack(side=tk.LEFT)
        tk.Radiobutton(type_frame, text="32-bit se znaménkem (-2147483648 až 2147483647)", variable=self.data_type_var, 
                      value="signed32", command=self.toggle_byte_order).pack(side=tk.LEFT)
        
        # Byte order options
        self.byte_order_var = tk.StringVar(value="little_endian")
        self.byte_order_frame = tk.Frame(options_frame)
        self.byte_order_frame.pack(fill=tk.X, pady=(0, 10))
        
        tk.Label(self.byte_order_frame, text="Pořadí bytů:").pack(anchor=tk.W)
        
        # 8-bit options
        bit8_frame = tk.Frame(self.byte_order_frame)
        bit8_frame.pack(fill=tk.X, pady=(0, 5))
        tk.Label(bit8_frame, text="8-bit: ").pack(side=tk.LEFT)
        tk.Radiobutton(bit8_frame, text="Standardní", variable=self.byte_order_var, 
                      value="standard_8bit").pack(side=tk.LEFT)
        
        # 16-bit options
        bit16_frame = tk.Frame(self.byte_order_frame)
        bit16_frame.pack(fill=tk.X, pady=(0, 5))
        tk.Label(bit16_frame, text="16-bit: ").pack(side=tk.LEFT)
        tk.Radiobutton(bit16_frame, text="Little Endian (49 07 = 0x0749)", variable=self.byte_order_var, 
                      value="little_endian").pack(side=tk.LEFT)
        tk.Radiobutton(bit16_frame, text="Big Endian (49 07 = 0x4907)", variable=self.byte_order_var, 
                      value="big_endian").pack(side=tk.LEFT)
        
        # 32-bit options
        bit32_frame = tk.Frame(self.byte_order_frame)
        bit32_frame.pack(fill=tk.X, pady=(0, 5))
        tk.Label(bit32_frame, text="32-bit: ").pack(side=tk.LEFT)
        tk.Radiobutton(bit32_frame, text="Little Endian (49 07 00 08 = 0x08000749)", variable=self.byte_order_var, 
                      value="little_endian_32").pack(side=tk.LEFT)
        tk.Radiobutton(bit32_frame, text="Big Endian (49 07 00 08 = 0x49070008)", variable=self.byte_order_var, 
                      value="big_endian_32").pack(side=tk.LEFT)
        
        # Initially hide byte order frame
        self.byte_order_frame.pack_forget()
        
        # Dataset visibility controls
        visibility_frame = tk.Frame(main_frame)
        visibility_frame.pack(fill=tk.X, pady=(0, 10))
        
        tk.Label(visibility_frame, text="Zobrazení datasetů:", font=("Arial", 12, "bold")).pack(anchor=tk.W)
        
        checkbox_frame = tk.Frame(visibility_frame)
        checkbox_frame.pack(fill=tk.X, pady=(5, 0))
        
        # Checkboxy pro zobrazení datasetů
        self.show_data1_var = tk.BooleanVar(value=True)
        self.show_data2_var = tk.BooleanVar(value=True)
        self.show_data3_var = tk.BooleanVar(value=True)
        self.show_data4_var = tk.BooleanVar(value=True)
        self.show_data5_var = tk.BooleanVar(value=True)
        self.show_data6_var = tk.BooleanVar(value=True)
        self.show_data7_var = tk.BooleanVar(value=True)
        
        # Checkbox pro zobrazení tabulek hodnot
        self.show_tables_var = tk.BooleanVar(value=False)
        
        tk.Checkbutton(checkbox_frame, text="Data 1 (modrá)", variable=self.show_data1_var).pack(side=tk.LEFT, padx=(0, 15))
        tk.Checkbutton(checkbox_frame, text="Data 2 (oranžová)", variable=self.show_data2_var).pack(side=tk.LEFT, padx=(0, 15))
        tk.Checkbutton(checkbox_frame, text="Data 3 (zelená + 2050)", variable=self.show_data3_var).pack(side=tk.LEFT, padx=(0, 15))
        tk.Checkbutton(checkbox_frame, text="Data 4 (červená - Kroky)", variable=self.show_data4_var).pack(side=tk.LEFT)
        
        # Second row of checkboxes
        checkbox_frame2 = tk.Frame(visibility_frame)
        checkbox_frame2.pack(fill=tk.X, pady=(5, 0))
        
        tk.Checkbutton(checkbox_frame2, text="Data 5 (fialová)", variable=self.show_data5_var).pack(side=tk.LEFT, padx=(0, 15))
        tk.Checkbutton(checkbox_frame2, text="Data 6 (hnědá)", variable=self.show_data6_var).pack(side=tk.LEFT, padx=(0, 15))
        tk.Checkbutton(checkbox_frame2, text="Data 7 (šedá)", variable=self.show_data7_var).pack(side=tk.LEFT, padx=(0, 15))
        
        # Checkbox pro zobrazení tabulek hodnot
        tk.Checkbutton(checkbox_frame2, text="Zobrazit tabulky hodnot", variable=self.show_tables_var).pack(side=tk.LEFT, padx=(20, 0))

        # Buttons frame
        button_frame = tk.Frame(main_frame)
        button_frame.pack(fill=tk.X, pady=(0, 20))
        
        plot_btn = tk.Button(button_frame, text="Zobrazit graf", command=self.plot_data, state=tk.DISABLED)
        plot_btn.pack(side=tk.LEFT)
        
        self.plot_button = plot_btn
        
        # Load all datasets button
        load_all_btn = tk.Button(button_frame, text="Načíst vše", command=self.load_all_datasets)
        load_all_btn.pack(side=tk.LEFT, padx=(10, 0))
        
        # Status label
        self.status_var = tk.StringVar(value="Vyberte soubor s HEX daty")
        status_label = tk.Label(main_frame, textvariable=self.status_var, fg="blue")
        status_label.pack()
        
    def auto_fill_file_paths(self):
        """Automaticky předvyplní názvy souborů do načítacích kolonek"""
        import os
        
        # Získáme aktuální adresář
        current_dir = os.getcwd()
        
        # Předvyplníme názvy souborů
        self.file_path_var1.set(os.path.join(current_dir, "sin_1.txt"))
        self.file_path_var2.set(os.path.join(current_dir, "sin_2.txt"))
        self.file_path_var3.set(os.path.join(current_dir, "deg.txt"))
        self.file_path_var4.set(os.path.join(current_dir, "steps.txt"))
        self.file_path_var5.set("C:/Users/rousa/Documents/jablotron/stepper_measurement/git/Tests/ADC_log/Usin_1.txt")
        self.file_path_var6.set("C:/Users/rousa/Documents/jablotron/stepper_measurement/git/Tests/ADC_log/Usin_2.txt")
        self.file_path_var7.set(os.path.join(current_dir, "data7.txt"))
        
        # Aktualizujeme status
        self.status_var.set("Soubory předvyplněny: sin_1.txt, sin_2.txt, deg.txt, steps.txt, Usin_1.txt, Usin_2.txt, data7.txt")
        
    def toggle_byte_order(self):
        """Show/hide byte order options based on data type selection"""
        if self.data_type_var.get() in ["unsigned16", "signed16", "unsigned32", "signed32"]:
            self.byte_order_frame.pack(fill=tk.X, pady=(0, 10))
        else:
            self.byte_order_frame.pack_forget()
        
    def browse_file(self, which: int):
        file_path = filedialog.askopenfilename(
            title="Vyberte textový soubor s HEX daty",
            filetypes=[("Text files", "*.txt"), ("All files", "*.*")]
        )
        if file_path:
            if which == 1:
                self.file_path_var1.set(file_path)
            elif which == 2:
                self.file_path_var2.set(file_path)
            elif which == 3:
                self.file_path_var3.set(file_path)
            elif which == 4:
                self.file_path_var4.set(file_path)
            elif which == 5:
                self.file_path_var5.set(file_path)
            elif which == 6:
                self.file_path_var6.set(file_path)
            else:  # which == 7
                self.file_path_var7.set(file_path)
            self.status_var.set(f"Vybraný soubor {which}: {os.path.basename(file_path)}")
            
    def load_and_convert(self, which: int):
        if which == 1:
            file_path = self.file_path_var1.get()
        elif which == 2:
            file_path = self.file_path_var2.get()
        elif which == 3:
            file_path = self.file_path_var3.get()
        elif which == 4:
            file_path = self.file_path_var4.get()
        elif which == 5:
            file_path = self.file_path_var5.get()
        elif which == 6:
            file_path = self.file_path_var6.get()
        else:  # which == 7
            file_path = self.file_path_var7.get()
            
        if not file_path:
            messagebox.showerror("Chyba", f"Vyberte prosím soubor {which}")
            return
            
        try:
            with open(file_path, 'r', encoding='utf-8') as file:
                lines = file.readlines()
                
            # Parse HEX data based on format
            hex_strings = []
            if self.format_var.get() == "space":
                # Split each line by spaces and flatten the result
                for line in lines:
                    line = line.strip()
                    if line:  # Skip empty lines
                        hex_strings.extend(line.split())
            elif self.format_var.get() == "comma":
                # Split each line by commas and flatten the result
                for line in lines:
                    line = line.strip()
                    if line:  # Skip empty lines
                        hex_strings.extend([x.strip() for x in line.split(',')])
            elif self.format_var.get() == "16bit_fixed":
                # Special format: each 16-bit value followed by 00 08
                for line in lines:
                    line = line.strip()
                    if line:
                        hex_parts = line.split()
                        if len(hex_parts) >= 4:  # Should have 4 bytes: XX XX 00 08
                            # Take first two bytes as the 16-bit value
                            hex_strings.extend(hex_parts[:2])
                        elif len(hex_parts) >= 2:  # At least 2 bytes
                            hex_strings.extend(hex_parts[:2])
                        else:
                            hex_strings.extend(hex_parts)
            else:  # line
                # Each line is one hex value
                hex_strings = [line.strip() for line in lines if line.strip()]
                
            # Filter out empty strings and clean hex values
            hex_strings = [x.strip() for x in hex_strings if x.strip()]
            
            # Convert to decimal (for selected dataset)
            if which == 1:
                self.decimal_data1 = []
                self.hex_data1 = []
            elif which == 2:
                self.decimal_data2 = []
                self.hex_data2 = []
            elif which == 3:
                self.decimal_data3 = []
                self.hex_data3 = []
            elif which == 4:
                self.decimal_data4 = []
                self.hex_data4 = []
            elif which == 5:
                self.decimal_data5 = []
                self.hex_data5 = []
            elif which == 6:
                self.decimal_data6 = []
                self.hex_data6 = []
            else:  # which == 7
                self.decimal_data7 = []
                self.hex_data7 = []
            
            # Speciální parsování pro třetí dataset (deg.txt) - IEEE-754 float
            if which == 3:
                self.parse_ieee754_float(hex_strings, which)
                return
            
            # Speciální parsování pro čtvrtý dataset (steps.txt) - počítání kroků
            if which == 4:
                self.parse_steps_data(hex_strings, which)
                return
            
            # Check if we need to combine bytes (for 16-bit or 32-bit values)
            combine_bytes = self.data_type_var.get() in ["unsigned16", "signed16", "unsigned32", "signed32"]
            
            if combine_bytes and len(hex_strings) >= 2:
                # Determine how many bytes to combine
                if self.data_type_var.get() in ["unsigned32", "signed32"]:
                    bytes_to_combine = 4  # 32-bit values
                    step_size = 4
                else:
                    bytes_to_combine = 2  # 16-bit values
                    step_size = 2
                
                # Process as multi-byte values
                for i in range(0, len(hex_strings) - bytes_to_combine + 1, step_size):
                    try:
                        # Get bytes for the value
                        bytes_list = []
                        for j in range(bytes_to_combine):
                            if i + j < len(hex_strings):
                                byte_str = hex_strings[i + j].replace('0x', '').replace('0X', '').replace('h', '').replace('H', '')
                                if byte_str:
                                    bytes_list.append(int(byte_str, 16))
                                else:
                                    bytes_list.append(0)
                            else:
                                bytes_list.append(0)
                        
                        if len(bytes_list) < bytes_to_combine:
                            continue
                            
                        # Combine bytes based on byte order
                        hex_value = 0
                        hex_display = ""
                        
                        if self.byte_order_var.get() in ["little_endian", "little_endian_32"]:
                            # Little Endian: least significant byte first
                            for j, byte in enumerate(bytes_list):
                                hex_value |= (byte << (8 * j))
                                hex_display = hex_display + f"{byte:02X}"
                        elif self.byte_order_var.get() in ["big_endian", "big_endian_32"]:
                            # Big Endian: most significant byte first
                            for j, byte in enumerate(bytes_list):
                                hex_value |= (byte << (8 * (bytes_to_combine - 1 - j)))
                                hex_display = hex_display + f"{byte:02X}"
                        else:
                            # Standard 8-bit processing
                            hex_value = bytes_list[0]
                            hex_display = f"{bytes_list[0]:02X}"
                        
                        # Apply signed conversion if needed
                        if self.data_type_var.get() == "signed16" and hex_value > 32767:
                            hex_value = hex_value - 65536
                        elif self.data_type_var.get() == "signed32" and hex_value > 2147483647:
                            hex_value = hex_value - 4294967296
                            
                        if which == 1:
                            self.decimal_data1.append(hex_value)
                            self.hex_data1.append(hex_display)
                        elif which == 2:
                            self.decimal_data2.append(hex_value)
                            self.hex_data2.append(hex_display)
                        elif which == 3:
                            self.decimal_data3.append(hex_value)
                            self.hex_data3.append(hex_display)
                        elif which == 4:
                            self.decimal_data4.append(hex_value)
                            self.hex_data4.append(hex_display)
                        elif which == 5:
                            self.decimal_data5.append(hex_value)
                            self.hex_data5.append(hex_display)
                        elif which == 6:
                            self.decimal_data6.append(hex_value)
                            self.hex_data6.append(hex_display)
                        else:  # which == 7
                            self.decimal_data7.append(hex_value)
                            self.hex_data7.append(hex_display)
                        
                    except ValueError:
                        continue
                        
                # Handle remaining bytes if any
                remaining_start = (len(hex_strings) // step_size) * step_size
                if remaining_start < len(hex_strings):
                    try:
                        # Process remaining bytes as individual 8-bit values
                        for i in range(remaining_start, len(hex_strings)):
                            hex_str = hex_strings[i].replace('0x', '').replace('0X', '').replace('h', '').replace('H', '')
                            if hex_str:
                                hex_value = int(hex_str, 16)
                                if self.data_type_var.get() == "signed16" and hex_value > 127:
                                    hex_value = hex_value - 256
                                elif self.data_type_var.get() == "signed32" and hex_value > 127:
                                    hex_value = hex_value - 256
                                if which == 1:
                                    self.decimal_data1.append(hex_value)
                                    self.hex_data1.append(hex_str)
                                elif which == 2:
                                    self.decimal_data2.append(hex_value)
                                    self.hex_data2.append(hex_str)
                                elif which == 3:
                                    self.decimal_data3.append(hex_value)
                                    self.hex_data3.append(hex_str)
                                elif which == 4:
                                    self.decimal_data4.append(hex_value)
                                    self.hex_data4.append(hex_str)
                                elif which == 5:
                                    self.decimal_data5.append(hex_value)
                                    self.hex_data5.append(hex_str)
                                elif which == 6:
                                    self.decimal_data6.append(hex_value)
                                    self.hex_data6.append(hex_str)
                                else:  # which == 7
                                    self.decimal_data7.append(hex_value)
                                    self.hex_data7.append(hex_str)
                    except ValueError:
                        pass
            else:
                # Process as individual 8-bit values
                for hex_str in hex_strings:
                    # Remove common prefixes
                    hex_str = hex_str.replace('0x', '').replace('0X', '').replace('h', '').replace('H', '')
                    
                    if not hex_str:
                        continue
                        
                    try:
                        hex_value = int(hex_str, 16)
                        
                        # Apply data type conversion
                        if self.data_type_var.get() == "signed":
                            if hex_value > 127:
                                hex_value = hex_value - 256
                        elif self.data_type_var.get() == "unsigned16":
                            hex_value = hex_value & 0xFFFF
                        elif self.data_type_var.get() == "signed16":
                            if hex_value > 32767:
                                hex_value = hex_value - 65536
                        elif self.data_type_var.get() == "unsigned32":
                            hex_value = hex_value & 0xFFFFFFFF
                        elif self.data_type_var.get() == "signed32":
                            if hex_value > 2147483647:
                                hex_value = hex_value - 4294967296
                                
                        if which == 1:
                            self.decimal_data1.append(hex_value)
                            self.hex_data1.append(hex_str)
                        elif which == 2:
                            self.decimal_data2.append(hex_value)
                            self.hex_data2.append(hex_str)
                        elif which == 3:
                            self.decimal_data3.append(hex_value)
                            self.hex_data3.append(hex_str)
                        elif which == 4:
                            self.decimal_data4.append(hex_value)
                            self.hex_data4.append(hex_str)
                        elif which == 5:
                            self.decimal_data5.append(hex_value)
                            self.hex_data5.append(hex_str)
                        elif which == 6:
                            self.decimal_data6.append(hex_value)
                            self.hex_data6.append(hex_str)
                        else:  # which == 7
                            self.decimal_data7.append(hex_value)
                            self.hex_data7.append(hex_str)
                        
                    except ValueError:
                        continue
                    
            # Odebereme prvních 5 hodnot (simulace „vymazání" prvních pěti hodnot ze souboru)
            if which == 1:
                if len(self.decimal_data1) > 5:
                    self.decimal_data1 = self.decimal_data1[5:]
                    self.hex_data1 = self.hex_data1[5:]
                else:
                    self.decimal_data1 = []
                    self.hex_data1 = []
                loaded = len(self.decimal_data1)
                preview = ", ".join(self.hex_data1[:5]) + ("..." if len(self.hex_data1) > 5 else "")
            elif which == 2:
                if len(self.decimal_data2) > 5:
                    self.decimal_data2 = self.decimal_data2[5:]
                    self.hex_data2 = self.hex_data2[5:]
                else:
                    self.decimal_data2 = []
                    self.hex_data2 = []
                loaded = len(self.decimal_data2)
                preview = ", ".join(self.hex_data2[:5]) + ("..." if len(self.hex_data2) > 5 else "")
            elif which == 3:
                if len(self.decimal_data3) > 5:
                    self.decimal_data3 = self.decimal_data3[5:]
                    self.hex_data3 = self.hex_data3[5:]
                else:
                    self.decimal_data3 = []
                    self.hex_data3 = []
                loaded = len(self.decimal_data3)
                preview = ", ".join(self.hex_data3[:5]) + ("..." if len(self.hex_data3) > 5 else "")
            elif which == 4:
                if len(self.decimal_data4) > 5:
                    self.decimal_data4 = self.decimal_data4[5:]
                    self.hex_data4 = self.hex_data4[5:]
                else:
                    self.decimal_data4 = []
                    self.hex_data4 = []
                loaded = len(self.decimal_data4)
                preview = ", ".join(self.hex_data4[:5]) + ("..." if len(self.hex_data4) > 5 else "")
            elif which == 5:
                if len(self.decimal_data5) > 5:
                    self.decimal_data5 = self.decimal_data5[5:]
                    self.hex_data5 = self.hex_data5[5:]
                else:
                    self.decimal_data5 = []
                    self.hex_data5 = []
                loaded = len(self.decimal_data5)
                preview = ", ".join(self.hex_data5[:5]) + ("..." if len(self.hex_data5) > 5 else "")
            elif which == 6:
                if len(self.decimal_data6) > 5:
                    self.decimal_data6 = self.decimal_data6[5:]
                    self.hex_data6 = self.hex_data6[5:]
                else:
                    self.decimal_data6 = []
                    self.hex_data6 = []
                loaded = len(self.decimal_data6)
                preview = ", ".join(self.hex_data6[:5]) + ("..." if len(self.hex_data6) > 5 else "")
            else:  # which == 7
                if len(self.decimal_data7) > 5:
                    self.decimal_data7 = self.decimal_data7[5:]
                    self.hex_data7 = self.hex_data7[5:]
                else:
                    self.decimal_data7 = []
                    self.hex_data7 = []
                loaded = len(self.decimal_data7)
                preview = ", ".join(self.hex_data7[:5]) + ("..." if len(self.hex_data7) > 5 else "")
                
            if loaded:
                self.status_var.set(f"Soubor {which}: načteno {loaded} hodnot. HEX: {preview}")
                # Enable plot if at least one dataset is loaded
                if len(self.decimal_data1) or len(self.decimal_data2) or len(self.decimal_data3) or len(self.decimal_data4) or len(self.decimal_data5) or len(self.decimal_data6) or len(self.decimal_data7):
                    self.plot_button.config(state=tk.NORMAL)
                # Status message without popup window
            else:
                self.status_var.set("Nepodařilo se načíst žádná platná HEX data")
                messagebox.showerror("Chyba", "Nepodařilo se načíst žádná platná HEX data")
                
        except Exception as e:
            messagebox.showerror("Chyba", f"Chyba při načítání souboru: {str(e)}")
            self.status_var.set("Chyba při načítání souboru")
    
    def parse_ieee754_float(self, hex_strings, which):
        """Parsuje hex stringy jako 32-bit IEEE-754 float v little-endian formátu"""
        print(f"DEBUG: Parsování IEEE-754 float pro dataset {which}")
        
        # Procházíme hex stringy po 4 bytech (32-bit float)
        for i in range(0, len(hex_strings) - 3, 4):
            try:
                # Získáme 4 byty pro 32-bit float
                byte1 = int(hex_strings[i].replace('0x', '').replace('0X', '').replace('h', '').replace('H', ''), 16)
                byte2 = int(hex_strings[i+1].replace('0x', '').replace('0X', '').replace('h', '').replace('H', ''), 16)
                byte3 = int(hex_strings[i+2].replace('0x', '').replace('0X', '').replace('h', '').replace('H', ''), 16)
                byte4 = int(hex_strings[i+3].replace('0x', '').replace('0X', '').replace('h', '').replace('H', ''), 16)
                
                # Vytvoříme bytes objekt pro little-endian
                bytes_data = bytes([byte1, byte2, byte3, byte4])
                
                # Parsujeme jako IEEE-754 float (little-endian)
                float_value = struct.unpack('<f', bytes_data)[0]
                
                # Vytvoříme hex display (little-endian pořadí)
                hex_display = f"{byte1:02X}{byte2:02X}{byte3:02X}{byte4:02X}"
                
                # Uložíme do příslušného datasetu
                if which == 3:
                    self.decimal_data3.append(float_value)
                    self.hex_data3.append(hex_display)
                
                # Debug výpis pro prvních několik hodnot
                if len(self.decimal_data3) <= 5:
                    print(f"DEBUG: IEEE-754 - byty: {byte1:02X} {byte2:02X} {byte3:02X} {byte4:02X} → float: {float_value}")
                
            except (ValueError, IndexError) as e:
                print(f"DEBUG: Chyba při parsování IEEE-754 na pozici {i}: {e}")
                continue
        
        print(f"DEBUG: IEEE-754 parsování dokončeno - načteno {len(self.decimal_data3)} hodnot")
    
    def parse_steps_data(self, hex_strings, which):
        """Parsuje hex stringy pro počítání kroků - převádí HEX na int32"""
        print(f"DEBUG: Parsování HEX na int32 kroků pro dataset {which}")
        
        # Procházíme hex stringy po 4 bytech (jeden int32 = 4 byty)
        steps_count = 0
        
        for i in range(0, len(hex_strings), 4):  # Po 4 bytech
            try:
                # Zkontrolujeme, že máme dostatek bytů
                if i + 3 >= len(hex_strings):
                    break
                
                # Vezmeme 4 byty a zkombinujeme je
                byte1 = int(hex_strings[i].replace('0x', '').replace('0X', '').replace('h', '').replace('H', ''), 16)
                byte2 = int(hex_strings[i+1].replace('0x', '').replace('0X', '').replace('h', '').replace('H', ''), 16)
                byte3 = int(hex_strings[i+2].replace('0x', '').replace('0X', '').replace('h', '').replace('H', ''), 16)
                byte4 = int(hex_strings[i+3].replace('0x', '').replace('0X', '').replace('h', '').replace('H', ''), 16)
                
                # Zkombinujeme do 32-bit hodnoty (little-endian)
                # byte1 je nejnižší byte, byte4 je nejvyšší byte
                combined_hex = f"{byte1:02x}{byte2:02x}{byte3:02x}{byte4:02x}"
                bytes_data = bytes.fromhex(combined_hex)
                current_value = struct.unpack('<i', bytes_data)[0]  # little-endian signed int32
                
                # Uložíme hodnotu jako krok
                self.decimal_data4.append(current_value)
                self.hex_data4.append(f"{byte1:02x} {byte2:02x} {byte3:02x} {byte4:02x}")
                
                # Debug výpis pro prvních několik hodnot
                if len(self.decimal_data4) <= 10:
                    print(f"DEBUG: HEX krok {i//4}: {byte1} {byte2} {byte3} {byte4} → {current_value}")
                
                steps_count += 1
                
            except (ValueError, IndexError) as e:
                print(f"DEBUG: Chyba při parsování HEX kroku na pozici {i}: {e}")
                continue
        
        print(f"DEBUG: Parsování HEX kroků dokončeno - načteno {len(self.decimal_data4)} hodnot")
            
    def plot_data(self):
        if not (self.decimal_data1 or self.decimal_data2 or self.decimal_data3 or self.decimal_data4 or self.decimal_data5 or self.decimal_data6 or self.decimal_data7):
            messagebox.showerror("Chyba", "Nejsou k dispozici žádná data pro zobrazení")
            return
            
        # Create the plot window
        plot_window = tk.Toplevel(self.root)
        plot_window.title("HEX Data Plot")
        plot_window.geometry("1400x900")
        
        # Create the plot - smaller to leave more space for text widgets
        fig = plt.figure(figsize=(8, 6))
        
        # Main plot
        ax = plt.subplot(111)
        
        # Skip first N samples in plotting
        start_index = 10
        window_len = 10000
        # Uložíme si pro tooltipy
        self.plot_start_index = start_index
        self.plot_window_len = window_len
        
        # Plot dataset 1 if available and enabled (only first 10000 samples)
        lines = []
        labels = []
        if self.decimal_data1 and self.show_data1_var.get():
            # Limit to first 10000 samples, show all values
            plot_data1 = self.decimal_data1[start_index:start_index+window_len]
            line1, = ax.plot(plot_data1, '-', color='tab:blue', linewidth=1.2, marker='o', markersize=3)
            lines.append(line1)
            labels.append(f'Data 1 (10000 zobrazených z {len(self.decimal_data1)}, vynecháno prvních 10)')
            
        # Plot dataset 2 if available and enabled (only first 10000 samples)
        if self.decimal_data2 and self.show_data2_var.get():
            # Limit to first 10000 samples, show all values
            plot_data2 = self.decimal_data2[start_index:start_index+window_len]
            line2, = ax.plot(plot_data2, '-', color='tab:orange', linewidth=1.2, marker='s', markersize=3)
            lines.append(line2)
            labels.append(f'Data 2 (10000 zobrazených z {len(self.decimal_data2)}, vynecháno prvních 10)')
            
        # Plot dataset 3 if available and enabled (only first 10000 samples)
        if self.decimal_data3 and self.show_data3_var.get():
            # Limit to first 10000 samples, show all values
            plot_data3 = self.decimal_data3[start_index:start_index+window_len]
            # Posuneme data o 2050 jednotek nahoru na ose Y
            plot_data3_offset = [y + 2050 for y in plot_data3]
            line3, = ax.plot(plot_data3_offset, '-', color='tab:green', linewidth=1.2, marker='^', markersize=3)
            lines.append(line3)
            labels.append(f'Data 3 (10000 zobrazených z {len(self.decimal_data3)}) + 2050, vynecháno prvních 10')
            
        # Plot dataset 4 if available and enabled (only first 10000 samples)
        if self.decimal_data4 and self.show_data4_var.get():
            # Limit to first 10000 samples, show all values
            plot_data4 = self.decimal_data4[start_index:start_index+window_len]
            # Vykreslíme kroky bez posunu a bez násobení
            plot_data4_offset = [y for y in plot_data4]
            line4, = ax.plot(plot_data4_offset, '-', color='tab:red', linewidth=1.2, marker='D', markersize=3)
            lines.append(line4)
            labels.append(f'Data 4 - Kroky (10000 zobrazených z {len(self.decimal_data4)}), vynecháno prvních 10')
            
            # Najdeme body kde se hodnota změní a zobrazíme je jako velké růžové body na středu sinusovky
            change_points_x = []
            change_points_y = []
            
            # Pro vyznačení změn použijeme stejné okno jako pro vykreslení
            end_index = min(len(self.decimal_data4), start_index + window_len)
            for i in range(start_index + 1, end_index):
                if self.decimal_data4[i] != self.decimal_data4[i-1]:
                    change_points_x.append(i - start_index)
                    # Vykreslíme značku změny na aktuální hodnotě kroků
                    change_points_y.append(plot_data4_offset[i - start_index])
            
            # Zobrazíme velké růžové body na místech změn
            if change_points_x:
                ax.scatter(change_points_x, change_points_y, color='magenta', s=50, alpha=0.8, zorder=5)
                # Přidáme do legendy
                from matplotlib.lines import Line2D
                legend_line = Line2D([0], [0], marker='o', color='magenta', linestyle='None', markersize=8)
                lines.append(legend_line)
                labels.append(f'Data 4 - Změny kroků ({len(change_points_x)} změn)')

        # Plot dataset 5 if available and enabled (only first 10000 samples)
        if self.decimal_data5 and self.show_data5_var.get():
            # Limit to first 10000 samples, show all values
            plot_data5 = self.decimal_data5[start_index:start_index+window_len]
            line5, = ax.plot(plot_data5, '-', color='tab:purple', linewidth=1.2, marker='v', markersize=3)
            lines.append(line5)
            labels.append(f'Data 5 (10000 zobrazených z {len(self.decimal_data5)}, vynecháno prvních 10)')
            
        # Plot dataset 6 if available and enabled (only first 10000 samples)
        if self.decimal_data6 and self.show_data6_var.get():
            # Limit to first 10000 samples, show all values
            plot_data6 = self.decimal_data6[start_index:start_index+window_len]
            line6, = ax.plot(plot_data6, '-', color='tab:brown', linewidth=1.2, marker='p', markersize=3)
            lines.append(line6)
            labels.append(f'Data 6 (10000 zobrazených z {len(self.decimal_data6)}, vynecháno prvních 10)')

        # Plot dataset 7 if available and enabled (only first 10000 samples)
        if self.decimal_data7 and self.show_data7_var.get():
            plot_data7 = self.decimal_data7[start_index:start_index+window_len]
            line7, = ax.plot(plot_data7, '-', color='tab:gray', linewidth=1.2, marker='*', markersize=3)
            lines.append(line7)
            labels.append(f'Data 7 (10000 zobrazených z {len(self.decimal_data7)}, vynecháno prvních 10)')

        title_counts = []
        if self.decimal_data1 and self.show_data1_var.get():
            title_counts.append(str(len(self.decimal_data1)))
        if self.decimal_data2 and self.show_data2_var.get():
            title_counts.append(str(len(self.decimal_data2)))
        if self.decimal_data3 and self.show_data3_var.get():
            title_counts.append(str(len(self.decimal_data3)))
        if self.decimal_data4 and self.show_data4_var.get():
            title_counts.append(str(len(self.decimal_data4)))
        if self.decimal_data5 and self.show_data5_var.get():
            title_counts.append(str(len(self.decimal_data5)))
        if self.decimal_data6 and self.show_data6_var.get():
            title_counts.append(str(len(self.decimal_data6)))
        if self.decimal_data7 and self.show_data7_var.get():
            title_counts.append(str(len(self.decimal_data7)))
        
        if title_counts:
            ax.set_title('HEX Data Converted to Decimal (' + ' + '.join(title_counts) + ' values)')
        else:
            ax.set_title('HEX Data Converted to Decimal (žádná data zobrazena)')
            
        ax.set_xlabel('Index')
        ax.set_ylabel('Decimal Value')
        ax.grid(True, alpha=0.3)
        
        # Add some statistics (pouze pro zobrazené datasety)
        combined = []
        if self.decimal_data1 and self.show_data1_var.get():
            combined.extend(self.decimal_data1)
        if self.decimal_data2 and self.show_data2_var.get():
            combined.extend(self.decimal_data2)
        if self.decimal_data3 and self.show_data3_var.get():
            combined.extend(self.decimal_data3)
        if self.decimal_data4 and self.show_data4_var.get():
            combined.extend(self.decimal_data4)
        if self.decimal_data5 and self.show_data5_var.get():
            combined.extend(self.decimal_data5)
        if self.decimal_data6 and self.show_data6_var.get():
            combined.extend(self.decimal_data6)
        if self.decimal_data7 and self.show_data7_var.get():
            combined.extend(self.decimal_data7)
        mean_val = np.mean(combined)
        std_val = np.std(combined)
        min_val = np.min(combined)
        max_val = np.max(combined)
        
        stats_text = f'Mean: {mean_val:.2f}, Std: {std_val:.2f}, Min: {min_val}, Max: {max_val}'
        ax.text(0.02, 0.98, stats_text, transform=ax.transAxes, 
                verticalalignment='top', bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))
        
        # Enable zoom and pan - limit to first 10000 samples (pouze zobrazené datasety)
        max_len = 0
        if self.decimal_data1 and self.show_data1_var.get():
            max_len = max(max_len, len(self.decimal_data1))
        if self.decimal_data2 and self.show_data2_var.get():
            max_len = max(max_len, len(self.decimal_data2))
        if self.decimal_data3 and self.show_data3_var.get():
            max_len = max(max_len, len(self.decimal_data3))
        if self.decimal_data4 and self.show_data4_var.get():
            max_len = max(max_len, len(self.decimal_data4))
        if self.decimal_data5 and self.show_data5_var.get():
            max_len = max(max_len, len(self.decimal_data5))
        if self.decimal_data6 and self.show_data6_var.get():
            max_len = max(max_len, len(self.decimal_data6))
        if self.decimal_data7 and self.show_data7_var.get():
            max_len = max(max_len, len(self.decimal_data7))
        
        max_len = min(10000, max_len)
        if max_len > 0:
            ax.set_xlim(0, max_len - 1)
        
        if lines:
            ax.legend(lines, labels)
        
        # Add zoom instructions
        ax.text(0.02, 0.02, 'Použijte myš pro zoom a pan', transform=ax.transAxes,
                verticalalignment='bottom', bbox=dict(boxstyle='round', facecolor='lightblue', alpha=0.8))
        
        # Add click functionality to show point values
        self.setup_click_events(ax)
        
        # Embed matplotlib plot in Tkinter window
        from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk
        canvas = FigureCanvasTkAgg(fig, plot_window)
        canvas.draw()
        canvas.get_tk_widget().pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        # Add navigation toolbar for zoom and pan
        toolbar = NavigationToolbar2Tk(canvas, plot_window)
        toolbar.update()
        
        # Right side: Scrollable text widgets for data values (only if checkbox is checked)
        if self.show_tables_var.get():
            self.create_data_text_widgets(plot_window)
        
        plt.tight_layout()
        
    def highlight_low_hysteresis_points(self, ax, data, dataset_name):
        """Zvýrazní body, které jsou součástí 7-bodové skupiny s hysterezí ≤ 40"""
        if len(data) < 7:
            print(f"DEBUG: {dataset_name} - málo dat: {len(data)} < 7")
            return
            
        print(f"DEBUG: {dataset_name} - zpracovávám {len(data)} bodů")
        
        # Počítáme kolik bodů jsme zvýraznili
        highlighted_count = 0
        
        # Vypíšeme prvních 10 hodnot pro kontrolu
        print(f"DEBUG: {dataset_name} - prvních 10 hodnot: {data[:10]}")
            
        # Procházíme data od 7. bodu (index 6)
        for i in range(6, len(data)):
            # Počítáme hysterezi (rozpětí) posledních 7 bodů
            last_7_points = data[i-6:i+1]  # i-6 až i (včetně)
            hysteresis = max(last_7_points) - min(last_7_points)
            
            # Debug výpis pro prvních několik iterací
            if i < 10:
                print(f"DEBUG: {dataset_name} - index {i}: body {last_7_points}, hystereze {hysteresis:.2f}")
            
            # Pokud je hystereze do 40, počítáme bod (ale nevykreslujeme)
            if hysteresis <= 40:
                print(f"DEBUG: {dataset_name} - NALEZEN! index {i}: hystereze {hysteresis:.2f} ≤ 40")
                highlighted_count += 1
        
        print(f"DEBUG: {dataset_name} - celkem zvýrazněno {highlighted_count} bodů")
        
        # Přidáme informaci o počtu zvýrazněných bodů do statistik
        if highlighted_count > 0:
            # Nastavíme pozici podle datasetu
            y_pos = 0.90 if dataset_name == 'Data 1' else 0.86
            ax.text(0.02, y_pos, f'Hystereze ≤ 40 {dataset_name}: {highlighted_count} bodů', 
                   transform=ax.transAxes, verticalalignment='top', 
                   bbox=dict(boxstyle='round', facecolor='lightgreen', alpha=0.8))
        else:
            print(f"DEBUG: {dataset_name} - žádné body s hysterezí ≤ 40 nenalezeny")
        
    def detect_micro_steps(self, ax, data, color, dataset_name):
        """Detekuje mikro kroky - když průměr posledních 7 bodů je do 40, vykreslí červený bod"""
        if len(data) < 7:
            return
            
        # Počítáme kolik mikro kroků jsme našli
        micro_steps_count = 0
            
        # Procházíme data od 7. bodu (index 6)
        for i in range(6, len(data)):
            # Počítáme průměr posledních 7 bodů
            last_7_points = data[i-6:i+1]  # i-6 až i (včetně)
            average = np.mean(last_7_points)
            
            # Pokud je průměr do 40, vykreslíme červený bod
            if average <= 40:
                ax.plot(i, data[i], 'ro', markersize=8, markeredgecolor='red', 
                       markeredgewidth=2, markerfacecolor='red')
                micro_steps_count += 1
        
        # Přidáme informaci o počtu nalezených mikro kroků do statistik
        if micro_steps_count > 0:
            # Nastavíme pozici podle datasetu
            y_pos = 0.88 if dataset_name == 'Data 1' else 0.82
            ax.text(0.02, y_pos, f'Mikro kroky {dataset_name}: {micro_steps_count}', 
                   transform=ax.transAxes, verticalalignment='top', 
                   bbox=dict(boxstyle='round', facecolor='lightcoral', alpha=0.8))
        
    def create_data_text_widgets(self, parent_window):
        """Create scrollable text widgets for displaying data values"""
        
        # Right side frame for text widgets
        right_frame = tk.Frame(parent_window)
        right_frame.pack(side=tk.RIGHT, fill=tk.BOTH, padx=10, pady=10)
        
        # Create horizontal frame for both datasets
        horizontal_frame = tk.Frame(right_frame)
        horizontal_frame.pack(fill=tk.BOTH, expand=True)
        
        # Dataset 1 text widget - LEFT SIDE
        if self.decimal_data1:
            # Left frame for Data 1
            left_frame = tk.Frame(horizontal_frame)
            left_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(0, 5))
            
            # Title
            tk.Label(left_frame, text=f"Data 1 - Prvních 10000 hodnot", 
                    font=("Arial", 14, "bold")).pack(anchor=tk.W)
            
            # Text widget with scrollbar
            text_frame1 = tk.Frame(left_frame)
            text_frame1.pack(fill=tk.BOTH, expand=True, pady=(5, 10))
            
            text1 = tk.Text(text_frame1, width=15, height=30, font=("Arial", 11))
            scrollbar1 = tk.Scrollbar(text_frame1, orient=tk.VERTICAL, command=text1.yview)
            text1.configure(yscrollcommand=scrollbar1.set)
            
            text1.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
            scrollbar1.pack(side=tk.RIGHT, fill=tk.Y)
            
            # Insert data values - up to 10000 values
            max_rows = min(10000, len(self.decimal_data1))
            for i in range(max_rows):
                text1.insert(tk.END, f"{self.decimal_data1[i]}\n")
            
            text1.config(state=tk.DISABLED)  # Make read-only
        
        # Dataset 2 text widget - MIDDLE SIDE
        if self.decimal_data2:
            # Middle frame for Data 2
            middle_data_frame = tk.Frame(horizontal_frame)
            middle_data_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(5, 5))
            
            # Title
            tk.Label(middle_data_frame, text=f"Data 2 - Prvních 10000 hodnot", 
                    font=("Arial", 14, "bold")).pack(anchor=tk.W)
            
            # Text widget with scrollbar
            text_frame2 = tk.Frame(middle_data_frame)
            text_frame2.pack(fill=tk.BOTH, expand=True, pady=(5, 10))
            
            text2 = tk.Text(text_frame2, width=15, height=30, font=("Arial", 11))
            scrollbar2 = tk.Scrollbar(text_frame2, orient=tk.VERTICAL, command=text2.yview)
            text2.configure(yscrollcommand=scrollbar2.set)
            
            text2.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
            scrollbar2.pack(side=tk.RIGHT, fill=tk.Y)
            
            # Insert data values - up to 10000 values
            max_rows = min(10000, len(self.decimal_data2))
            for i in range(max_rows):
                text2.insert(tk.END, f"{self.decimal_data2[i]}\n")
            
            text2.config(state=tk.DISABLED)  # Make read-only
        
        # Dataset 3 text widget - THIRD SIDE
        if self.decimal_data3:
            # Third frame for Data 3
            third_data_frame = tk.Frame(horizontal_frame)
            third_data_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(5, 5))
            
            # Title
            tk.Label(third_data_frame, text=f"Data 3 - Prvních 10000 hodnot", 
                    font=("Arial", 14, "bold")).pack(anchor=tk.W)
            
            # Text widget with scrollbar
            text_frame3 = tk.Frame(third_data_frame)
            text_frame3.pack(fill=tk.BOTH, expand=True, pady=(5, 10))
            
            text3 = tk.Text(text_frame3, width=15, height=30, font=("Arial", 11))
            scrollbar3 = tk.Scrollbar(text_frame3, orient=tk.VERTICAL, command=text3.yview)
            text3.configure(yscrollcommand=scrollbar3.set)
            
            text3.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
            scrollbar3.pack(side=tk.RIGHT, fill=tk.Y)
            
            # Insert data values - up to 10000 values
            max_rows = min(10000, len(self.decimal_data3))
            for i in range(max_rows):
                text3.insert(tk.END, f"{self.decimal_data3[i]}\n")
            
            text3.config(state=tk.DISABLED)  # Make read-only
        
        # Dataset 4 text widget - RIGHT SIDE
        if self.decimal_data4:
            # Right frame for Data 4
            right_data_frame = tk.Frame(horizontal_frame)
            right_data_frame.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=(5, 0))
            
            # Title
            tk.Label(right_data_frame, text=f"Data 4 - Kroky (prvních 10000)", 
                    font=("Arial", 14, "bold")).pack(anchor=tk.W)
            
            # Text widget with scrollbar
            text_frame4 = tk.Frame(right_data_frame)
            text_frame4.pack(fill=tk.BOTH, expand=True, pady=(5, 10))
            
            text4 = tk.Text(text_frame4, width=15, height=30, font=("Arial", 11))
            scrollbar4 = tk.Scrollbar(text_frame4, orient=tk.VERTICAL, command=text4.yview)
            text4.configure(yscrollcommand=scrollbar4.set)
            
            text4.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
            scrollbar4.pack(side=tk.RIGHT, fill=tk.Y)
            
            # Insert data values - up to 10000 values
            max_rows = min(10000, len(self.decimal_data4))
            for i in range(max_rows):
                text4.insert(tk.END, f"{self.decimal_data4[i]}\n")
            
            text4.config(state=tk.DISABLED)  # Make read-only
        
        # Dataset 5 text widget - FIFTH SIDE
        if self.decimal_data5:
            # Fifth frame for Data 5
            fifth_data_frame = tk.Frame(horizontal_frame)
            fifth_data_frame.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=(5, 0))
            
            # Title
            tk.Label(fifth_data_frame, text=f"Data 5 - Prvních 10000 hodnot", 
                    font=("Arial", 14, "bold")).pack(anchor=tk.W)
            
            # Text widget with scrollbar
            text_frame5 = tk.Frame(fifth_data_frame)
            text_frame5.pack(fill=tk.BOTH, expand=True, pady=(5, 10))
            
            text5 = tk.Text(text_frame5, width=15, height=30, font=("Arial", 11))
            scrollbar5 = tk.Scrollbar(text_frame5, orient=tk.VERTICAL, command=text5.yview)
            text5.configure(yscrollcommand=scrollbar5.set)
            
            text5.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
            scrollbar5.pack(side=tk.RIGHT, fill=tk.Y)
            
            # Insert data values - up to 10000 values
            max_rows = min(10000, len(self.decimal_data5))
            for i in range(max_rows):
                text5.insert(tk.END, f"{self.decimal_data5[i]}\n")
            
            text5.config(state=tk.DISABLED)  # Make read-only
        
        # Dataset 6 text widget - SIXTH SIDE
        if self.decimal_data6:
            # Sixth frame for Data 6
            sixth_data_frame = tk.Frame(horizontal_frame)
            sixth_data_frame.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=(5, 0))
            
            # Title
            tk.Label(sixth_data_frame, text=f"Data 6 - Prvních 10000 hodnot", 
                    font=("Arial", 14, "bold")).pack(anchor=tk.W)
            
            # Text widget with scrollbar
            text_frame6 = tk.Frame(sixth_data_frame)
            text_frame6.pack(fill=tk.BOTH, expand=True, pady=(5, 10))
            
            text6 = tk.Text(text_frame6, width=15, height=30, font=("Arial", 11))
            scrollbar6 = tk.Scrollbar(text_frame6, orient=tk.VERTICAL, command=text6.yview)
            text6.configure(yscrollcommand=scrollbar6.set)
            
            text6.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
            scrollbar6.pack(side=tk.RIGHT, fill=tk.Y)
            
            # Insert data values - up to 10000 values
            max_rows = min(10000, len(self.decimal_data6))
            for i in range(max_rows):
                text6.insert(tk.END, f"{self.decimal_data6[i]}\n")
            
            text6.config(state=tk.DISABLED)  # Make read-only

        # Dataset 7 text widget - SEVENTH SIDE
        if self.decimal_data7:
            # Seventh frame for Data 7
            seventh_data_frame = tk.Frame(horizontal_frame)
            seventh_data_frame.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=(5, 0))
            
            # Title
            tk.Label(seventh_data_frame, text=f"Data 7 - Prvních 10000 hodnot", 
                    font=("Arial", 14, "bold")).pack(anchor=tk.W)
            
            # Text widget with scrollbar
            text_frame7 = tk.Frame(seventh_data_frame)
            text_frame7.pack(fill=tk.BOTH, expand=True, pady=(5, 10))
            
            text7 = tk.Text(text_frame7, width=15, height=30, font=("Arial", 11))
            scrollbar7 = tk.Scrollbar(text_frame7, orient=tk.VERTICAL, command=text7.yview)
            text7.configure(yscrollcommand=scrollbar7.set)
            
            text7.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
            scrollbar7.pack(side=tk.RIGHT, fill=tk.Y)
            
            # Insert data values - up to 10000 values
            max_rows = min(10000, len(self.decimal_data7))
            for i in range(max_rows):
                text7.insert(tk.END, f"{self.decimal_data7[i]}\n")
            
            text7.config(state=tk.DISABLED)  # Make read-only
        
        # Instructions
        tk.Label(right_frame, text="Hodnoty lze kopírovat pomocí Ctrl+C (zobrazuje až 10000 hodnot)", 
                font=("Arial", 10), fg="blue").pack(anchor=tk.W, pady=(10, 0))
        
    def setup_click_events(self, ax):
        """Setup hover events to show point values"""
        self.current_annotation = None
        
        def on_mouse_move(event):
            if event.inaxes != ax:
                # Mouse left the plot area, remove annotation
                if self.current_annotation:
                    self.current_annotation.remove()
                    self.current_annotation = None
                    ax.figure.canvas.draw()
                return
                
            # Find the closest point in both datasets
            x_mouse = event.xdata
            y_mouse = event.ydata
            
            if x_mouse is None or y_mouse is None:
                return
                
            closest_point = None
            closest_distance = float('inf')
            dataset_info = ""
            
            # Check dataset 1 (pouze pokud je zobrazen)
            if self.decimal_data1 and self.show_data1_var.get():
                for i, y in enumerate(self.decimal_data1):
                    distance = abs(i - x_mouse) + abs(y - y_mouse) * 0.1  # Weight y-distance less
                    if distance < closest_distance:
                        closest_distance = distance
                        closest_point = (i, y)
                        dataset_info = f"Data 1: Index {i}, HEX: {self.hex_data1[i] if i < len(self.hex_data1) else 'N/A'}, Decimal: {y}"
            
            # Check dataset 2 (pouze pokud je zobrazen)
            if self.decimal_data2 and self.show_data2_var.get():
                for i, y in enumerate(self.decimal_data2):
                    distance = abs(i - x_mouse) + abs(y - y_mouse) * 0.1  # Weight y-distance less
                    if distance < closest_distance:
                        closest_distance = distance
                        closest_point = (i, y)
                        dataset_info = f"Data 2: Index {i}, HEX: {self.hex_data2[i] if i < len(self.hex_data2) else 'N/A'}, Decimal: {y}"
            
            # Check dataset 3 (pouze pokud je zobrazen)
            if self.decimal_data3 and self.show_data3_var.get():
                for i, y in enumerate(self.decimal_data3):
                    # Pro dataset 3 musíme zohlednit offset +2050
                    y_offset = y + 2050
                    distance = abs(i - x_mouse) + abs(y_offset - y_mouse) * 0.1  # Weight y-distance less
                    if distance < closest_distance:
                        closest_distance = distance
                        closest_point = (i, y_offset)
                        dataset_info = f"Data 3: Index {i}, HEX: {self.hex_data3[i] if i < len(self.hex_data3) else 'N/A'}, Decimal: {y} (zobrazeno: {y_offset})"
            
            # Check dataset 4 (pouze pokud je zobrazen)
            if self.decimal_data4 and self.show_data4_var.get():
                for i, y in enumerate(self.decimal_data4):
                    # Dataset 4 bez posunu a bez násobení
                    y_offset = y
                    distance = abs(i - x_mouse) + abs(y_offset - y_mouse) * 0.1  # Weight y-distance less
                    if distance < closest_distance:
                        closest_distance = distance
                        closest_point = (i, y_offset)
                        dataset_info = f"Data 4 - Kroky: Index {i}, HEX: {self.hex_data4[i] if i < len(self.hex_data4) else 'N/A'}, Krok: {y} (zobrazeno: {y_offset})"
            
            # Check dataset 5 (pouze pokud je zobrazen)
            if self.decimal_data5 and self.show_data5_var.get():
                for i, y in enumerate(self.decimal_data5):
                    distance = abs(i - x_mouse) + abs(y - y_mouse) * 0.1  # Weight y-distance less
                    if distance < closest_distance:
                        closest_distance = distance
                        closest_point = (i, y)
                        dataset_info = f"Data 5: Index {i}, HEX: {self.hex_data5[i] if i < len(self.hex_data5) else 'N/A'}, Decimal: {y}"
            
            # Check dataset 6 (pouze pokud je zobrazen)
            if self.decimal_data6 and self.show_data6_var.get():
                for i, y in enumerate(self.decimal_data6):
                    distance = abs(i - x_mouse) + abs(y - y_mouse) * 0.1  # Weight y-distance less
                    if distance < closest_distance:
                        closest_distance = distance
                        closest_point = (i, y)
                        dataset_info = f"Data 6: Index {i}, HEX: {self.hex_data6[i] if i < len(self.hex_data6) else 'N/A'}, Decimal: {y}"
            
            # Check dataset 7 (pouze pokud je zobrazen)
            if self.decimal_data7 and self.show_data7_var.get():
                for i, y in enumerate(self.decimal_data7):
                    distance = abs(i - x_mouse) + abs(y - y_mouse) * 0.1
                    if distance < closest_distance:
                        closest_distance = distance
                        closest_point = (i, y)
                        dataset_info = f"Data 7: Index {i}, HEX: {self.hex_data7[i] if i < len(self.hex_data7) else 'N/A'}, Decimal: {y}"
            
            # Show tooltip if we found a close point
            if closest_point and closest_distance < 1.5:  # Threshold for "close enough"
                # Remove previous annotation if it exists
                if self.current_annotation:
                    self.current_annotation.remove()
                
                # Add new annotation
                self.current_annotation = ax.annotate(dataset_info, 
                                                   xy=closest_point,
                                                   xytext=(10, 10), textcoords='offset points',
                                                   bbox=dict(boxstyle='round,pad=0.5', facecolor='yellow', alpha=0.8),
                                                   arrowprops=dict(arrowstyle='->', connectionstyle='arc3,rad=0'))
                
                # Update the plot
                ax.figure.canvas.draw()
            else:
                # No close point found, remove annotation if it exists
                if self.current_annotation:
                    self.current_annotation.remove()
                    self.current_annotation = None
                    ax.figure.canvas.draw()
        
        # Connect the mouse motion event
        ax.figure.canvas.mpl_connect('motion_notify_event', on_mouse_move)
    
    def on_format_change(self):
        """Automatically select data type when 16bit_fixed format is chosen"""
        if self.format_var.get() == "16bit_fixed":
            self.data_type_var.set("unsigned16") # Default to unsigned 16-bit
            self.toggle_byte_order() # Show byte order options
        else:
            self.data_type_var.set("unsigned") # Default to unsigned 8-bit
            self.toggle_byte_order() # Hide byte order options
    
    def load_all_datasets(self):
        """Načte všechny datasetové soubory, které mají vyplněnou cestu."""
        for which in [1, 2, 3, 4, 5, 6, 7]:
            try:
                if which == 1:
                    path = self.file_path_var1.get()
                elif which == 2:
                    path = self.file_path_var2.get()
                elif which == 3:
                    path = self.file_path_var3.get()
                elif which == 4:
                    path = self.file_path_var4.get()
                elif which == 5:
                    path = self.file_path_var5.get()
                elif which == 6:
                    path = self.file_path_var6.get()
                else:
                    path = self.file_path_var7.get()

                if not path:
                    continue

                self.load_and_convert(which)
            except Exception:
                continue
    
    def on_closing(self):
        """Handle proper application shutdown"""
        try:
            # Close all matplotlib figures
            plt.close('all')
            # Destroy the root window
            self.root.destroy()
            # Force exit the application
            import sys
            sys.exit(0)
        except:
            # If something goes wrong, force quit
            import os
            os._exit(0)
        
    def run(self):
        try:
            self.root.mainloop()
        except KeyboardInterrupt:
            self.on_closing()
        except Exception as e:
            print(f"Error in mainloop: {e}")
            self.on_closing()

if __name__ == "__main__":
    app = HexDataPlotter()
    app.run()
