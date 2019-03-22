#include "monster.h"

#define MAP_WIDTH 40
#define MAP_HEIGHT 30
char collision = 0;

//game structure
struct monster_game_t {
    uint8_t id; //ID of game
};

//struct that stores information to generate a building
//note that the x values are positive but the y values are inverted
//example: a max_y of 10 will place the wall at MAX_max_y-10
//to create a building:
//1 - define it
//2 - declare parameters
//3 - test collision on it when moving
typedef struct building_t{
    char x; //distance from left of screen to close wall
    char y; //distance from top of screen to close wall
    char max_x; //distance from left of screen to far wall
    char max_y; //distance from top of screen to far wall
    char dx; //x coordinate of door
    char dy; //y coordinate of door
}building_t;

//item structure
typedef struct item_t{
    char x; //x coordinate
    char y; //y coordinate
    char status; //1 for on the ground, 0 for already picked up by player
    char c; //appearance
}item_t;


//struct to store player location, status, and inventory
typedef struct player_t{
    char c; ///< Displayed character ("sprite")
    char x; ///< x position (increasing from left to right)
    char y; ///< y position (increasing from top to bottom)
    int health; //number of hit points
    int def; //level of defense
    int dam; //amount of damage
    int health_potion; //number of health potions held
    int dam_potion; //number of damage potions held
    char buff; //determines whether player is under effect of damage potion
    int kills; //number of monsters killed
} player_t;

typedef struct enemy_t{
    int health; //enemy health
    int dam; //enemy damage
}enemy_t;


//general functions

static void Callback(int argc, char * argv[]);
static void Receiver(uint8_t c);
static void Play(void);
static void Help(void);
static void ClearLine(int x, int line);

//game specific functions
static void MoveLeft(void);
static void MoveRight(void);
static void MoveUp(void);
static void MoveDown(void);
static void DrawBuilding(building_t building);
static void CollisionDown(building_t building, player_t player);
static void CollisionUp(building_t building, player_t player);
static void CollisionLeft(building_t building, player_t player);
static void CollisionRight(building_t building, player_t player);
static void MapInit(void);
static void SpawnItems(void);
static void GetItem(void);
static void StartBattle(void);
static void CheckPortal(void);
static void BattleReceiver(uint8_t c);
static void attack(void);
static void block(void);
static void health(void);
static void damage(void);
static void WinBattle(void);
static void Death(void);
static void Replay(void); // loads back to town after winning a battle



static struct monster_game_t game;
static struct building_t b1;
static struct building_t b2;
static struct item_t weapon; //increases player damage
static struct item_t armor; //increases player defense
static struct item_t health_potion; //heals player upon use
static struct item_t shield; //enables blocking
static struct item_t dam_potion; //increases damage for 1 turn
static struct player_t player; //character location and status
static struct enemy_t enemy; //enemy health and damage

//enemy art
char *cyclops[] = {"            _......._",
                   "        .-'.'.'.'.'.'.`-.",
                   "      .'.'.'.'.'.'.'.'.'.`.",
                   "     /.'.'               '.\ ",
                   "     |.'    _.--...--._     |",
                   "     \    `._.-.....-._.'   /",
                   "     |     _..- .-. -.._   |",
                   "  .-.'    `.   ((@))  .'   '.-.",
                   " ( ^ \      `--.   .-'     / ^ )",
                   "  \  /         .   .       \  /",
                   "  /          .'     '.  .-    \ ",
                   " ( _.\    \ (_`-._.-'_)    /._\)",
                   "  `-' \   ' .--.          / `-'",
                   "      |  / /|_| `-._.'\   |",
                   "      |   |       |_| |   /-.._",
                   "   ..-\   `.--.______.'  |",
                   "       \       .....     |",
                   "        `.  .'      `.  /",
                   "          \           .'",
                   "           `-..___..-`         ",
                                    '\0'};

char *dragon[] = {
" <>=======() ",
"(/\\___   /|\\\         ()==========<>_ ",
"      \\_/ | \\\       //|\   ______/ \) ",
"        \\_|  \\\     // | \_/ ",
"         \\|\\/|\\_  //  /\/ ",
"           (oo)\ \_//  / ",
"          //_^_) /  | ",
"         @@/  |=\  \  | ",
"              \\_=\\_ \\ | ",
"                \\==\\ \\|\_ ",
"            __(|===|(  )} ",
"            (((~) __(_/ | \\",
"                 (((~) \\  / ",
"                 ______/ / ",
"                 '------' ",
'\0'};


