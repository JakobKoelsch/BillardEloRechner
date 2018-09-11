#include "math.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include <Encoder.h>
#define PLAYERNUMBER 25

#define OLED_RESET 4

Adafruit_SSD1306 display(OLED_RESET);
Encoder scrollEnc(5, 6);

// for further reference: http://cdn-reichelt.de/documents/datenblatt/F100/402097STEC12E08.PDF

struct player { uint16_t elo; String name; int id;};

player players[PLAYERNUMBER] = { {1000, "FRE", 0}, {1000, "HEN", 1}, {1000, "ANE", 2}, {1000, "MRC", 3}, {1000, "ANN", 4}, {1000, "ALX", 5}, {1000, "JAK", 6}, {1000, "TOB", 7},
                                 {1000, "A.B", 8}, {1000, "THO", 9}, {1000, "OLL", 10},{1000, "CHR", 11},{1000, "ERM", 12},{1000, "STE", 13},{1000, "SVE", 14},{1000, "SAN", 15},
                                 {1000, "AND", 16},{1000, "LUK", 17},{1000, "S.C", 18},{1000, "JAN", 19},{1000, "DEN", 20},{1000, "S.H", 21},{1000, "MOE", 22},{1000, "IMA", 23},
                                 {1000, "JOS", 24} };
int winner;
int loser;
int l_diff;
int w_diff;

int state = 0; //states: 0: list 1: select winner 2: select loser

int button = 0;

int prev_button = 1;

int list_pointer = 0; //highlighted
long old_scroll_position = 0;
#define scroll_threshold 3
int scroll_up_counter = 0;
int scroll_down_counter = 0;

bool draw_new = true;

const int K = 45;

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

void sort_by_elo()
{
  bubbleSort();
  //quickSort(0, PLAYERNUMBER-1); //needs too much memory
}

void bubbleSort() {
      bool swapped = true;
      int j = 0;
      player tmp;
      while (swapped) {
            swapped = false;
            j++;
            for (int i = 0; i < PLAYERNUMBER - j; i++) {
                  if (players[i].elo < players[i + 1].elo) {
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
      player tmp;
      int pivot = (left + right) / 2;
      /* partition */
      while (i <= j) 
      {
            while (players[i].elo > players[pivot].elo)
                  i++;
            while (players[j].elo < players[pivot].elo)
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

  double el_w = players[winner].elo;
  double el_l = players[loser].elo;

  double E_w = Q(el_w)/(Q(el_w)+Q(el_l));
  double E_l = Q(el_l)/(Q(el_w)+Q(el_l));

  int old_w_elo = players[winner].elo;
  int old_l_elo = players[loser].elo;

  players[winner].elo = (int)(el_w + K*(1 - E_w))+1;
  players[loser].elo = (int)(el_l - K*(1 - E_l))+1;

  w_diff = old_w_elo - players[winner].elo;
  l_diff = players[loser].elo - old_l_elo;
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
  delay(3000);
}

void bestaetigungsscreen()
{
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.clearDisplay();
  display.println(players[winner].name+"\nwins vs\n"+players[loser].name);
  display.display();
  delay(3000);
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
