#include "math.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define PLAYERNUMBER 4

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

struct player { int elo; String name; };

player players[PLAYERNUMBER] = { { 900, "THO"}, {1000, "MAR"}, {1100, "JAK"}, {1200, "ANN"} };

int state = 0; //states: 0: list 1: select winner 2: select loser

int list_pointer = 0; //highlighted

const int K = 15;

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
  players[loser].elo = (int)(el_l + K*(1 - E_l));
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

void setup() {
  // put your setup code here, to run once:
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(1000);

}

void loop() {
  // put your main code here, to run repeatedly:
  switch(state)
  {
    case 0:
      // title: hall of fame
      sort_by_elo();
      display_players("hall of fame", players,PLAYERNUMBER, list_pointer, true);
      // toDo scroallable list of players sorted by elo
      // on enter: go to state 1
    break;
    case 1:
      // title: select winner
      // toDo scrollable list of players by elo, one highlighted is the selected winner
      // on enter: save highlighted as winner, proceed to 2
    break;
    case 2:
      // title: select loser
      // toDo scrollable list of players by alphabet without winner, one highlighted is the selected loser
      // on enter: save highlighted as loser. compute and enter new elos. go to state 0
    break; 
    default:
      break;
      
  }

  list_pointer++;
  list_pointer = list_pointer>=PLAYERNUMBER?0:list_pointer;
  delay(1000);

}
