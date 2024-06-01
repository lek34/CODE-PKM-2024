const int ph_Pin = 35;
float Po=0;
float PH_step;
int nilai_analog_ph;
double TeganganPh;

float PH4 = 3.1;
float PH7 = 2.485;

void setup(){
  pinMode (ph_Pin,INPUT);
  Serial.begin(9600);
}

void loop(){
  int nilai_analog_ph = analogRead(ph_Pin);
  Serial.print("Nilai ADC Ph : ");
  Serial.println(nilai_analog_ph);
  TeganganPh = 3.3/4096 * nilai_analog_ph;
  Serial.print("Tegangan ph : ");
  Serial.print(TeganganPh,3);
  PH_step = (PH4 - PH7) / 3;
  Po = 7.00 + ((PH7 - TeganganPh)/PH_step);
  Serial.print("Nilai PH : ");
  Serial.println(Po,2);
  delay(3000);

}