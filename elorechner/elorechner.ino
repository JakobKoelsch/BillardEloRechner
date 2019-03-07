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

struct player { uint16_t elo; char name[7];};

int state = 0; 

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

int w_diff;
int l_diff;
int winner;
int loser;


void first_load_players()
{
  // dampfwalze();

  //  player p0 = {1000, "Fredy\0"};
  //  save_player(0, p0);
  //  player p1 = {1000, "Jakob\0"};
  //  save_player(1, p1);
  //  player p2 = {1000, "toni \0"};
  //  save_player(2, p2);
  //  player p3 = {1000, "heck \0"};
  //  save_player(3, p3);
  //  player p4 = {1000, "Thom \0"};
  //  save_player(4, p4);
  //  player p5 = {1000, "Andi \0"};
  //  save_player(5, p5);
  //  player p6 = {1000, "doni \0"};
  //  save_player(6, p6);
  //  player p7 = {1000, "AlexH\0"};
  //  save_player(7, p7);
  //  player p8 = {1000, "MarcS\0"};
  //  save_player(8, p8);
  //   player p9 = {1000, "$MOE$\0"};
  //   save_player(9, p9);
  //   player p10 = {1000, "Sed12\0"};
  //   save_player(10, p10);
  //   player p11 = {1000, "_Zzz_\0"};
  //   save_player(11, p11);
  //   player p12 = {1000, "DVM  \0"};
  //   save_player(12, p12);
  //   player p13 = {1000, "Flo!!\0"};
  //   save_player(13, p13);
  //   player p14 = {1000, "Jair \0"};
  //   save_player(14, p14);
  //   player p15 = {1000, "Laz  \0"};
  //   save_player(15, p15);
  //   player p16 = {1000, "Lax  \0"};
  //   save_player(16, p16);
   
}


void test()
{
  dampfwalze();
  player freddy = {1000, "FREDY\0"};
  player anni = {1001, "ANIKA\0"};
  player jak = {999, "JAKOB\0"};
  save_player(0, freddy);
  save_player(1, anni);
  save_player(2, jak);
  Serial.println(freddy.name);
}


//greift auf die sortierte players struktur zu und gibt den player auf position ranking aus dem eeprom zurück
player get_player_at(int ranking)
{
  //Serial.print("Get Player at ");
  //Serial.print(ranking);
  //Serial.print(" gibt ");
  //Serial.println(players[ranking]);
  return load_player(players[ranking]);
}



// setzt playercount und initialisiert die players bis dahin
void init_players()
{

  Serial.println("start init");
  
  int temp = 0;

  int test;

  do{
    EEPROM.get(temp++*PLAYERSIZE, test);
  } while(test != 0);

  playercount = temp-1;

  Serial.print("found players:");
  Serial.println(playercount);
  for(int i = 0; i<playercount; i++)
  {
    Serial.println(load_player(i).name);
    Serial.println(load_player(i).elo);
    players[i] = i;
  }

  
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
    int nothing = 0x00;
    EEPROM.update(i, nothing);
  }
}

void save_player(int id, player player)
{

  EEPROM.put(id*PLAYERSIZE, player);
}

player load_player(int id)
{
  player temp;
  EEPROM.get(id*PLAYERSIZE, temp);
  //Serial.print("Und ist ");
  //Serial.println(temp.name);
  return temp;
}

int save_elo(int id, short elo)
{
  EEPROM.put(id*PLAYERSIZE, elo);
}

void sort_at_init()
{
  quickSort(0, playercount-1);

}

void sort_by_elo()
{
  bubbleSort();
}

void bubbleSort() {
      bool swapped = true;
      int j = 0;
      short tmp;
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
      short tmp;
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

void display_players(String title,  int dim, int pointer, bool highlight)
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
    Serial.print("...");
    //Serial.println((highlight&&i==pointer?">":" ") +(String)get_player_at(i).name + "-"+get_player_at(i).elo);
    //Serial.println((i==pointer?">":" ")+(String)get_player_at(i).name) + " - ";
  
    char ausgabe[21];
    ausgabe[0] = i==pointer?'>':' ';
    player pl = get_player_at(i);
    for(int j= 1; j<6;j++)
    {
      ausgabe[j]=pl.name[j-1];
    }
    ausgabe[6] = ' ';
    ausgabe[7] = ' ';
    ausgabe[8] = '-';
    ausgabe[9] = ' ';
    ausgabe[10] = ' ';
    int elo = pl.elo;
    ausgabe[14] = elo%10>0?'0'+elo%10:0x4f;
    elo/=10;
    ausgabe[13] = elo%10>0?'0'+elo%10:0x4f;
    elo/=10;
    ausgabe[12] = elo%10>0?'0'+elo%10:0x4f;
    elo/=10;
    ausgabe[11] = elo%10>0?'0'+elo%10:0x4f;
    ausgabe[15] = ' ';
    ausgabe[16] = '(';
    ausgabe[17] = (i+1)/10>0?0x30+(i+1)/10:0x4f;
    ausgabe[18] = 0x30+(i+1)%10;
    ausgabe[19] = ')';
    ausgabe[20] = '\0';
    

    display.println(ausgabe);
    
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

void display_result()
{
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.clearDisplay();
  display.println((String)get_player_at(winner).name +" +"+ w_diff);
  display.println("  winning vs\n");
  display.println((String)get_player_at(loser).name + " -"+l_diff);
  display.display();
  delay(5000);
}

void bestaetigungsscreen()
{
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.clearDisplay();
  display.println((String)get_player_at(winner).name+"\nwins vs\n"+(String)get_player_at(loser).name+"\n      ok?");
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
      add_game(players[winner], players[loser]);
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
      list_pointer = list_pointer >= playercount-1?playercount-1:list_pointer+1;
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


   //test();
   first_load_players();

  Serial.println("Start Debug");
  
  init_players();

  Serial.println("Players initialized");
  
  //save();
  sort_by_elo();

  Serial.println("sorted");
  
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Champion");
  display.println("ist");
  display.display();
  delay(1500);
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("   "+(String)get_player_at(0).name);
  display.display();
  delay(3000);

  state = 0;
}

void loop() {
  // put your main code here, to run repeatedly:

  handleInput();

  if(draw_new)
  {
    // Serial.println("fresh draw");
    switch(state)
    {
      case 0:
        // title: hall of fame
        display_players("hall of fame", playercount, list_pointer, true);
        // toDo scroallable list of players sorted by elo
        // on enter: go to state 1
      break;
      case 1:
        // title: select winner
        // toDo scrollable list of players by elo, one highlighted is the selected winner
        display_players("select winner", playercount, list_pointer, true);
        // on enter: save highlighted as winner, proceed to 2
      break;
      case 2:
        // title: select loser
        // toDo scrollable list of players by alphabet including winner (excluding him appears very complicated), one highlighted is the selected loser
        display_players("select loser", playercount, list_pointer, true);
        // on enter: save highlighted as loser. compute and enter new elos. go to state 0
      break; 
      default:
        break;
    }
    draw_new = false;
  }
}
