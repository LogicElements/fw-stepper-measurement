import matplotlib.pyplot as plt
import numpy as np
import tkinter as tk
from tkinter import filedialog, messagebox
import os

class HexDataPlotter:
    def __init__(self):
        self.root = tk.Tk()
        self.root.title("HEX to Decimal Plotter")
        self.root.geometry("800x600")
        
        # Data storage for two datasets
        self.hex_data1 = []
        self.decimal_data1 = []
        self.hex_data2 = []
        self.decimal_data2 = []
        
        # Setup proper window closing protocol
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
        
        self.setup_ui()
        
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
        file_frame2.pack(fill=tk.X, pady=(0, 20))
        
        tk.Label(file_frame2, text="Soubor 2:").pack(side=tk.LEFT, padx=(0, 8))
        self.file_path_var2 = tk.StringVar()
        file_entry2 = tk.Entry(file_frame2, textvariable=self.file_path_var2, width=45)
        file_entry2.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 8))
        
        browse_btn2 = tk.Button(file_frame2, text="Procházet", command=lambda: self.browse_file(2))
        browse_btn2.pack(side=tk.LEFT, padx=(0, 8))
        load_btn2 = tk.Button(file_frame2, text="Načíst 2", command=lambda: self.load_and_convert(2))
        load_btn2.pack(side=tk.LEFT)
        
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
        
        # Buttons frame
        button_frame = tk.Frame(main_frame)
        button_frame.pack(fill=tk.X, pady=(0, 20))
        
        plot_btn = tk.Button(button_frame, text="Zobrazit graf", command=self.plot_data, state=tk.DISABLED)
        plot_btn.pack(side=tk.LEFT)
        
        self.plot_button = plot_btn
        
        # Status label
        self.status_var = tk.StringVar(value="Vyberte soubor s HEX daty")
        status_label = tk.Label(main_frame, textvariable=self.status_var, fg="blue")
        status_label.pack()
        
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
            else:
                self.file_path_var2.set(file_path)
            self.status_var.set(f"Vybraný soubor {which}: {os.path.basename(file_path)}")
            
    def load_and_convert(self, which: int):
        file_path = self.file_path_var1.get() if which == 1 else self.file_path_var2.get()
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
            else:
                self.decimal_data2 = []
                self.hex_data2 = []
            
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
                        else:
                            self.decimal_data2.append(hex_value)
                            self.hex_data2.append(hex_display)
                        
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
                                else:
                                    self.decimal_data2.append(hex_value)
                                    self.hex_data2.append(hex_str)
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
                        else:
                            self.decimal_data2.append(hex_value)
                            self.hex_data2.append(hex_str)
                        
                    except ValueError:
                        continue
                    
            loaded = len(self.decimal_data1) if which == 1 else len(self.decimal_data2)
            if loaded:
                if which == 1:
                    preview = ", ".join(self.hex_data1[:5]) + ("..." if len(self.hex_data1) > 5 else "")
                else:
                    preview = ", ".join(self.hex_data2[:5]) + ("..." if len(self.hex_data2) > 5 else "")
                self.status_var.set(f"Soubor {which}: načteno {loaded} hodnot. HEX: {preview}")
                # Enable plot if at least one dataset is loaded
                if len(self.decimal_data1) or len(self.decimal_data2):
                    self.plot_button.config(state=tk.NORMAL)
                # Status message without popup window
            else:
                self.status_var.set("Nepodařilo se načíst žádná platná HEX data")
                messagebox.showerror("Chyba", "Nepodařilo se načíst žádná platná HEX data")
                
        except Exception as e:
            messagebox.showerror("Chyba", f"Chyba při načítání souboru: {str(e)}")
            self.status_var.set("Chyba při načítání souboru")
            
    def plot_data(self):
        if not (self.decimal_data1 or self.decimal_data2):
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
        
        # Plot dataset 1 if available (only first 1500 samples, all values shown)
        lines = []
        labels = []
        if self.decimal_data1:
            # Limit to first 1500 samples, show all values
            plot_data1 = self.decimal_data1[:1500]
            line1, = ax.plot(plot_data1, '-', color='tab:blue', linewidth=1, marker='o', markersize=3)
            lines.append(line1)
            labels.append(f'Data 1 (prvních 1500 z {len(self.decimal_data1)})')
        # Plot dataset 2 if available (only first 1500 samples, all values shown)
        if self.decimal_data2:
            # Limit to first 1500 samples, show all values
            plot_data2 = self.decimal_data2[:1500]
            line2, = ax.plot(plot_data2, '-', color='tab:orange', linewidth=1, marker='s', markersize=3)
            lines.append(line2)
            labels.append(f'Data 2 (prvních 1500 z {len(self.decimal_data2)})')

        title_counts = []
        if self.decimal_data1:
            title_counts.append(str(len(self.decimal_data1)))
        if self.decimal_data2:
            title_counts.append(str(len(self.decimal_data2)))
        ax.set_title('HEX Data Converted to Decimal (' + ' + '.join(title_counts) + ' values)')
        ax.set_xlabel('Index')
        ax.set_ylabel('Decimal Value')
        ax.grid(True, alpha=0.3)
        
        # Add some statistics
        combined = []
        if self.decimal_data1:
            combined.extend(self.decimal_data1)
        if self.decimal_data2:
            combined.extend(self.decimal_data2)
        mean_val = np.mean(combined)
        std_val = np.std(combined)
        min_val = np.min(combined)
        max_val = np.max(combined)
        
        stats_text = f'Mean: {mean_val:.2f}, Std: {std_val:.2f}, Min: {min_val}, Max: {max_val}'
        ax.text(0.02, 0.98, stats_text, transform=ax.transAxes, 
                verticalalignment='top', bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))
        
        # Enable zoom and pan - limit to first 1500 samples
        max_len = min(1500, max(len(self.decimal_data1) if self.decimal_data1 else 0,
                      len(self.decimal_data2) if self.decimal_data2 else 0))
        ax.set_xlim(0, max_len - 1)
        
        if lines:
            ax.legend(lines, labels)
        
        # Add zoom instructions
        ax.text(0.02, 0.02, 'Použijte myš pro zoom a pan', transform=ax.transAxes,
                verticalalignment='bottom', bbox=dict(boxstyle='round', facecolor='lightblue', alpha=0.8))
        
        # Add click functionality to show point values
        self.setup_click_events(ax)
        
        # Detect and plot microsteps
        self.plot_microsteps(ax)
        
        # Embed matplotlib plot in Tkinter window
        from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk
        canvas = FigureCanvasTkAgg(fig, plot_window)
        canvas.draw()
        canvas.get_tk_widget().pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        # Add navigation toolbar for zoom and pan
        toolbar = NavigationToolbar2Tk(canvas, plot_window)
        toolbar.update()
        
        # Right side: Scrollable text widgets for data values
        self.create_data_text_widgets(plot_window)
        
        plt.tight_layout()
        
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
            tk.Label(left_frame, text=f"Data 1 - Prvních 1500 hodnot", 
                    font=("Arial", 14, "bold")).pack(anchor=tk.W)
            
            # Text widget with scrollbar
            text_frame1 = tk.Frame(left_frame)
            text_frame1.pack(fill=tk.BOTH, expand=True, pady=(5, 10))
            
            text1 = tk.Text(text_frame1, width=15, height=30, font=("Arial", 11))
            scrollbar1 = tk.Scrollbar(text_frame1, orient=tk.VERTICAL, command=text1.yview)
            text1.configure(yscrollcommand=scrollbar1.set)
            
            text1.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
            scrollbar1.pack(side=tk.RIGHT, fill=tk.Y)
            
            # Insert data values - up to 1500 values
            max_rows = min(1500, len(self.decimal_data1))
            for i in range(max_rows):
                text1.insert(tk.END, f"{self.decimal_data1[i]}\n")
            
            text1.config(state=tk.DISABLED)  # Make read-only
        
        # Dataset 2 text widget - RIGHT SIDE
        if self.decimal_data2:
            # Right frame for Data 2
            right_data_frame = tk.Frame(horizontal_frame)
            right_data_frame.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=(5, 0))
            
            # Title
            tk.Label(right_data_frame, text=f"Data 2 - Prvních 1500 hodnot", 
                    font=("Arial", 14, "bold")).pack(anchor=tk.W)
            
            # Text widget with scrollbar
            text_frame2 = tk.Frame(right_data_frame)
            text_frame2.pack(fill=tk.BOTH, expand=True, pady=(5, 10))
            
            text2 = tk.Text(text_frame2, width=15, height=30, font=("Arial", 11))
            scrollbar2 = tk.Scrollbar(text_frame2, orient=tk.VERTICAL, command=text2.yview)
            text2.configure(yscrollcommand=scrollbar2.set)
            
            text2.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
            scrollbar2.pack(side=tk.RIGHT, fill=tk.Y)
            
            # Insert data values - up to 1500 values
            max_rows = min(1500, len(self.decimal_data2))
            for i in range(max_rows):
                text2.insert(tk.END, f"{self.decimal_data2[i]}\n")
            
            text2.config(state=tk.DISABLED)  # Make read-only
        
        # Instructions
        tk.Label(right_frame, text="Hodnoty lze kopírovat pomocí Ctrl+C (zobrazuje až 1500 hodnot)", 
                font=("Arial", 10), fg="blue").pack(anchor=tk.W, pady=(10, 0))
        
    def plot_microsteps(self, ax):
        """Detect and plot microsteps based on hysteresis analysis"""
        # Analyze dataset 1 for microsteps
        if len(self.decimal_data1) >= 10:
            self.analyze_microsteps(ax, self.decimal_data1, 'Data 1', 'red')
        
        # Analyze dataset 2 for microsteps
        if len(self.decimal_data2) >= 10:
            self.analyze_microsteps(ax, self.decimal_data2, 'Data 2', 'darkred')
    
    def analyze_microsteps(self, ax, data, dataset_name, color):
        """Analyze data for microsteps using sliding window of 7 values (only first 1500 samples, excluding values outside 950-3150)"""
        window_size = 7
        hysteresis_threshold = 60
        
        # Limit analysis to first 1500 samples
        max_samples = min(1500, len(data))
        
        # Counter for consecutive microstep detections
        consecutive_count = 0
        
        # Track the last green point in each cluster
        last_green_in_cluster = None
        
        for i in range(window_size - 1, max_samples):
            # Skip points with values outside range 950-3150
            if data[i] < 550 or data[i] > 3450:
                continue
                
            # Get last 7 values
            window_data = data[i - window_size + 1:i + 1]
            
            # Check if any value in window is outside range 950-3150
            if any(val < 550 or val > 3450 for val in window_data):
                continue
            
            # Calculate mean of the window
            mean_val = np.mean(window_data)
            
            # Check if all values are within hysteresis threshold
            all_within_threshold = all(abs(val - mean_val) <= hysteresis_threshold for val in window_data)
            
            if all_within_threshold:
                consecutive_count += 1
                # Always plot green point for any microstep detection
                # ax.plot(i, data[i], 'o', color='green', markersize=4, 
                #        label=f'{dataset_name} Detekce' if i == window_size - 1 else "")
                
                # Track the last green point in this cluster
                last_green_in_cluster = (i, data[i])
                
                # Plot red point only once at the start of a stable region (after 4 consecutive detections)
                if consecutive_count == 4:
                    ax.plot(i, data[i], 'o', color=color, markersize=8, 
                           label=f'{dataset_name} Mikrokrok' if i == window_size - 1 else "")
            else:
                # Plot pink square for the last green point in the cluster before resetting
                if last_green_in_cluster and consecutive_count > 0:
                    x_pos, y_pos = last_green_in_cluster
                    ax.plot(x_pos, y_pos, 's', color='magenta', markersize=6, 
                           label=f'{dataset_name} Konec shluku' if x_pos == window_size - 1 else "")
                    last_green_in_cluster = None
                
                # Reset counter if no microstep detected
                consecutive_count = 0
        
        # Add legend entries for both types of points
        if max_samples >= window_size:
            # ax.plot([], [], 'o', color='green', markersize=4, label=f'{dataset_name} Detekce (prvních 1500)')
            ax.plot([], [], 'o', color=color, markersize=8, label=f'{dataset_name} Mikrokrok (prvních 1500)')
            ax.plot([], [], 's', color='magenta', markersize=6, label=f'{dataset_name} Konec shluku (prvních 1500)')
        
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
            
            # Check dataset 1
            if self.decimal_data1:
                for i, y in enumerate(self.decimal_data1):
                    distance = abs(i - x_mouse) + abs(y - y_mouse) * 0.1  # Weight y-distance less
                    if distance < closest_distance:
                        closest_distance = distance
                        closest_point = (i, y)
                        dataset_info = f"Data 1: Index {i}, HEX: {self.hex_data1[i] if i < len(self.hex_data1) else 'N/A'}, Decimal: {y}"
            
            # Check dataset 2
            if self.decimal_data2:
                for i, y in enumerate(self.decimal_data2):
                    distance = abs(i - x_mouse) + abs(y - y_mouse) * 0.1  # Weight y-distance less
                    if distance < closest_distance:
                        closest_distance = distance
                        closest_point = (i, y)
                        dataset_info = f"Data 2: Index {i}, HEX: {self.hex_data2[i] if i < len(self.hex_data2) else 'N/A'}, Decimal: {y}"
            
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