char *gryphon[] = {
"                       ______ ",
"             ______,---'__,---' ",
"         _,-'---_---__,---' ",
"  /_    (,  ---____', ",
" /  ',,   `, ,-' ",
";/)   ,',,_/,' ",
"| /\\   ,.'//\\ ",
"`-` \\ ,,'    `. ",
"     `',   ,-- `. ",
"     '/ / |      `,         _ ",
"     //'',.\\_    .\\\\      ,{==>- ",
"  __//   __;_`-  \\ `;.__,;' ",
"((,--,) (((,------;  `--' ",
"```  '   ```' ",
'\0'};


char *death[] = {
"                             ,--. ",
"                            {    } ",
"                            K,   } ",
"                           /  `Y` ",
"                      _   /   / ",
"                     {_'-K.__/ ",
"                      `/-.__L._ ",
"                      /  ' /`\_} ",
"                     /  ' /     ",
"             ____   /  ' / ",
"      ,-'~~~~    ~~/  ' /_ ",
"    ,'             ``~~~%%', ",
"   (                     %  Y ",
"  {                      %% I ",
" {      -                 %  `. ",
"|       ',                %  ) ",
"|        |   ,..__      __. Y ",
"|    .,_./  Y ' / ^Y   J   )| ",
" \\           |' /   |   |   || ",
"  \\          L_/    . _ (_,.'( ",
"   \\,   ,      ^^""' / |      ) ",
"    \\_  \\          /,L]     / ",
"      '-_`-,       ` `   ./` ",
"         `-(_            ) ",
"             ^^\\..___,.--` ",
'\0'};

void monsterGame_init(void)
{
    //register game system
    Game_ClearScreen();
    game.id = Game_Register("MON", "Monster Adventure Game", Play, Help);
    Game_RegisterCallback(game.id, Callback); //support extra callbacks
}

void Help(void)
{
    Game_Printf("Use WASD to move. Encounters will prompt you for attacks\r\n");
}

void Callback(int argc, char * argv[]) {
    // "play" and "help" are called automatically so just process "reset" here
    if(argc == 0) Game_Log(game.id, "too few args");
    if(strcasecmp(argv[0],"reset") == 0) {
        // reset scores
        Game_Log(game.id, "Scores reset");
    }else Game_Log(game.id, "command not supported");
}

void MapInit(void)
{
    Game_DrawRect(0, 0, MAP_WIDTH, MAP_HEIGHT);
    if (player.kills < 1)
    {
    SpawnItems();
    }
    else if (player.kills >=1 )
    {
        health_potion.x = random_int(1, MAP_WIDTH-1);
        health_potion.y = random_int(1, MAP_HEIGHT-1);
        health_potion.status = 1;
        health_potion.c = '+';
        Game_CharXY(health_potion.c, health_potion.x, health_potion.y);
    }
    //building 1
    b1.x = 5;
    b1.y = 5;
    b1.max_x = 10;
    b1.max_y = 10;
    b1.dx = b1.max_x;
    b1.dy = b1.y+(b1.y/2);
    DrawBuilding(b1);

    //building 2
    b2.x = 20;
    b2.y = 20;
    b2.max_x = 25;
    b2.max_y = 25;
    b2.dx = b2.x;
    b2.dy = b2.y+((b2.max_y-b2.y)/2);
    DrawBuilding(b2);

    Game_CharXY(' ', 18, MAP_HEIGHT-1);
    Game_Printf("****");

}

void Play(void)
{
    Game_ClearScreen();
    MapInit();
    player.x = 1;
    player.y = 1;
    player.c = '&';
    player.health = 50;
    player.dam = 1;
    player.def = 0;
    player.health_potion = 0;
    player.dam_potion = 0;
    player.buff = 0;
    player.kills = 0;


    Game_CharXY(player.c, player.x, player.y);
    Game_RegisterPlayer1Receiver(Receiver);
    Game_HideCursor();
}

