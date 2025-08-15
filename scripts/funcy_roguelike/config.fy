# config.fy — tunables and colors

# Map size
MAP_W = 64;
MAP_H = 24;

# Spawns per floor
MONSTERS_PER_FLOOR = 8;
ITEMS_PER_FLOOR = 6;

# Vision
FOV_ENABLED = true;
FOV_RADIUS = 6;

# Color
COLOR_ENABLED = true;
C_RESET    = "\e[0m";
C_WALL     = "\e[37m";  # light gray (visible walls)
C_FLOOR    = "\e[37m";  # light gray (visible floors)
C_WALL_DIM = "\e[90m";  # dark gray (remembered walls)
C_PLAYER   = "\e[97m";
C_MON      = "\e[91m";
C_POTION   = "\e[95m";
C_WEAPON   = "\e[96m";
C_EXIT     = "\e[93m";
C_TRADER   = "\e[92m";

# Equipment
MAX_RING_SLOTS = 2;
ARMOR_BASE_REDUCTION = 0;

# Leveling
LEVEL_XP_BASE = 10;
LEVEL_XP_SCALE = 5;
XP_PER_KILL_BASE = 3;

# Economy & shops
GOLD_START = 0;
REST_SHOP_CHANCE = 75;     # % chance to see rest-shop on descent
TRADER_SPAWN_CHANCE = 70;  # % chance a trader 'T' appears in-level
SHOP_OFFER_COUNT = 5;

PRICE_POTION_BASE    = 12; # + (heal-5)
PRICE_WEAPON_PER_ATK = 10;
PRICE_ARMOR_PER_DEF  = 12;
PRICE_RING_BASE      = 25;
