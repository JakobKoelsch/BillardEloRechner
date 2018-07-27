#include "math.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#define PLAYERNUMBER 22

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

struct player { uint16_t elo; String name; int id;};

player players[PLAYERNUMBER] = { {1000, "FREDD", 0}, {1000, "HENRI", 1}, {1000, "ANNE ", 2}, {1000, "MARCO", 3}, {1000, "ANNIK", 4}, {1000, "ALEX ", 5}, {1000, "JAKOB", 6}, {1000, "TOBI ", 7},
                                 {1000, "MARIE", 8}, {1000, "THOMA", 9}, {1000, "OLLI ", 10},{1000, "CHRIS", 11},{1000, "ERMAN", 12},{1000, "STEFF", 13},{1000, "SVEN ", 14},{1000, "SANDR", 15},
                                 {1000, "ANDRE", 16},{1000, "LUKAS", 17},{1000, "STEPC", 18},{1000, "JAN  ", 19},{1000, "DENNI", 20},{1000, "STEPH", 21} };
int winner;

int state = 0; //states: 0: list 1: select winner 2: select loser

int button[3] = {0,0,0};

int prev_button[3] = {0,0,0};

int list_pointer = 0; //highlighted

const int K = 15;

void load()
{
  for(int i = 0; i < PLAYERNUMBER; i++)
  {
    EEPROM.get(i*2, players[i].elo);
  }
}

void save()
{
  for(int i = 0; i<PLAYERNUMBER;i++)
  {
    EEPROM.put(players[i].id*2, players[i].elo);
  }
}

int get_max_elo()
{
  int temp = 0;
  for(int i = 0; i < PLAYERNUMBER; i++)
  {
    temp = players[i].elo>temp?players[i].elo:temp;
  }
  return temp;
}

void sort_by_elo()
{
  // toDo fix a quick root sort

  int already_sorted = 0;

  player temp[PLAYERNUMBER];
  
  for(int i = get_max_elo(); i>0 && already_sorted<PLAYERNUMBER;i--)
  {
    for(int j = 0; j<PLAYERNUMBER;j++)
    {
      if(players[j].elo == i)
      {
        temp[already_sorted++] = players[j];
      }
    }
  }
  for(int i = 0; i < PLAYERNUMBER;i++)
  {
    players[i] = temp[i];
  }
}

void display_players(String title, player data[],int dim, int pointer, bool highlight)
{
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(15,0);
  display.clearDisplay();
  display.println(title);
  int display_pointer = pointer > dim-3 ? dim-3:pointer-1;
  display_pointer = display_pointer<0?0:display_pointer;
  for(int i = display_pointer; i < dim && i < display_pointer + 3 ; i++)
  {
    display.println((highlight&&i==pointer?">":" ") +data[i].name + "   ---   "+data[i].elo);
  }
  display.display();
  delay(1);
}

double Q(double x)
{
  return pow(10, x/400);
}

void add_game(int winner, int loser)
{

  double el_w = players[winner].elo;
  double el_l = players[loser].elo;

  double E_w = Q(el_w)/(Q(el_w)+Q(el_l));
  double E_l = Q(el_l)/(Q(el_w)+Q(el_l));

  players[winner].elo = (int)(el_w + K*(1 - E_w));
  players[loser].elo = (int)(el_l - K*(1 - E_l));
}

int get_player_by_name(String name)
{
  for(int i = 0; i < PLAYERNUMBER; i++)
  {
    if(players[i].name == name)
      return i;
  }
  return -1;
}

void display_result()
{
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.clearDisplay();
  display.println(players[winner].name + " has now " + players[winner].elo);
  display.println("  winning vs");
  display.println(players[list_pointer].name + " has now " + players[list_pointer].elo);
  display.display();
  delay(3000);
}

void handleInput()
{
  for(int i = 0; i<3;i++)
  {
    prev_button[i] = button[i];
  }
  button[0] = analogRead(A1)>500?1:0; //Enter
  delay(10);
  button[1] = analogRead(A0)>500?1:0; //Scroll Up
  delay(10);
  button[2] = analogRead(A2)>500?1:0; //Scroll Down

  if(button[0] == 1 && prev_button[0] == 0)
  {
    if(state == 1)
    {
      winner = list_pointer;
      list_pointer = 0;
    }
    if(state == 2)
    {
      add_game(winner, list_pointer);
      display_result();
      list_pointer = 0;
      save();
      sort_by_elo();
      
    }
    state = state>=2?0:state+1;
    
  }
  if(button[1] == 1 && prev_button[1] == 0)
  {
    list_pointer = list_pointer == 0?0:list_pointer-1;
  }
  if(button[2] == 1 && prev_button[2] == 0)
  {
    list_pointer = list_pointer >= PLAYERNUMBER-1?PLAYERNUMBER-1:list_pointer+1;
  }
  delay(10);
}

void setup() {
  // put your setup code here, to run once:
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(1000);
  load();
  sort_by_elo();
}

void loop() {
  // put your main code here, to run repeatedly:

  handleInput();
  
  switch(state)
  {
    case 0:
      // title: hall of fame
      display_players("hall of fame", players,PLAYERNUMBER, list_pointer, true);
      // toDo scroallable list of players sorted by elo
      // on enter: go to state 1
    break;
    case 1:
      // title: select winner
      // toDo scrollable list of players by elo, one highlighted is the selected winner
      display_players("select winner", players,PLAYERNUMBER, list_pointer, true);
      // on enter: save highlighted as winner, proceed to 2
    break;
    case 2:
      // title: select loser
      // toDo scrollable list of players by alphabet including winner (excluding him appears very complicated), one highlighted is the selected loser
      display_players("select loser", players, PLAYERNUMBER, list_pointer, true);
      // on enter: save highlighted as loser. compute and enter new elos. go to state 0
    break; 
    default:
      break;
      
  }
}
