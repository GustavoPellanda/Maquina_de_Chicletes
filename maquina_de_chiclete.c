#include <Servo.h> 

const int sensor_batidas = 0; //Sensor Piezo que detecta as batidas
const int potenciometro = 1; //Potenciômetro que regula a sensibilidade da detecção
const int botao_programador = 2; //Botão que será usado para começar a programação de um código novo
const int LED_branco1 = 3; //LED que pisca conforme as batidas são escutadas
const int LED_verde = 4; //LED que indica que o código escutado foi aprovado
const int LED_vermelho = 5; //LED que indica que o código escutado foi reprovado
const int LED_branco2 = 6; //LED que acende quando uma bola de chclete está sendo liberada
const int servo_entrada = 7; //Controle do Servo 

const int sensibilidade_maxima = 40; //Define a sensbilidade máxima do potenciômetro
const int tolerancia_individual = 30; //Define a margem de erro máxima que o tempo de uma batida pode ter
const int tolerancia_conjunto = 20; //Define a margem de erro máxima que a sequência como um todo pode ter
const int tempo_batida = 80; //Será usado como padrão de tempo entre uma batida e outra
const int batidas_limite = 20;  //Número máximo de batidas dentro do código
const int pausa_tempo = 1500; //Tempo máximo que o programa esperará para ouvir a próxima batida