void Replay(void)
{
    Game_ClearScreen();
    MapInit();
    player.x = 1;
    player.y =1;
    player.c = '&';
    player.buff = 0;
    Game_CharXY(player.c, player.x, player.y);
    Game_RegisterPlayer1Receiver(Receiver);
}

void Receiver(uint8_t c)
{
    switch (c){
    case 'w':
    case 'W':
        CollisionUp(b1, player);
        if (collision != 1){
        CollisionUp(b2, player);
        }
        if (collision == 0){
        MoveUp();
        GetItem();
        }
        break;
    case 'a' :
    case 'A' :
        CollisionLeft(b1, player);
        if (collision != 1){
        CollisionLeft(b2, player);
        }
        if (collision == 0){
        MoveLeft();
        GetItem();
        CheckPortal();
        }
        break;
    case 's':
    case 'S':
        CollisionDown(b1, player);
        if (collision != 1){
        CollisionDown(b2, player);
        }
        if (collision == 0){
        MoveDown();
        GetItem();
        CheckPortal();
        }
        break;
    case 'd' :
    case 'D' :
        CollisionRight(b1, player);
        if (collision != 1){
        CollisionRight(b2, player);
        }
        if (collision == 0){
        MoveRight();
        GetItem();
        CheckPortal();
        }
        break;
    default:
        break;
    }

}


void MoveDown(void){
    if (player.y < MAP_HEIGHT-1){
        Game_CharXY(' ', player.x, player.y);
        player.y++;
        Game_CharXY(player.c, player.x, player.y);
    }
}

void MoveUp(void){
    if (player.y > 1){
        Game_CharXY(' ', player.x, player.y);
        player.y--;
        Game_CharXY(player.c, player.x, player.y);
    }
}

void MoveLeft(void){
    if (player.x > 1){
        Game_CharXY(' ', player.x, player.y);
        player.x--;
        Game_CharXY(player.c, player.x, player.y);
    }
}

void MoveRight(void){
    if (player.x < MAP_WIDTH-1){
        Game_CharXY(' ', player.x, player.y);
        player.x++;
        Game_CharXY(player.c, player.x, player.y);
    }
}


void DrawBuilding(building_t building)
{
    Game_DrawRect(building.x, building.y, building.max_x, building.max_y);
    Game_CharXY(' ', building.dx, building.dy);
}

void ClearLine(int x, int line)
{
    Game_CharXY(' ', x, line);
    Game_Printf("                                                                              ");
    Game_CharXY(' ', x, line);
}

void CollisionDown(building_t building, player_t player)
{
    if (player.x == building.dx && (building.dy-1 <= player.y && building.dy+1 >= player.y)) //door
    {
       collision = 0;
       MoveDown(); //move through doorway to prevent glitch through door frame
    }
    else if (player.x >= building.x && player.x <= building.max_x && player.y == building.y-1) //exterior collision
    {
        collision = 1;
    }
    else if (player.x >= building.x+1 && player.x <= building.max_x-1 && player.y == building.max_y-1) //interior collision
    {
        collision = 1;
    }
    else //no collision
    {
        collision = 0;
    }
}


void CollisionUp(building_t building, player_t player)
{
    if (player.x == building.dx && (building.dy-1 <= player.y && building.dy+1 >= player.y)) //door
    {
       collision = 0;
       MoveUp();
    }
    else if (building.x <= player.x && building.max_x >= player.x && player.y == building.max_y+1) //exterior collision
    {
        collision = 1;
    }
    else if (building.x+1 <= player.x && building.max_x-1 >= player.x && player.y == building.y+1) //interior collision
    {
        collision = 1;
    }

    else //no collision
    {
        collision = 0;
    }
}


void CollisionLeft(building_t building, player_t player)
{
    if (player.y == building.dy && (building.dx-1 <= player.x && building.dx+1 >= player.x)) //door
    {
       collision = 0;
       MoveLeft();
    }
    else if (building.y <= player.y && building.max_y >= player.y && player.x == building.max_x+1) //exterior collision
    {
        collision = 1;
    }
    else if (player.x == building.x+1 && building.y <= player.y && player.y <= building.max_y) //interior collision
    {
        collision = 1;
    }
    else //no collision
    {
        collision = 0;
    }
}


