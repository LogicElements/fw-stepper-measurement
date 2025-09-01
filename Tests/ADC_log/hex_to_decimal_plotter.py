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
                      value="space").pack(side=tk.LEFT)
        tk.Radiobutton(format_frame, text="Oddělené čárkami", variable=self.format_var, 
                      value="comma").pack(side=tk.LEFT)
        tk.Radiobutton(format_frame, text="Jeden HEX na řádek", variable=self.format_var, 
                      value="line").pack(side=tk.LEFT)
        
        # Data type options
        type_label = tk.Label(options_frame, text="Typ dat:")
        type_label.pack(anchor=tk.W)
        
        self.data_type_var = tk.StringVar(value="unsigned16")
        type_frame = tk.Frame(options_frame)
        type_frame.pack(fill=tk.X, pady=(0, 10))
        
        tk.Radiobutton(type_frame, text="Bez znaménka (0-255)", variable=self.data_type_var, 
                      value="unsigned", command=self.toggle_byte_order).pack(side=tk.LEFT)
        tk.Radiobutton(type_frame, text="Se znaménkem (-128 až 127)", variable=self.data_type_var, 
                      value="signed", command=self.toggle_byte_order).pack(side=tk.LEFT)
        tk.Radiobutton(type_frame, text="16-bit bez znaménka (0-65535)", variable=self.data_type_var, 
                      value="unsigned16", command=self.toggle_byte_order).pack(side=tk.LEFT)
        tk.Radiobutton(type_frame, text="16-bit se znaménkem (-32768 až 32767)", variable=self.data_type_var, 
                      value="signed16", command=self.toggle_byte_order).pack(side=tk.LEFT)
        
        # Byte order option for 16-bit values
        self.byte_order_var = tk.StringVar(value="little_endian")
        self.byte_order_frame = tk.Frame(options_frame)
        self.byte_order_frame.pack(fill=tk.X, pady=(0, 10))
        
        tk.Label(self.byte_order_frame, text="Pořadí bytů (16-bit):").pack(side=tk.LEFT)
        tk.Radiobutton(self.byte_order_frame, text="Little Endian (49 07 = 0x0749)", variable=self.byte_order_var, 
                      value="little_endian").pack(side=tk.LEFT)
        tk.Radiobutton(self.byte_order_frame, text="Big Endian (49 07 = 0x4907)", variable=self.byte_order_var, 
                      value="big_endian").pack(side=tk.LEFT)
        
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
        if self.data_type_var.get() in ["unsigned16", "signed16"]:
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
                content = file.read().strip()
                
            # Parse HEX data based on format
            if self.format_var.get() == "space":
                hex_strings = content.split()
            elif self.format_var.get() == "comma":
                hex_strings = [x.strip() for x in content.split(',')]
            else:  # line
                hex_strings = content.split('\n')
                
            # Filter out empty strings
            hex_strings = [x.strip() for x in hex_strings if x.strip()]
            
            # Convert to decimal (for selected dataset)
            if which == 1:
                self.decimal_data1 = []
                self.hex_data1 = []
            else:
                self.decimal_data2 = []
                self.hex_data2 = []
            
            # Check if we need to combine bytes (for 16-bit values)
            combine_bytes = self.data_type_var.get() in ["unsigned16", "signed16"]
            
            if combine_bytes and len(hex_strings) >= 2:
                # Process as 16-bit values (combine pairs of bytes)
                for i in range(0, len(hex_strings) - 1, 2):
                    try:
                        # Get first and second byte
                        first_byte = hex_strings[i].replace('0x', '').replace('0X', '').replace('h', '').replace('H', '')
                        second_byte = hex_strings[i + 1].replace('0x', '').replace('0X', '').replace('h', '').replace('H', '')
                        
                        if not first_byte or not second_byte:
                            continue
                            
                        # Combine bytes based on byte order
                        if self.byte_order_var.get() == "little_endian":
                            # Little Endian: first_byte is lower, second_byte is upper
                            hex_value = (int(second_byte, 16) << 8) | int(first_byte, 16)
                            hex_display = f"{second_byte}{first_byte}"
                        else:
                            # Big Endian: first_byte is upper, second_byte is lower
                            hex_value = (int(first_byte, 16) << 8) | int(second_byte, 16)
                            hex_display = f"{first_byte}{second_byte}"
                        
                        # Apply signed conversion if needed
                        if self.data_type_var.get() == "signed16" and hex_value > 32767:
                            hex_value = hex_value - 65536
                            
                        if which == 1:
                            self.decimal_data1.append(hex_value)
                            self.hex_data1.append(hex_display)
                        else:
                            self.decimal_data2.append(hex_value)
                            self.hex_data2.append(hex_display)
                        
                    except ValueError:
                        continue
                        
                # If odd number of values, add the last one as 8-bit
                if len(hex_strings) % 2 == 1:
                    try:
                        last_hex = hex_strings[-1].replace('0x', '').replace('0X', '').replace('h', '').replace('H', '')
                        if last_hex:
                            hex_value = int(last_hex, 16)
                            if self.data_type_var.get() == "signed16" and hex_value > 127:
                                hex_value = hex_value - 256
                            if which == 1:
                                self.decimal_data1.append(hex_value)
                                self.hex_data1.append(last_hex)
                            else:
                                self.decimal_data2.append(hex_value)
                                self.hex_data2.append(last_hex)
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
        hysteresis_threshold = 40
        
        # Limit analysis to first 1500 samples
        max_samples = min(1500, len(data))
        
        # Counter for consecutive microstep detections
        consecutive_count = 0
        
        # Track the last green point in each cluster
        last_green_in_cluster = None
        
        for i in range(window_size - 1, max_samples):
            # Skip points with values outside range 950-3150
            if data[i] < 950 or data[i] > 3150:
                continue
                
            # Get last 7 values
            window_data = data[i - window_size + 1:i + 1]
            
            # Check if any value in window is outside range 950-3150
            if any(val < 950 or val > 3150 for val in window_data):
                continue
            
            # Calculate mean of the window
            mean_val = np.mean(window_data)
            
            # Check if all values are within hysteresis threshold
            all_within_threshold = all(abs(val - mean_val) <= hysteresis_threshold for val in window_data)
            
            if all_within_threshold:
                consecutive_count += 1
                # Always plot green point for any microstep detection
                ax.plot(i, data[i], 'o', color='green', markersize=4, 
                       label=f'{dataset_name} Detekce' if i == window_size - 1 else "")
                
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
            ax.plot([], [], 'o', color='green', markersize=4, label=f'{dataset_name} Detekce (prvních 1500)')
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
