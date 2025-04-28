import csv
import statistics

def read_csv_file(filename):
    times = []
    with open(filename, 'r') as f:
        reader = csv.reader(f)
        next(reader)  # Skip header
        for row in reader:
            if len(row) >= 2:
                try:
                    time = float(row[1])
                    times.append(time)
                except ValueError:
                    pass
    return times

def analyze_performance_data(virtual_file, function_ptr_file):
    # Read the data
    virtual_times = read_csv_file(virtual_file)
    function_ptr_times = read_csv_file(function_ptr_file)
    
    # Calculate statistics
    virtual_mean = statistics.mean(virtual_times)
    virtual_median = statistics.median(virtual_times)
    virtual_min = min(virtual_times)
    virtual_max = max(virtual_times)
    virtual_stddev = statistics.stdev(virtual_times)
    
    fp_mean = statistics.mean(function_ptr_times)
    fp_median = statistics.median(function_ptr_times)
    fp_min = min(function_ptr_times)
    fp_max = max(function_ptr_times)
    fp_stddev = statistics.stdev(function_ptr_times)
    
    # Calculate performance improvement
    improvement = (virtual_mean - fp_mean) / virtual_mean * 100
    
    # Print results
    print("Virtual Functions Statistics:")
    print(f"  Mean: {virtual_mean:.6f} µs")
    print(f"  Median: {virtual_median:.6f} µs")
    print(f"  Min: {virtual_min:.6f} µs")
    print(f"  Max: {virtual_max:.6f} µs")
    print(f"  Std Dev: {virtual_stddev:.6f} µs")
    
    print("\nFunction Pointers Statistics:")
    print(f"  Mean: {fp_mean:.6f} µs")
    print(f"  Median: {fp_median:.6f} µs")
    print(f"  Min: {fp_min:.6f} µs")
    print(f"  Max: {fp_max:.6f} µs")
    print(f"  Std Dev: {fp_stddev:.6f} µs")
    
    print(f"\nPerformance Improvement: {improvement:.2f}% (Function Pointers over Virtual Functions)")
    
    # Create ASCII art visualization
    print("\nSimple ASCII Visualization:")
    print("Average Update Time (µs)")
    print("    |")
    
    max_value = max(virtual_mean, fp_mean)
    scale = 20 / max_value  # Scale for 20 chars height
    
    v_bars = int(virtual_mean * scale)
    fp_bars = int(fp_mean * scale)
    
    for i in range(20, 0, -1):
        line = f"{(i/scale):.2f} |"
        
        if i <= v_bars:
            line += " ████ "
        else:
            line += "      "
            
        if i <= fp_bars:
            line += " ████"
        
        print(line)
    
    print("     +----------------------")
    print("      Virtual  Function")
    print("      Function Pointers")

if __name__ == "__main__":
    virtual_functions_file = "benchmark_function_ptr.csv"
    function_pointers_file = "benchmark_virtual.csv"
    
    analyze_performance_data(virtual_functions_file, function_pointers_file)