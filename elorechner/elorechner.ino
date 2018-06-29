#include "math.h"
#define PLAYERNUMBER 3

struct player { int elo; String name; };

player players[PLAYERNUMBER] = { { 1000, "THO"}, {1000, "MAR"}, {1000, "JAK"} };

const int K = 15;

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

}

void loop() {
  // put your main code here, to run repeatedly:

}
