#include "math.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include <Encoder.h>

#define OLED_RESET 4

#define PLAYERSIZE 9

#define PLAYERLIMIT 256

/*  
*   Player:
*   elo #16
*   name #40 (5 Buchstaben)
*   Puffer #16
*/

Adafruit_SSD1306 display(OLED_RESET);
Encoder scrollEnc(5, 6);

// for further reference: http://cdn-reichelt.de/documents/datenblatt/F100/402097STEC12E08.PDF

struct player { uint16_t elo; String name;};

players u_int8_t

int state = 0; //states: 0: init 1: menu 2: hall of fame, 3: new game, 4: add player

int8_t players[PLAYERLIMIT];
int playercount = 0;

int button = 0;

int prev_button = 1;

int list_pointer = 0; //highlighted
long old_scroll_position = 0;
#define scroll_threshold 3
int scroll_up_counter = 0;
int scroll_down_counter = 0;

bool draw_new = true;   //Neue Eingabe? globale

const int K = 45;   // "Haerte" des Elosystems


//greift auf die sortierte players struktur zu und gibt den player auf position ranking aus dem eeprom zurück
player get_player_at(int ranking)
{
  return load_player(players[i]);
}

// setzt playercount und initialisiert die players bis dahin
void init_players()
{
  int temp = 0;

  int test;

  do{
    EEPROM.get(temp++*PLAYERSIZE, test)
  } while(test != 0);

  playercount = temp;

  for(int i = 0; i<playercount; i++)
  {
    players[i] = i;
  }
}

player load_player(int id)
{
  player temp;
  EEPROM.get(id*PLAYERSIZE, temp);
  return temp;
}

int load_elo(int id)
{
  int temp;
  EEPROM.get(id*PLAYERSIZE, temp);
  return temp;
}

// leert die EEPROM
void dampfwalze()
{
  for(int i = 0; i < PLAYERLIMIT*PLAYERSIZE; i++)
  {
    EEPROM.put(i, 00x0);
  }
}

void save_player(id, player)
{

  EEPROM.put(id*PLAYERSIZE, player.elo);
  EEPROM.put(id*PLAYERSIZE+2, player.name);
}

int save_elo(id, elo)
{
  EEPROM.put(id*PLAYERSIZE, elo);
}

void sort_at_init()
{
  quickSort(0, PLAYERNUMBER-1);

}

void sort_by_elo()
{
  bubbleSort();
}

void bubbleSort() {
      bool swapped = true;
      int j = 0;
      u_int8_t tmp;
      while (swapped) {
            swapped = false;
            j++;
            for (int i = 0; i < playercount - j; i++) {
                  if (get_player_at(i).elo < get_player_at(i+1).elo) {
                        tmp = players[i];
                        players[i] = players[i + 1];
                        players[i + 1] = tmp;
                        swapped = true;
                  }
            }
      }
}

void quickSort(int left, int right) 
{
      int i = left, j = right;
      u_int8_t tmp;
      int pivot = (left + right) / 2;
      /* partition */
      while (i <= j) 
      {
            while (get_player_at(i).elo > get_player_at(pivot).elo)
                  i++;
            while (get_player_at(i).elo < get_player_at(pivot).elo)
                  j--;
            if (i <= j) {
                  tmp = players[i];
                  players[i] = players[j];
                  players[j] = tmp;
                  i++;
                  j--;
            }
      }
      /* recursion */
      if (left < j)
            quickSort(left, j);
      if (i < right)
            quickSort(i, right);
}

void display_players(String title, player data[], int dim, int pointer, bool highlight)
{
  //TODO
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(15,0);
  display.clearDisplay();
  display.println(title);
  int display_pointer = pointer > dim-3 ? dim-3:pointer-1;
  display_pointer = display_pointer<0?0:display_pointer;
  for(int i = display_pointer; i < dim && i < display_pointer + 3 ; i++)
  {
    display.println((highlight&&i==pointer?">":" ") +data[i].name + "  ---  "+data[i].elo +" ("+(1+i)+")");
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

  double el_w = load_elo(winner);
  double el_l = load_elo(loser);
  int old_w_elo = el_w;
  int old_l_elo = el_l;

  double E_w = Q(el_w)/(Q(el_w)+Q(el_l));
  double E_l = Q(el_l)/(Q(el_w)+Q(el_l));

  int new_w_elo = (int)(el_w + K*(1 - E_w))+1;
  int new_l_elo = (int)(el_l + K*(-E_l))+1;

  save_elo(winner, new_w_elo);
  save_elo(loser, new_l_elo);

  w_diff = -(old_w_elo - new_w_elo);
  l_diff = -(new_l_elo - old_l_elo);
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
  display.println(players[winner].name + " has now " + players[winner].elo + " +"+ w_diff);
  display.println("  winning vs");
  display.println(players[loser].name + " has now " + players[loser].elo + " -"+l_diff);
  display.display();
  delay(5000);
}

void bestaetigungsscreen()
{
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.clearDisplay();
  display.println(players[winner].name+"\nwins vs\n"+players[loser].name+"\n      ok?");
  display.display();
  delay(1000);
}

void handleInput()
{
  prev_button = button;
  button = analogRead(A1)>500?1:0; //Enter
  

  if(button == 1 && prev_button == 0)
  {
    draw_new = true;
    if(state == 1)
    {
      winner = list_pointer;
      list_pointer = 0;
    }
    if(state == 2)
    {
      loser = list_pointer;
      list_pointer = 0;
      if(winner==loser)
      {
        list_pointer = 0;
        return;
      }
      bestaetigungsscreen();
    }
    if(state == 3)
    {
      add_game(winner, loser);
      save();
      display_result();
      sort_by_elo(); 
    }
    state = state>=3?0:state+1;
    
    delay(500);
  }

  long scroll_position = scrollEnc.read();


  if(scroll_position-1>old_scroll_position)
  {
    scroll_up_counter++;
    scroll_down_counter = 0;
    if(scroll_up_counter>=scroll_threshold)
    {
      list_pointer = list_pointer >= PLAYERNUMBER-1?PLAYERNUMBER-1:list_pointer+1;
      old_scroll_position = scroll_position;
      draw_new = true;
      scroll_up_counter = 0;
    }
    
  }
  if(scroll_position+1<old_scroll_position)
  {
    scroll_down_counter++;
    scroll_up_counter = 0;
    if(scroll_down_counter>=scroll_threshold)
    {
      list_pointer = list_pointer == 0?0:list_pointer-1;
      old_scroll_position = scroll_position;
      draw_new = true;
      scroll_down_counter = 0;
    }
   
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.clearDisplay();
  display.println("Elorechner");
  display.display();
  delay(1500);
  load();
  //save();
  sort_by_elo();
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Champion");
  display.println("ist");
  display.display();
  delay(1500);
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("   "+players[0].name);
  display.display();
  delay(3000);

  state = 1;
}

void loop() {
  // put your main code here, to run repeatedly:

  handleInput();

  if(draw_new)
  {
    Serial.println("fresh draw");
    switch(state)
    {
      case 0:
        // title: hall of fame
        display_players("hall of fame", players, PLAYERNUMBER, list_pointer, true);
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
    draw_new = false;
  }
}
