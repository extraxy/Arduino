//****** 29.09.2021***********
//****** Для канала "Человек с паяльником"
//****** Демонстрация работы матричной клавиатуры 4х4


#include <Keypad.h> // подключаем библиотеку Keypad

const byte ROWS = 4; //задаем 4 строки в моем варианте
const byte COLS = 4; //задаем 4 столбца в моем варианте  
                     //у Вас возможно другое количество строк и столбцов
                   
//Задаем символы (они отображены на матричной клавиатуре). У Вас может быть другой вариант
char MatrixKeyPad[ROWS][COLS] = {
      {'1','2','3','4'},
      {'5','6','7','8'},
      {'9','A','B','C'},
      {'D','E','F','G'}         };

byte rowPins[ROWS] = {6, 7, 8, 9}; // Задаем пины строк 
byte colPins[COLS] = {5, 4, 3, 2}; // Задаем пины столбцов 

// создаем переменную типа класс библиотеки Keypad
Keypad customKeypad = Keypad(makeKeymap(MatrixKeyPad), colPins, rowPins, COLS, ROWS);  

void setup(){
//Подключаем серийный монитор, если не нужен, то удалите эту строку
  Serial.begin(9600); 
}
  
void loop(){
  // печатаем на серийном мониторе значения нажатой кнопки
  char customKey = customKeypad.getKey(); 
  if (customKey>'0'&customKey<'A'){Serial.print('S');Serial.println(customKey);}
  if (customKey=='A'){Serial.println("S10");}
  if (customKey=='B'){Serial.println("S11");}
  if (customKey=='C'){Serial.println("S12");}
  if (customKey=='D'){Serial.println("S13");}
  if (customKey=='E'){Serial.println("S14");}
  if (customKey=='F'){Serial.println("S15");}
  if (customKey=='G'){Serial.println("S16");}
}
