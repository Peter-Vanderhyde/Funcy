# test_monster_glyphs.fy — Test monster glyph generation for themes
import "config.fy";
import "entities.fy";

print("Testing monster glyph generation for themes...");

# Test catacombs monsters
print("\nCatacombs monsters:");
catacombs_names = ["Skeleton", "Zombie", "Ghost", "Wraith", "Specter"];
for i = 0, i < length(catacombs_names), i += 1 {
    name = catacombs_names[i];
    m = Monster(name, 0, 0, 10, 5);
    print("  " + name + " -> Glyph: '" + m.glyph + "'");
}

# Test caves monsters
print("\nCaves monsters:");
caves_names = ["Slime", "Bat", "Spider", "Cave Rat", "Fungus"];
for i = 0, i < length(caves_names), i += 1 {
    name = caves_names[i];
    m = Monster(name, 0, 0, 10, 5);
    print("  " + name + " -> Glyph: '" + m.glyph + "'");
}

# Test forge monsters
print("\nForge monsters:");
forge_names = ["Golem", "Fire Imp", "Lava Spawn", "Ash Demon", "Ember Beast"];
for i = 0, i < length(forge_names), i += 1 {
    name = forge_names[i];
    m = Monster(name, 0, 0, 10, 5);
    print("  " + name + " -> Glyph: '" + m.glyph + "'");
}

print("\nMonster glyph test complete!");