void CollisionRight(building_t building, player_t player)
{
    if (player.y == building.dy && (building.dx-1 <= player.x && building.dx+1 >= player.x)) //door
    {
       collision = 0;
       MoveRight();
    }
    else if (building.y <= player.y && building.max_y >= player.y && player.x == building.x-1) //exterior collision
    {
        collision = 1;
    }
    else if (player.x == building.max_x-1 && building.y <= player.y && player.y <= building.max_y) //interior collision
    {
        collision = 1;
    }
    else //no collision
    {
        collision = 0;
    }
}

void SpawnItems(void)
{
    weapon.x = random_int(1, MAP_WIDTH-1);
    weapon.y = random_int(1, MAP_HEIGHT-1);
    weapon.status = 1;
    weapon.c = 'L';
    Game_CharXY(weapon.c, weapon.x, weapon.y);

    armor.x = random_int(1, MAP_WIDTH-1);
    armor.y = random_int(1, MAP_HEIGHT-1);
    armor.status = 1;
    armor.c = '8';
    Game_CharXY(armor.c, armor.x, armor.y);

    health_potion.x = random_int(1, MAP_WIDTH-1);
    health_potion.y = random_int(1, MAP_HEIGHT-1);
    health_potion.status = 1;
    health_potion.c = '+';
    Game_CharXY(health_potion.c, health_potion.x, health_potion.y);

    dam_potion.x = random_int(1, MAP_WIDTH-1);
    dam_potion.y = random_int(1, MAP_HEIGHT-1);
    dam_potion.status = 1;
    dam_potion.c = '^';
    Game_CharXY(dam_potion.c, dam_potion.x, dam_potion.y);

    shield.x = random_int(1, MAP_WIDTH-1);
    shield.y = random_int(1, MAP_HEIGHT-1);
    shield.status = 1;
    shield.c = 'O';
    Game_CharXY(shield.c, shield.x, shield.y);
}

static void GetItem(void)
{
     if (player.x == weapon.x && player.y == weapon.y && weapon.status == 1)
     {
         weapon.status = 0;
         player.dam += 5;
         ClearLine(0, MAP_HEIGHT+1);
         Game_Printf("You acquire a sword and increase your damage by 5.");
     }
     else if (player.x == armor.x && player.y == armor.y && armor.status == 1)
     {
         armor.status = 0;
         player.def += 5;
         ClearLine(0, MAP_HEIGHT+1);
         Game_Printf("You acquire some armor and increase your defense by 5.");
     }

     else if (player.x == health_potion.x && player.y == health_potion.y && health_potion.status == 1)
      {
         int num = random_int(2, 5);
         health_potion.status = 0;
         player.health_potion += num;
         ClearLine(0, MAP_HEIGHT+1);
         Game_Printf("You acquire %i", num);
         Game_Printf(" health potions for use during battle.");
      }
     else if (player.x == shield.x && player.y == shield.y && shield.status == 1)
      {
          shield.status = 0;
          ClearLine(0, MAP_HEIGHT+1);
          Game_Printf("You acquire a shield, and with it the ability to block attacks.");
      }
     else if (player.x == dam_potion.x && player.y == dam_potion.y && dam_potion.status == 1)
       {
           dam_potion.status = 0;
           player.dam_potion += 5;
           ClearLine(0, MAP_HEIGHT+1);
           Game_Printf("You acquire 5 damage potions for use during battle.");
       }

}

void CheckPortal(void)
{
    if (player.x >= 18 && player.x <= 22 && player.y == MAP_HEIGHT-1)
    {
        StartBattle();
    }
}

void BattleReceiver(uint8_t c)
{
    switch (c){
    case 'a':
    case 'A':
        attack();//attack
        break;
    case 's' :
    case 'S' :
        block();//block
        break;
    case 'd':
    case 'D':
        health();//health potion
        break;
    case 'f' :
    case 'F' :
        damage();//damage potion
        break;
    default:
        break;
    }

}