int codigo_secreto[batidas_limite] = {50, 25, 25, 50, 100, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //Esse é o código. Cada número define uma porcentgem de duração de uma batida. Recomentda-se que use então 100, 50 ou 25 para gerar padrões
int leitor_batidas[batidas_limite]; //Matriz que será usada para armazenar as pausas entre as batidas
int batida_recente = 0; //Letura mais recente a ser captada pelo leitror
int batida_minima =1023; //Sinal mínimo a que o Piezo deve captar (controlada pelo pontenciômetro)
int botao_programador_apertado = false; //Define o estado do botão programador
long contador = 0; //Será usado paradefinir alguns tempos de duração
Servo servo; //O Serco em si

void setup() {

  //Define as portas digitais dos LEDs como saídas
  pinMode(LED_vermelho, OUTPUT);
  pinMode(LED_verde, OUTPUT);
  pinMode(LED_branco1, OUTPUT);
  pinMode(LED_branco2, OUTPUT);
  pinMode(botao_programador, INPUT);

  //Inicializa o potenciometro
  batida_minima = map(analogRead(potenciometro), 0, 1023, 0, sensibilidade_maxima); 

  //Liga todas as luzes, apenas para conferir que tudo funciona.
  digitalWrite(LED_vermelho, HIGH);
  digitalWrite(LED_verde, HIGH);
  digitalWrite(LED_branco1, HIGH);
  digitalWrite(LED_branco2, HIGH);

  //Inicializa o Servo:
  servo.attach(servo_entrada);
  delay(50);
  for(int i=0; i<150; i++){        
    servo.write(0);
    delay(20);
  }  
  delay(50);
  servo.detach();

  //Desliga as luzes novamente
  digitalWrite(LED_vermelho, LOW);
  digitalWrite(LED_verde, LOW);
  digitalWrite(LED_branco1, LOW);
  digitalWrite(LED_branco2, LOW);
}

void loop() {
  
  //Escuta as batidas:
  batida_recente = analogRead(sensor_batidas);
  contador++;
  if(contador >= 1500){ //Um loop dura enquanto o Piezo busca por batidas, dentro de seu tempo limite
    contador=0; //Se o tempo limite for atingido e nada for detectado, o contador é reiniciado
    digitalWrite(LED_branco2, LOW); //Se o tempo limite for atingido e algo tiver sido detectado, esse LED foi aceso e precisamos desligá-lo antes de começar mais um loop 
  }

  //Lê qual sensibilidade o potenciômetro tem definida como mínima
  batida_minima = map(analogRead(potenciometro), 0, 1023, 0, sensibilidade_maxima);

  //Busca por um sial do botão programador: 
  if(digitalRead(botao_programador)==HIGH){ 
    botao_programador_apertado = true;
    digitalWrite(LED_vermelho, HIGH);
    digitalWrite(LED_verde, HIGH);
  }

  else{
    botao_programador_apertado = false;
    digitalWrite(LED_vermelho, LOW);
    digitalWrite(LED_verde, LOW);
  }

  //Chama pela função que escuta as batidas quando o Piezo detecta uma que está acima de seu valor mínimo:
  if(batida_recente > batida_minima){
    escuta_batida();
  }
} 

//Função que escuta as batidas:
void escuta_batida(){   

  digitalWrite(LED_branco2, LOW); //Desliga essa luz caso ela tenha sido deixado acesa

  int i = 0;
  int contador = 0;

  //Reseta a matriz que guarda batidas:
  for(i=0; i<batidas_limite; i++){
    leitor_batidas[i] = 0;
  }

  //Refeências de tempo e de batidas que a matriz usará para se orientar
  int batida_atual = 0;
  int tempo_inicio = millis();
  int agora = millis();

  digitalWrite(LED_verde, HIGH);

  delay(tempo_batida);
  digitalWrite(LED_verde, LOW); 

  do{
    
    //Espera e escuta pela próxima batida:
    batida_recente = analogRead(sensor_batidas);
    batida_minima = map(analogRead(potenciometro), 0, 1023, 0, sensibilidade_maxima);  

    //Quando uma nova batida é detectada:
    if(batida_recente > batida_minima){
      agora=millis(); //Salva o tempo entre essa e a última
      leitor_batidas[batida_atual] = agora - tempo_inicio;
      batida_atual++;
      tempo_inicio = agora;          
      digitalWrite(LED_verde, HIGH);
      delay(tempo_batida/2);
      digitalWrite(LED_verde, LOW);  
      delay(tempo_batida/2);   
    }

    agora = millis();

  //Até que o tempo acabe pu aconteçam batidas demais:
  } while((agora-tempo_inicio < pausa_tempo) && (batida_atual < batidas_limite));

  //Com uma sequência completa, verifica se ela corresponde ao código e se o botão de programação está ativo:
  if(botao_programador_apertado==false){  
    if(valida_batida() == true){      
      codigo_aprovado(); 
    } 
    else{
      codigo_rejeitado(); 
    }
  } 
  
  else{
    valida_batida();
    digitalWrite(LED_vermelho, LOW);
    digitalWrite(LED_verde, LOW);
    for(i=0; i<3; i++){
      delay(100);
      digitalWrite(LED_vermelho, HIGH);
      digitalWrite(LED_verde, HIGH);
      delay(100);
      digitalWrite(LED_vermelho, LOW);
      digitalWrite(LED_verde, LOW);      
    }
  }
}

void codigo_aprovado(){
  int i=0;

  digitalWrite(LED_verde, HIGH);
  digitalWrite(LED_branco1, HIGH);

  //Opera o Servo-motor
  servo.attach(servo_entrada);
  delay(50);
  int define_luzes=HIGH;
  int d=10;
  for (int i=0;i<=180;i++){        
    if (i>140){
      servo.write(140);
    } 
    else {
      servo.write(i);
    }
    if (i % 18 == 0){
      define_luzes =! define_luzes;
    }
    digitalWrite(LED_branco1, define_luzes); 
    delay(d);
  }
  delay(100);   
  digitalWrite(LED_branco1, LOW);   

  digitalWrite(LED_branco2, HIGH);
  for(int i = 140;i >= 0; i--){
    servo.write(i);   
    delay(d);
  }   
  delay(50);
  servo.detach();

  delay(100);
}

void codigo_rejeitado(){
  digitalWrite(LED_verde, LOW);  
  digitalWrite(LED_vermelho, LOW);		
  
  for (int i=0; i<4; i++){					
    digitalWrite(LED_vermelho, HIGH);
    delay(100);
    digitalWrite(LED_vermelho, LOW);
    delay(100);
  }
}

//Função que verifica se a sequência está de acordo com o código:
boolean valida_batida(){
  int i=0;

  //Referências que serão usadas para fazer as comparações:
  int Nbatidas_atual = 0;
  int Nbatidas_secretas = 0;
  int intervalo_limite = 0;

  //Primeiramente verifica se o número de batidas coincide:
  for(i=0; i < batidas_limite; i++){
    if(leitor_batidas[i] > 0){
      Nbatidas_atual++;
    }
    if(codigo_secreto[i] > 0){  					
      Nbatidas_secretas++;
    }
    if(leitor_batidas[i] > intervalo_limite){
      intervalo_limite = leitor_batidas[i];
    }
  }

  //Caso o botão de programação esteja ativo:
  if(botao_programador_apertado == true){
    for(i = 0; i < batidas_limite; i++){
      codigo_secreto[i]= map(leitor_batidas[i],0, intervalo_limite, 0, 100);
    }
    
    //Pisca as luzes para indicar que a programação acabou
    digitalWrite(LED_verde, LOW);
    digitalWrite(LED_vermelho, LOW);
    delay(750);

    //Pisca as luzes de acordo com o padrão criado:
    digitalWrite(LED_verde, HIGH);
    digitalWrite(LED_vermelho, HIGH);
    delay(40);
    for(i = 0; i < batidas_limite ; i++){
      digitalWrite(LED_verde, LOW);
      digitalWrite(LED_vermelho, LOW);  

      if(botao_programador_apertado== true){
        if (codigo_secreto[i] > 0){                                   
          delay(map(codigo_secreto[i],0, 100, 0, intervalo_limite));
          digitalWrite(LED_verde, HIGH);
          digitalWrite(LED_vermelho, HIGH);  
        }
      }
      delay(40);
      digitalWrite(LED_verde, LOW);
      digitalWrite(LED_vermelho, LOW);  
    }
    return false;
  }

  //Quando o número de batidas não é o mesmo:
  if (Nbatidas_atual != Nbatidas_secretas){
    return false;
  }

  //Compara os intervlos das batidas:
  bool codigo_encontrado = true;
  int diferenca_total = 0;
  int diferenca_tempo = 0;

  for(i=0; i < batidas_limite; i++){
    leitor_batidas[i] = map(leitor_batidas[i], 0, intervalo_limite, 0, 100);      
    diferenca_tempo = abs(leitor_batidas[i] - codigo_secreto[i]);
    if(diferenca_tempo > tolerancia_individual){
      codigo_encontrado=false;
    }
    diferenca_total += diferenca_tempo; //Quando a tolerâcia individual encontrada é inaceitável
  }
  
  if(diferenca_total / Nbatidas_secretas > tolerancia_conjunto){  //Quando a tolerâcia total encontrada é inaceitável
    codigo_encontrado = false;
  }

  if(codigo_encontrado == false){
    return false;
  } 
  else {
    return true;
  }
}