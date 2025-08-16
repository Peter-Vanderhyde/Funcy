# test_depth_progression.fy — Test depth progression and theme changes
import "config.fy";
import "map.fy";

print("Testing depth progression and theme changes...");

# Test theme determination for various depths
for depth = 1, depth <= 15, depth += 1 {
    theme = getThemeForDepth(depth);
    print("Depth " + str(depth) + " -> Theme: " + theme);
}

print("\nTesting theme boundaries:");
print("Depth 4 should be catacombs: " + getThemeForDepth(4));
print("Depth 5 should be caves: " + getThemeForDepth(5));
print("Depth 9 should be caves: " + getThemeForDepth(9));
print("Depth 10 should be forge: " + getThemeForDepth(10));

print("\nDepth progression test complete!");