void StartBattle(void)
{
    Game_ClearScreen();
    int num = random_int(1, 3);
    switch (num){
    case 1:
        Game_DrawTile(cyclops, 5, 2);
        enemy.health = 100;
        enemy.dam = 5;
        break;
    case 2:
        Game_DrawTile(dragon, 5, 2);
        enemy.health = 75;
        enemy.dam = 3;
        break;
    case 3:
        Game_DrawTile(gryphon, 5, 2);
        enemy.health = 50;
        enemy.dam = 1;
        break;
    default:
        break;
    }

    ClearLine(1, 23);
    Game_Printf("A: Attack");
    ClearLine(1, 24);
    Game_Printf("S: Block");
    ClearLine(1, 25);
    Game_Printf("D: Use Health Potion");
    ClearLine(1, 26);
    Game_Printf("F: Use Damage Potion");
    ClearLine(1, 27);
    Game_Printf("Current Health: %i", player.health);
    ClearLine(1, 28);
    Game_Printf("Enemy Health: %i", enemy.health);
    Game_UnregisterPlayer1Receiver(Receiver);
    Game_RegisterPlayer1Receiver(BattleReceiver);
}

void attack(void)
{
    int damage = random_int(player.dam, player.dam+3);
    enemy.health = enemy.health - damage;
    ClearLine(1, 29);
    Game_Printf("Damage Dealt: %i", damage);
    ClearLine(1, 28);
    Game_Printf("Enemy Health: %i", enemy.health);
    if (player.buff == 1)
    {
        player.buff = 0;
        player.dam = player.dam/3;
    }
    if (enemy.health <= 0)
    {
        WinBattle();
    }

    else
    {
        int enDam = random_int(enemy.dam, enemy.dam+3); //enemy damage
        player.health = (player.health - enDam + (player.def/3));
        ClearLine(1, 27);
        Game_Printf("Current Health: %i", player.health);
    }

    if (player.health <= 0)
    {
        Death();
    }
}

void block(void)
{
    if (shield.status == 0)
    {
        ClearLine(1, 30);
        Game_Printf("You block with your shield");
    }
    else if (shield.status == 1)
    {
        ClearLine(1, 30);
        Game_Printf("Your mind wanders to the shield you left lying in the grass.");
        //take damage
        int enDam = random_int(enemy.dam, enemy.dam+3); //enemy damage
        player.health = (player.health - enDam + (player.def/3));
        ClearLine(1, 27);
        Game_Printf("Current Health: %i", player.health);

        if (player.health <= 0)
        {
             Death();
        }
    }
}

void health(void)
{
    if (player.health_potion > 0)
    {
    player.health += 10;
    player.health_potion -= 1;
    ClearLine(1, 30);
    Game_Printf("You restored 10 health.");
    ClearLine(1, 27);
    Game_Printf("Current Health: %i", player.health);
    }
    else
    {
        ClearLine(1, 30);
        Game_Printf("You don't have any health potions left!");
    }
}

void damage (void)
{
    if (player.dam_potion > 0 && player.buff == 0)
    {
        player.dam = player.dam*3;
        player.dam_potion -=1;
        player.buff = 1;
        ClearLine(1,30);
        Game_Printf("You take a potion which boosts the effectiveness of your next attack.");
        //take damage
        int enDam = random_int(enemy.dam, enemy.dam+3); //enemy damage
        player.health = (player.health - enDam + (player.def/3));
        ClearLine(1, 27);
        Game_Printf("Current Health: %i", player.health);

        if (player.health <= 0)
        {
             Death();
        }

    }
    else if (player.dam_potion == 0)
    {
        ClearLine(1, 30);
        Game_Printf("You don't have any damage potions left!");
    }
    else if (player.buff == 1)
    {
        ClearLine(1,30);
        Game_Printf("You are already under the effects of a damage potion.");
    }
}

void WinBattle(void)
{
    ClearLine(1, 30);
    Game_Printf("The enemy has been slain. Returning to town...");
    player.kills +=1;
    Game_UnregisterPlayer1Receiver(BattleReceiver);
    DelayMs(5000);
    Game_ClearScreen();
    Replay();

}

void Death(void)
{
    Game_ClearScreen();
    Game_DrawTile(death, 5, 2);
    ClearLine(1, 25);
    Game_Printf("You are dead.");
    ClearLine(1, 26);
    Game_Printf("Number of enemies taken with you: %i", player.kills);

}
