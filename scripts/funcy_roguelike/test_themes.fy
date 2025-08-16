# test_themes.fy — Test script for theme system
import "config.fy";
import "map.fy";

print("Testing theme system...");

# Test theme determination
print("Depth 1 theme: " + getThemeForDepth(1));
print("Depth 5 theme: " + getThemeForDepth(5));
print("Depth 10 theme: " + getThemeForDepth(10));
print("Depth 15 theme: " + getThemeForDepth(15));

# Test map generation for each theme
print("\nTesting map generation...");

catacombs = generateMap("catacombs");
print("Catacombs map generated with " + str(length(catacombs["rooms"])) + " rooms");

caves = generateMap("caves");
print("Caves map generated with " + str(length(caves["rooms"])) + " rooms");

forge = generateMap("forge");
print("Forge map generated with " + str(length(forge["rooms"])) + " rooms");

print("\nTheme system test complete!");
