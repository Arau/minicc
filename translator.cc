#include <cassert>
#include <iostream>
#include <map>
using namespace std;

#include "translator.hh"

Translator Translator::translator;

const char* Translator::_translations[100][Translator::NUM_LANGS] = { /* 
   { 
      English, 
      Spanish, 
      Catalan 
   } */
   {
      "UNIMPLEMENTED",
      "NO IMPLEMENTADO",
      "NO IMPLEMENTAT"
   }, {
      "Compilation Error", 
      "Error de compilación", 
      "Error de compilació" 
   }, {
      "Execution Error",
      "Error de ejecución",
      "Error d'execució"
   }, {
      "UNIMPLEMENTED",
      "NO IMPLEMENTADO",
      "NO IMPLEMENTAT"
   }, {
      "Unexpected character '%c'",
      "Caracter '%c' inesperado",
      "Caracter '%c' inesperat"
   }, {
      "The '%s' function does not exist.",
      "La función '%s' no existe.",
      "La funció '%s' no existeix."
   }, {
      "Indirect call to functions is not implemented", 
      "La llamada no-directa a funciones no se ha implementado",
      "La crida indirecta a funcions no s'ha implementat"
   }, {
      "An if's condition needs to be a bool value",
      "La condición de un 'if' debe ser un valor de tipo 'bool'",
      "La condició d'un 'if' ha de ser un valor Booleà"
   }, {
      "Expected '}' but end of text found",
      "Esperaba '}' pero he llegado al final del texto",
      "Esperava '}' però he arribat al final del text"
   }, {
      "Expected ';' after expression",
      "Esperaba un ';' después de la expresión",
      "Esperava un ';' després de l'expressió"
   }, {
      "You should specify a filename",
      "No has indicado el fichero de código",
      "No has indicat el fitxer de codi"
   }, {
      "Error when reading input",
      "Error al leer de la entrada",
      "Error al llegir de l'entrada"
   }, {
      "Unexpected '%s' here.",
      "No esperaba '%s' aquí.",
      "No esperava '%s' aquí."
   }, {
      "ignoring macro '%s'",
      "ignorando la macro '%s'",
      "ignorant la macro '%s'"
   }, {
      "'#include' missing closing '%c'",
      "Al '#include' le falta el '%c' de cerrar",
      "A l'#include li falta el '%c' de tancar"
   }, {
      "Expected '%s' here.",
      "Esperaba un '%s' aquí.",
      "Esperava un '%s' aquí."
   }, {
      "Expected an identifier here.",
      "Esperaba un identificador aquí.",
      "Esparaba un identificador aquí."
   }, {
      "Basic types are not templates",
      "Los tipos básicos no son plantillas",
      "Els tipus bàsics no són plantilles"
   }, {
      "Unexpected character '%c' in parameter list",
      "Caracter '%c' inesperado en la lista de parámetros",
      "Caràcter '%c' inesperat a la llista de paràmetres"
   }, {
      "Expected '\"' or '<' here.",
      "Esperaba '\"' o '<' aquí.",
      "Esperava '\"' o '<' aquí."
   }, {
      "Expected an integer here.",
      "Esperaba un entero aquí.",
      "Esperaba un enter aquí."     
   }, {
      "The 'main' function does not exist.",
      "La función 'main' no existe.",
      "La funció 'main no existeix."
   }, {
      "The program begins.",
      "Empieza el programa.",
      "Comença el programa."
   }, {
      "The program ends.",
      "Termina el programa.",
      "Acaba el programa."
   }, {
      "The 'if's condition must be a value of type 'bool'.",
      "La condición de un 'if' debe ser un valor de tipo 'bool'.",
      "La condició d'un 'if' ha de ser un valor de tipus 'bool'."
   }, {
      "The condition is 'true', we take the first branch.",
      "La condición vale 'true', seguimos por la primera rama.",
      "La condició val 'true', seguim per la primera branca."
   }, {
      "The condition is 'false', we take the second branch.",
      "La condición vale 'false', seguimos por la segunda rama.",
      "La condició val 'false', seguim per la segona branca."
   }, {
      "The condition is 'false', we continue.",
      "La condición vale 'false', continuamos.",
      "La condició val 'false', continuem."
   }, {
      "The condition in a '%s' must be a value of type 'bool'.",
      "La condición de un '%s' debe ser un valor de tipo 'bool'.",
      "La condició a un '%s' ha de ser un valor de tipus 'bool'."
   }, {
      "The condition is 'false', we exit the %s.",
      "La condición vale 'false', salimos del %s.",
      "La condició és 'false', sortim del %s."
   }, {
      "The condition is 'true', we enter the %s.",
      "La condición vale 'true', entramos en el %s.",
      "La condició val 'true', entrem al %s."
   }, {
      "%s is returned.",
      "Se retorna %s.",
      "Es retorna %s."
   }, {
      "Some output is written.",
      "Se escribe a la salida.",
      "S'escriu a la sortida."
   }, {
      "Some input is read.",
      "Se lee de la entrada.",
      "Es llegeix de l'entrada."
   }, {
      "The expression evaluated to %s.",
      "La expresión ha dado %s.",
      "L'expressió ha donat %s."
   }, {
      "We assign the value.",
      "Asignamos el valor.",
      "Assignem el valor."
   }, {
      "We evaluate the %s parameter.",
      "Se evalúa el %s parámetro.",
      "S'avalua el %s paràmetre."
   }, {
      "first", "primer", "primer"
   }, {
      "second", "segundo", "segon"
   }, {
      "third", "tercer", "tercer"
   }, {
      "fourth", "cuarto", "quart"
   }, {
      "fifth", "quinto", "cinquè"
   }, {
      "sixth", "sexto", "sisè"
   }, {
      "seventh", "séptimo", "setè"
   }, {
      "We evaluate parameter number %d.",
      "Se evalúa el parámetro número %d.",
      "S'avalua el paràmetre número %d."
   }, {
      "We jump to function '%s'.",
      "Saltamos a la función '%s'.",
      "Saltem a la funció '%s'."
   }, {
      "Variable '%s' is declared.",
      "Se declara la variable '%s'.",
      "Es declara la variable '%s'."
   }, {
      "Variables %s are declared.",
      "Se declaran las variables %s.",
      "Es declaren les variables %s."
   }, {
      " and ", " y ", " i "
   },
   { "END" }
};
