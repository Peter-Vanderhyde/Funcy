# pattern_scalable.fy - The Ultimate Math Benchmark

print("\e[H\e[J"); 
print("--- Funcy Pattern Gallery ---");
print("1. Interference Ripple");
print("2. Pulsating Ring");
print("3. Hypnotic Starburst");
print("4. Twisting Kaleidoscope");
choice = input("Enter your choice (1-4) -> ");

# --- Configuration ---
# Adjust this scale variable to change the dimensions!
# 1.0 = 60x20 | 2.0 = 120x40 | 0.5 = 30x10
scale = 1.0; 

width = int(60.0 * scale);
height = int(20.0 * scale);
iterations = 80;

shades = [" ", ".", ":", "-", "=", "+", "*", "#", "%", "@"];
num_shades_float = float(length(shades)) - 1.0;
num_shades_int = length(shades) - 1;

showCursor(false);
start_time = time();

for frame = 0, frame < iterations, frame += 1 {
    print("\e[H"); 
    f_frame = float(frame);
    
    for y = 0, y < height, y += 1 {
        row = "";
        for x = 0, x < width, x += 1 {
            
            # Explicitly force floats for division and coordinates
            nx = (float(x) - float(width) / 2.0) / (10.0 * scale);
            ny = (float(y) - float(height) / 2.0) / (5.0 * scale);
            dist = math.sqrt(math.pow(nx, 2.0) + math.pow(ny, 2.0));
            
            value = 0.0;
            
            if choice == "1" {
                value = math.sin(dist * 6.0 - (f_frame / 2.0)) * math.cos(nx * 3.0);
            } elif choice == "2" {
                ring_radius = 2.0 + math.sin(f_frame / 4.0);
                diff = math.abs(dist - ring_radius);
                value = math.cos(diff * 4.0 - f_frame);
            } elif choice == "3" {
                geo_x = math.sin(nx * 8.0);
                geo_y = math.cos(ny * 8.0);
                value = math.cos(geo_x + geo_y - dist * 10.0 + f_frame);
            } else {
                # Twisting Kaleidoscope
                hyperbola = math.abs(nx * ny);
                value = math.sin(hyperbola * 50.0 - f_frame / 2.0);
            }
            
            # Normalize to 0.0 - 1.0, then map to shades array
            normalized = (value + 1.0) / 2.0;
            index = int(math.round(normalized * num_shades_float));
            
            index = max(0, min(num_shades_int, index));
            
            row += shades[index];
        }
        draw(row);
    }
}

end_time = time();
showCursor(true);

# Benchmark calculations
total_time = end_time - start_time;
float_avg = float(total_time) / float(iterations);
avg_time = math.round(float_avg * 100.0) / 100.0;

print("\n--- Benchmark Complete ---");
print("Rendered " + str(iterations) + " frames at " + str(width) + "x" + str(height) + ".");
print("Total execution time: " + str(total_time) + " ms");
print("Average time per frame: " + str(avg_time) + " ms");