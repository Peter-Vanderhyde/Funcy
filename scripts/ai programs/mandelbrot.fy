/* Mandelbrot Scout - Interactive View Finder
   Used to find the perfect starting coordinates and zoom level.
*/

C_RESET  = "\e[0m";
C_BLACK  = "\e[30m";
C_GRAD = ["\e[38;5;18m", "\e[38;5;21m", "\e[38;5;27m", "\e[38;5;33m", "\e[38;5;43m", "\e[38;5;76m", "\e[38;5;208m", "\e[38;5;196m", "\e[38;5;198m"];
CHARS = [" ", ".", ",", "-", "~", "=", "+", "*", "%", "#", "@"];

class MandelbrotScout {
    &width = 78;
    &height = 34;
    &max_iters = 60; # Increased for better clarity while scouting
    func &MandelbrotScout() {}

    func &renderView(target_r, target_i, span_r, span_i) {
        # Clear screen
        print("\e[H\e[J");
        
        min_r = target_r - (span_r / 2.0);
        max_r = target_r + (span_r / 2.0);
        min_i = target_i - (span_i / 2.0);
        max_i = target_i + (span_i / 2.0);

        for row in range(&height // 2) {
            line = list();
            ci = min_i + (0.0 + row / (&height - 1)) * (max_i - min_i);
            for col in range(&width) {
                cr = min_r + (0.0 + col / (&width - 1)) * (max_r - min_r);
                zr = 0.0; zi = 0.0; iters = 0;
                while (zr * zr + zi * zi) <= 4.0 and iters < &max_iters {
                    tmp = zr * zr - zi * zi + cr;
                    zi = 2.0 * zr * zi + ci;
                    zr = tmp;
                    iters += 1;
                }
                if iters == &max_iters { line.append(C_BLACK + "@" + C_RESET); }
                else { line.append(C_GRAD[iters % 9] + CHARS[int((0.0 + iters / &max_iters) * 10)] + C_RESET); }
            }
            print("\e[" + str(row) + ";0H" + "".join(line));
            print("\e[" + str(&height - 1 - row) + ";0H" + "".join(line));
        }
    }
}

func main() {
    scout = MandelbrotScout();
    tr = 0.0; ti = 0.0; span = 3.0;

    while true {
        scout.renderView(tr, ti, span, span * 0.8); # 0.8 aspect ratio approx for terminal
        
        print("\e[" + str(scout.height) + ";0H");
        print("\n--- SCOUT MODE ---");
        print("Current: X=" + str(tr) + " | Zoom=" + str(span));
        in_tr = input("New X offset (Enter to keep): ");
        in_span = input("New Zoom (Span) (Enter to keep): ");
        
        if in_tr != "" { tr = float(in_tr); }
        if in_span != "" { span = float(in_span); }
    }
}

main();