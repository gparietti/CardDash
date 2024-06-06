/*
 * Progetto: CarDash
 * Autore: Gabriele Parietti
 * Versione: V4.0
 * Data: 06/06/2024
 
 * Descrizione:
 * CarDash è un dispositivo innovativo basato sul microcontrollore ESP32, progettato per offrire informazioni e intrattenimento mentre sei in viaggio. 
 * Montato sul cruscotto dell'auto e proiettando i dati direttamente sul parabrezza, CarDash offre un accesso rapido e sicuro a una serie di utili funzionalità. 
 * Con un'interfaccia intuitiva e personalizzabile, CarDash consente agli utenti di selezionare e visualizzare le informazioni desiderate, come l'oroscopo del giorno, le previsioni meteo locali, la data e l'ora, barzellette per allietare il viaggio, avvisi importanti e le ultime notizie. 
 * Funzionante tramite connessione WiFi, supporta anche gli hotspot e può essere utilizzato in modalità non proiettore su vetro per adattarsi alle tue esigenze di viaggio. 
 * Sicuro, conveniente e altamente personalizzabile, CarDash è il compagno ideale per ogni viaggio in auto.
 *
 * Licenza:
 * Questo progetto è rilasciato sotto la licenza MIT. Puoi utilizzarlo, modificarlo e distribuirlo
 * liberamente, a condizione che venga mantenuto il riconoscimento originale all'autore.
 *
 * MIT License
 * 
 * Copyright © [2024] [Gabriele Parietti]
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

//Librerie necessarie per compilare il progetto.
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

//Inizializzazione Display Dot Matrix
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW    //Definizione del tipo di display (Qualora uscissero scritte errate, provare a cambiare il tipo). Solitamente si usa FC16_HW. 
#define MAX_DEVICES 4  //Numero di moduli 8x8 che compongono il display.
#define CS_PIN 15  //Pin CS collegato al GPIO 15 dell'ESP32.
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

//Inizializzazione WebServer in ascolto sulla porta 80 (Standard traffico HTTP). 
AsyncWebServer server(80); 

//Inizializzazione array contenente i vari segni zodiacali (per ogni mese).
const char* segni[13] = {
  "",
    "capricorn",    // Mese 1 (Gennaio)
    "aquarius",     // Mese 2 (Febbraio)
    "pisces",       // Mese 3 (Marzo)
    "aries",        // Mese 4 (Aprile)
    "taurus",       // Mese 5 (Maggio)
    "gemini",       // Mese 6 (Giugno)
    "cancer",       // Mese 7 (Luglio)
    "leo",          // Mese 8 (Agosto)
    "virgo",        // Mese 9 (Settembre)
    "libra",        // Mese 10 (Ottobre)
    "scorpio",      // Mese 11 (Novembre)
    "sagittarius"  // Mese 12 (Dicembre)
};
int MeseOroscopo; //Variabile per associare i vari segni al mese corrente.
bool SetOroscopo, SetWeather, SetDate, SetJoke, SetNews, SetAlert = HIGH; //Varibili che 


//DEFINIZIONE VARIABILI GLOBALI --> INSERIRE I PROPRI DATI QUI! 
#define SPECCHIO //Commentare questa parte se non si vuole ribaltare la scritta (Utilizzare il display senza specchio di fronte).
#define IntDisplay 2 //Intensità display (0-15; attenzione alla corrente!)
#define SpeedText 30 //Velocità scorrimento testo (Più grande è il valore inserito più lo scorrimento sarà lento).
#define InfoDelay 5000 //Pausa tra ogni elemento mostrato (in millisecondi). 
const char* ssid     = "Iphone di Gabriele";  //Inserire il Nome (SSID) della rete WiFi a cui si vuole collegare il dispositivo.
const char* password = "deliddo27";  //Inserire la Password della rete WiFi a cui si vuole collegare il dispositivo.
//FUNZIONA ANCHE CON HOTSPOT!!


// --INSERIMENTO URL A CUI LE API FARANNO RICHIESTA-- 
//   INSERIRE IL PROPRIO API TOKEN DOVE RICHIESTO!
//        NB: TUTTI I SERVIZI SONO GRATUITI!
//         NON CONDIVIDERE LA TUA API KEY!


//PER LA GESTIONE DELLE NOTIZIE IL DISPOSITIVO UTILIZZA LA API DI NEWSAPI: Creare un account su https://newsapi.org/ e nella pagina del proprio account copiare la propria API KEY per poi inserirla nel link sottostante.
//PER CAMBIARE LA FONTE DELLE NOTIZIE BASTA SOSTITUIRE LA PARTE TRA "v2/" E "&pageSize".
/*
//NB: MOLTE FONTI NON SONO SUPPORTATE. ECCO ALCUNI ESEMPI:

https://newsapi.org/v2/top-headlines?sources=bbc-news&pageSiz... --> https://newsapi.org/v2/everything?domains=politico.eu&pageSiz...

 * 1)everything?domains=politico.eu
 * 2)top-headlines?sources=cnn
 * 3)everything?domains=washingtonpost.com
 */
 
String serverNameNews ="https://newsapi.org/v2/top-headlines?sources=bbc-news&pageSize=1&page=1&apiKey=INSERT-HERE-API-KEY";
//                                             ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


//PER LA GESTIONE DELLE BATTUTE IL DISPOSITIVO UTILIZZA LA API DI JOKEAPI: NON SERVE UNA API KEY. 
//PER FILTRARE LE BATTUTE VEDERE LA DOCUMENTAZIONE DEL SITO (https://v2.jokeapi.dev/) E MODIFICARE IL LINK SOTTOSTANTE. 

const char* serverNameJoke = "https://v2.jokeapi.dev/joke/Any?type=single";

//PER LA GESTIONE DELLA DATA E DELL'ORA ATTUALE IL DISPOSITIVO UTILIZZA LA API DI TIMEAPI: NON SERVE UNA API KEY. 
//PER CAMBIARE LA POSIZIONE VEDERE LA DOCUMENTAZIONE DEL SITO (https://timeapi.io/swagger/index.html) E MODIFICARE IL LINK SOTTOSTANTE.
//NB: IN ALTERNATIVA, CAMBIARE IL NOME DELLA ZONA DIRETTAMENTE NELL'URL SOTTOSTANTE --> Europe/Rome

const char* serverNameDate = "https://timeapi.io/api/Time/current/zone?timeZone=Europe/Rome";

//PER LA GESTIONE DEL METEO IL DISPOSITIVO UTILIZZA LA API DI OPENWEATHER: Creare un account su https://openweathermap.org/ e nella pagina del proprio account copiare la propria API KEY per poi inserirla nel link sottostante.
//NB: PER CAMBIARE LE UNITA' E LA LINGUA DEL METEO MODIFICARE IL LINK SOTTOSTANTE --> lang=en&units=metric

const char* serverNameWeather = "https://api.openweathermap.org/data/2.5/weather?id=3182164&appid=INSERT-HERE-API-KEY&lang=en&units=metric";
//                                                                                  ^^^^^^^
//IL METEO FA RIFERIMENTO ALLA POSIZIONE DEFINITA DAL CODICE ID (ES: 2182164 --> BERGAMO).
//PER CAMBIARE POSIZIONE, BASTA RECARSI SU https://openweathermap.org/ E CERCARE NELLA BARRA DI RICERCA (IN ALTO A SINISTRA) LA PROPRIA POSIZIONE.
//UNA VOLTA FATTO, IN ALTO, NELLA BARRA DI RICERCA DEL BROWSER COMPARE UN URL DI QUESTO TIPO: https://openweathermap.org/city/2643743 --> (2643743 = London)
//IL CODICE DOPO "city/" CORRISPONDE ALL'ID DA SOSTITUIRE NELL'URL "serverNameWeather" (riga 122-123) PER CAMBIARE POSIZIONE.

//LA GESTIONE DELL'OROSCOPO VIENE DEFINITA NELLA RELATIVA FUNZIONE ALLA RIGA 700.
//NON RICHIEDE NESSUNA API KEY; NON MODIFICARE IL LINK.

//PER LA GESTIONE DELL'ALERT (MESSAGGIO PERSONALIZZATO / DRIVE ADVICE) SCRIVERE QUELLO CHE SI VUOLE NELLA STRINGA SOTTOSTANTE (TRA "").

String Infomessage = "Pay attention when driving. It's not just you on the road.";


void setup() {
    Serial.begin(115200);

    //Tentativo di connessione alla WiFi con le credenziali definite precedentemente (SSID e Password).
    WiFi.begin(ssid, password);
    WiFi.setAutoReconnect(true); //Se disponibile, tenta riconnessione.

    //Accensione Display
    P.begin();

    //INVERSIONE DELLA SCRITTA SULLO SPECCHIO. PER RIMUOVELA COMMENTARE LA DEFINE ALL'INIZIO DEL CODICE (RIGA 81) O LA RIGA 153.
    #ifdef SPECCHIO
    P.setZoneEffect(0, true, PA_FLIP_LR); 
    #endif
    
    P.setIntensity(IntDisplay);  //Intensità display.

    //Mostra avvio su display.
    P.displayClear();
    P.displayText("Welcome on CarDash V4.0 - by Gabriele Parietti", PA_CENTER, 30, P.getPause(), PA_SCROLL_RIGHT, PA_SCROLL_RIGHT);
    while (!P.displayAnimate()) {
        //Attendi la fine dell'animazione.
    }

    //Mostra tentativo connessione su display.
    P.displayClear();
    P.displayText("Connecting, please wait...", PA_CENTER, P.getSpeed(), P.getPause(), PA_SCROLL_RIGHT, PA_SCROLL_RIGHT);
    while (!P.displayAnimate()) {
        //Attendi la fine dell'animazione.
    }
    
    //LOGICA RICONNESSIONE WiFi --> Tratta dall'esempio WiFiClientConnect della libreria WiFi.
    //PER LA DESCRIZIONE COMPLETA VEDERE L'ESEMPIO.
    
    int tryDelay = 500;       // Pausa di 500ms tra ogni tentativo.
    int numberOfTries = 20;   // Numero di tentativi. 

    while (numberOfTries > 0) {
        wl_status_t wifiStatus = WiFi.status(); 
        switch (wifiStatus) {
            case WL_NO_SSID_AVAIL:
                Serial.println("[WiFi] SSID not found");
                break;
            case WL_CONNECT_FAILED:
                Serial.println("[WiFi] Failed - WiFi not connected!");
                P.displayClear();
                P.displayText("Failed to Connect! Try double-checking the credentials you entered or unplug and re-plug the power!", PA_CENTER, P.getSpeed(), P.getPause(), PA_SCROLL_RIGHT, PA_SCROLL_RIGHT);
                while (!P.displayAnimate()) {
                    //Attendi la fine dell'animazione.
                }
                return;
            case WL_CONNECTION_LOST:
                Serial.println("[WiFi] Connection was lost");
                break;
            case WL_SCAN_COMPLETED:
                Serial.println("[WiFi] Scan is completed");
                break;
            case WL_DISCONNECTED:
                Serial.println("[WiFi] WiFi is disconnected");
                break;
            case WL_CONNECTED:
                {
                    Serial.println("[WiFi] WiFi is connected!");
                    Serial.print("[WiFi] IP address: ");
                    Serial.println(WiFi.localIP());
                    String InfoWifi = "Successfully connected to:" + String(ssid) +" ! Please remember the IP address: " + WiFi.localIP().toString() + " repeat: " + WiFi.localIP().toString() + " to manage the display!";
                    P.displayClear();
                    P.displayText(InfoWifi.c_str(), PA_CENTER, P.getSpeed(), P.getPause(), PA_SCROLL_RIGHT, PA_SCROLL_RIGHT);
                    while (!P.displayAnimate()) {
                        //Attendi la fine dell'animazione.
                    }

                    
//CONFIGURAZIONE PAGINA HTML (WebServer) presente all'indirizzo mostrato sul display una volta connesso.
//La pagina permette la gestione degli elementi mostrati sul display.

server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){  //Utilizza la libreria AsyncWebServerRequest per accendere il server.

//CODICE HTML --> MODIFICARE QUESTA PARTE PER PERSONALIZZARE L'ASPETTO DELLA PAGINA.
String content = "<form id='form' action='/update' method='post'>";
content += "<body style='font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #d28e8e;'>";
content += "<div class='container' style='text-align: center; margin-top: 100px;'>";
content += "<div class='title' style='font-size: 36px; font-weight: bold; color: #000000; text-transform: uppercase; letter-spacing: 2px;'>CarDash</div>";
content += "<div class='subtitle' style='font-size: 17px; color: #000000; margin-top: 10px;'>V4.0</div>";
content += "<div class='subtitle' style='font-size: 24px; color: #000000; margin-top: 10px;'>by Gabriele Parietti</div>";
content += "<div class='subtitle' style='font-size: 27px; color: #000000; margin-top: 10px;'>Here you can choose what you want to be shown on the display!</div>";
content += "<form>";
content += "<table>";
content += "<tr>";
content += "<td>1</td>";
content += "<td>";
content += "<div class='switch-container'>";
content += "<label class='switch'>";
content += "<input type='checkbox' id='switch1' name='Oroscope' onchange='submitForm()' ";
content += SetOroscopo ? "checked" : "";
content += "><span class='slider'></span>";
content += "</label>";
content += "</div>";
content += "</td>";
content += "<td>Oroscope</td>";
content += "</tr>";
content += "<tr>";
content += "<td>2</td>";
content += "<td>";
content += "<div class='switch-container'>";
content += "<label class='switch'>";
content += "<input type='checkbox' id='switch2' name='News' onchange='submitForm()' ";
content += SetNews ? "checked" : "";
content += "><span class='slider'></span>";
content += "</label>";
content += "</div>";
content += "</td>";
content += "<td>News</td>";
content += "</tr>";
content += "<tr>";
content += "<td>3</td>";
content += "<td>";
content += "<div class='switch-container'>";
content += "<label class='switch'>";
content += "<input type='checkbox' id='switch3' name='Weather' onchange='submitForm()' ";
content += SetWeather ? "checked" : "";
content += "><span class='slider'></span>";
content += "</label>";
content += "</div>";
content += "</td>";
content += "<td>Weather</td>";
content += "</tr>";
content += "<tr>";
content += "<td>4</td>";
content += "<td>";
content += "<div class='switch-container'>";
content += "<label class='switch'>";
content += "<input type='checkbox' id='switch4' name='Joke' onchange='submitForm()' ";
content += SetJoke ? "checked" : "";
content += "><span class='slider'></span>";
content += "</label>";
content += "</div>";
content += "</td>";
content += "<td>Joke</td>";
content += "</tr>";
content += "<tr>";
content += "<td>5</td>";
content += "<td>";
content += "<div class='switch-container'>";
content += "<label class='switch'>";
content += "<input type='checkbox' id='switch5' name='Date and Hour' onchange='submitForm()' ";
content += SetDate ? "checked" : "";
content += "><span class='slider'></span>";
content += "</label>";
content += "</div>";
content += "</td>";
content += "<td>Date and Hour</td>";
content += "</tr>";
content += "<tr>";
content += "<td>6</td>";
content += "<td>";
content += "<div class='switch-container'>";
content += "<label class='switch'>";
content += "<input type='checkbox' id='switch6' name='Alert' onchange='submitForm()' ";
content += SetAlert ? "checked" : "";
content += "><span class='slider'></span>";
content += "</label>";
content += "</div>";
content += "</td>";
content += "<td>Alert</td>";
content += "</tr>";
content += "</table>";
content += "</form>";
//Parti di CSS per gli switch e parti di Javascript per l'invio dei dati.
content += "<style>.switch-container { margin-bottom: 10px; } .switch { position: relative; display: inline-block; width: 60px; height: 34px; } .switch input { opacity: 0; width: 0; height: 0; } .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; -webkit-transition: .4s; transition: .4s; border-radius: 34px; } .slider:before { position: absolute; content: ''; height: 26px; width: 26px; left: 4px; bottom: 4px; background-color: white; -webkit-transition: .4s; transition: .4s; border-radius: 50%; } input:checked + .slider { background-color: #2196F3; } input:focus + .slider { box-shadow: 0 0 1px #2196F3; } input:checked + .slider:before { -webkit-transform: translateX(26px); -ms-transform: translateX(26px); transform: translateX(26px); } .slider.round { border-radius: 34px; } .slider.round:before { border-radius: 50%; } </style>";
content += "<script>function submitForm() { document.getElementById('form').submit(); }</script>";

request->send(200, "text/html", content); //Manda risposta al dispositivo che effettua richieste al server (client): l'ESP32.
});

//Gestione dell'aggiornamento degli switch.
//Imposta le variabili booleane dichiarate all'inizio in base ai valori degli switch inviati dalla richiesta POST (richiesta che invia valori come parametri).
server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
  //Controlla se lo switch per l'oroscopo è stato attivato.
  SetOroscopo = request->hasArg("Oroscope");
  //Controlla se lo switch per le notizie è stato attivato.
  SetNews = request->hasArg("News");
  //Controlla se lo switch per il meteo è stato attivato.
  SetWeather = request->hasArg("Weather");
  //Controlla se lo switch per la battuta è stato attivato.
  SetJoke = request->hasArg("Joke");
  //Controlla se lo switch per la data e l'ora è stato attivato.
  SetDate = request->hasArg("Date and Hour");
  //Controlla se lo switch per l'allarme è stato attivato.
  SetAlert = request->hasArg("Alert");
  request->redirect("/");  // Una volta aggiornato, ritorna alla pagina principale
});


//Avvio del server
server.begin();
return;
}
            
default:
Serial.print("[WiFi] WiFi Status: ");
Serial.println(wifiStatus);
break;
}
delay(tryDelay);
numberOfTries--;
}

    Serial.println("[WiFi] Unable to connect after multiple attempts.");
    P.displayClear();
    P.displayText("Can't connect after multiple attempts! Try double-checking the credentials you entered or unplug and re-plug the power!", PA_CENTER, P.getSpeed(), P.getPause(), PA_SCROLL_RIGHT, PA_SCROLL_RIGHT);
    while (!P.displayAnimate()) {
        //Attendi la fine dell'animazione.
    }
}


void loop() {
  
  if (WiFi.status() == WL_CONNECTED) {  //Verifica se WiFi è connesso.

    //Trasforma il valore ritornato da ciascuna funzione in una stringa (al fine di concatenare i vari testi). 
    String JokeText = Joke();      
    String dateTimeText = DataOra();
    String newsText = News();
    String WeatherText = Weather();
    String OroscopeText = Oroscope();

    //VERIFICA SE GLI SWITCH SONO ATTIVI (VARIABILI BOOL DICHIARATE ALL'INIZIO) E MOSTRA LE INFORMAZIONI RELATIVE A QUELLI ABILITATI.
    
    if (SetOroscopo == HIGH) {
    P.displayClear();
    P.displayText(OroscopeText.c_str(), PA_CENTER, SpeedText, P.getPause(), PA_SCROLL_RIGHT, PA_SCROLL_RIGHT);   //c.str() restituirà un puntatore al primo carattere della stringa, permettendo di utilizzarla in funzioni che richiedono una stringa di tipo const char* (Un puntatore a carattere costante, utile se si desidera garantire che il contenuto di una stringa non venga modificato accidentalmente all'interno di una funzione o di un contesto in cui non dovrebbe essere modificato).
    while (!P.displayAnimate()) {
      //Attendi la fine dell'animazione.
    }
    delay(InfoDelay); //Pausa tra un elemento e l'altro.
    }

    if (SetNews == HIGH) {
    P.displayClear();
    P.displayText(JokeText.c_str(), PA_CENTER, SpeedText, P.getPause(), PA_SCROLL_RIGHT, PA_SCROLL_RIGHT);
    while (!P.displayAnimate()) {
      //Attendi la fine dell'animazione.
    }
    delay(InfoDelay);
    }
    
    if (SetWeather == HIGH) {
    P.displayClear();
    P.displayText(WeatherText.c_str(), PA_CENTER, SpeedText, P.getPause(), PA_SCROLL_RIGHT, PA_SCROLL_RIGHT);
    while (!P.displayAnimate()) {
      //Attendi la fine dell'animazione.
    }
    delay(InfoDelay);
    }
    
 if (SetJoke == HIGH) {
    P.displayClear();
    P.displayText(JokeText.c_str(), PA_CENTER, SpeedText, P.getPause(), PA_SCROLL_RIGHT, PA_SCROLL_RIGHT);
    while (!P.displayAnimate()) {
      //Attendi la fine dell'animazione.
    }
    delay(InfoDelay);
    }
    
 if (SetDate == HIGH) {
    P.displayClear();
    P.displayText(dateTimeText.c_str(), PA_CENTER, SpeedText, P.getPause(), PA_SCROLL_RIGHT, PA_SCROLL_RIGHT);
    while (!P.displayAnimate()) {
      //Attendi la fine dell'animazione.
    }
    delay(InfoDelay); 
    }
    
 if (SetAlert == HIGH) {
    P.displayClear();
    P.displayText(Infomessage.c_str(), PA_CENTER, SpeedText, P.getPause(), PA_SCROLL_RIGHT, PA_SCROLL_RIGHT);
    while (!P.displayAnimate()) {
      //Attendi la fine dell'animazione.
    }
    delay(InfoDelay);
    }

//Avvisa se nulla abilitato; nulla da mostrare.

 if (SetAlert == HIGH && SetWeather == HIGH && SetDate == HIGH && SetOroscopo == HIGH && SetJoke == HIGH && SetNews == HIGH) {
    P.displayClear();
    String Advice = "Nothing to show... Enable from IP address/WebServer here:"  + WiFi.localIP().toString();
    P.displayText(Advice.c_str(), PA_CENTER, SpeedText, P.getPause(), PA_SCROLL_RIGHT, PA_SCROLL_RIGHT);
    while (!P.displayAnimate()) {
      //Attendi la fine dell'animazione.
    }
    delay(InfoDelay);
 }
 }
}
  
//DEFINIZIONE FUNZIONI CON LE VARIE RICHIESTE API.

//Funzione che ritorna una battuta.
String Joke() {
  String Battuta = "I found nothing to show!"; //Valore di default in caso di errore..

  //Chiamata all'API
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverNameJoke);   //Richiesta HTTP inviata dall'ESP32 a un server (mediante URL contenente tutti i parametri necessari per la richista) per richiedere una risorsa specifica e ottenere una risposta dal medesimo (Informazioni da mostrare). L'URL della risorsa è definito all'inizio del codice.

    int httpResponseCode = http.GET();  //Ottieni il codice di risposta HTTP: 200 = Successo, 404 = Non trovato, 401 = Non autorizzato, 403 = Accesso negato, 500 = Errore durante l'elaborazione della richiesta.

    if (httpResponseCode > 0) { //Verifica se richista completata con successo (Server ha risposto alla richiesta fatta dall'ESP32, il client).
    String payload = http.getString();
    
      //LA RISPOSTA OTTENUTA DAL SERVER DOPO LA RICHIESTA DEL CLIENT VIENE FORNITA IN FORMATO JSON (formato di dati leggero e flessibile; organizzati in un formato facilmente leggibile sia per gli esseri umani che per le macchine).
      //IL CODICE SEGUENTE DESERIALIZZA LA RISPOSTA JSON (converte i dati JSON che sono codificati in una stringa di testo in un formato manipolabile dal programma (Variabili di tipo String/Int/Float...)) E SALVA I VALORI RICHIESTI IN UNA VARIABILE.
      
      DynamicJsonDocument doc(1024);  //1024 indica la dimensione della risposta in byte.     
      DeserializationError error = deserializeJson(doc, payload);
      
      if (!error) {
        Battuta = doc["joke"].as<String>();   //SALVA NELLA VARIABILE "BATTUTA" LA RISPOSTA RELATIVA A "JOKE" PRESENTE TRA I DIVERSI CAMPI PRESENTI NELLA RISPOSTA JSON.
      } else {
        Battuta = "Error deserializing JSON";
      }
    } else {
      Battuta = "Error calling API. Code: " + String(httpResponseCode);
    }

    http.end();
  }

  return Battuta; //Ritorna la Battuta o il messaggio di errore.
}


//Funzione che ritorna una stringa contenente data e ora attuale.
String DataOra() {
  String DataOra = "I found nothing to show!";

  // Chiamata all'API
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverNameDate);

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String payload = http.getString();
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, payload);
      if (!error) {
        int Anno = doc["year"].as<int>();   //Essendo dati di tipo numerico, salva in una variabile di tipo INT (4 Byte).
        int Mese = doc["month"].as<int>();
        int Giorno = doc["day"].as<int>();
        int Ora = doc["hour"].as<int>();
        int Minuti = doc["minute"].as<int>();
        MeseOroscopo = Mese;
        
//IMPLEMENTO CONGRUENZA DI ZELLER PER CALCOLARE GIORNO DELLA SETTIMANA IN BASE A DATA.
//Visita https://it.wikipedia.org/wiki/Congruenza_di_Zeller per comprendere i calcoli effettuati.
//La data utilizzata nell'algoritmo è presa dalla risposta JSON effettuata precedentemente.

const char* giorniSettimana[7] = {"Saturday ", "Sunday ", "Monday ", "Tuesday ", "Wednesday ", "Thursday ", "Friday "};
        
if (Mese == 1 || Mese == 2) {
Mese += 12;
Anno -= 1;
}
  
int K = Anno % 100;
int J = Anno / 100;
  
int f = Giorno + ((13 * (Mese + 1)) / 5) + K + (K / 4) + (J / 4) - (2 * J);
int giornoSettimana = f % 7;
String DayName = (giorniSettimana[giornoSettimana]); //Associa il numero ottenuto in base alla posizione del relativo giorno della settimana presente nell'array "giorniSettimana".

String MinutiStr, MeseStr, OraStr, GiornoStr;

//Aggiungi uno zero davanti a cifre singole. ES: 21:3 --> 21:03 / 7/6 --> 07/06.

if (Minuti < 10) {
  MinutiStr = "0" + String(Minuti);
} else {
  MinutiStr = String(Minuti);
}
  
if (Mese < 10) {
  MeseStr = "0" + String(Mese);
} else {
  MeseStr = String(Mese);
}

if (Giorno < 10) {
  GiornoStr = "0" + String(Giorno);
} else {
  GiornoStr = String(Giorno);
} 

if (Ora == 0) {
  OraStr = "0" + String(Ora);
} else {
  OraStr = String(Ora);
}

DataOra = "Time:  " + String(Ora) + ":" + String(Minuti) + " on " + DayName + " " + String(Giorno) + "/" + String(Mese);  //Inserisci informazioni elaborate in una stringa completa.
     
  } else {
    DataOra = "Error deserializing JSON";
  }
  } else {
    DataOra = "Error calling API. Code: " + String(httpResponseCode);
  }

    http.end();
  }

  return DataOra; //Ritorna la stringa contenente data e ora o il messaggio di errore.
}

//Funzione che ritorna una notizia.
String News() {
  String Notizia = "I found nothing to show!"; //Valore di default in caso di errore.

  // Chiamata all'API
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverNameNews);

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String payload = http.getString();
      DynamicJsonDocument doc(2048);
      DeserializationError error = deserializeJson(doc, payload);
      
      if (!error) {
        
        //Estrae l'array di articoli JSON dalla risposta della richiesta HTTP
        JsonArray articles = doc["articles"];

        //Estrae il primo oggetto articolo dall'array
        JsonObject articles_0 = articles[0];

//Inzializzazioni variabili in cui scrivere la risposta. 
//Le const char* corrispondono ad un puntatore a carattere costante (*), utile se si desidera garantire che il contenuto di una stringa non venga modificato accidentalmente all'interno di una funzione o di un contesto in cui non dovrebbe essere modificato.
const char* name0;
const char* title0;
const char* description0;
const char* url0;

//Verifica se l'oggetto articolo estratto è valido
if (articles_0) {
  //Estrae e memorizza il nome della fonte dell'articolo
  name0 = articles_0["source"]["name"];
  
  //Estrae e memorizza il titolo dell'articolo
  title0 = articles_0["title"];
  
  //Estrae e memorizza la descrizione dell'articolo
  description0 = articles_0["content"];
}
        
Notizia = "News from: " + String(name0) + ": " + String(title0) + ", delving deeper: " + String(description0) + "..."; //Inserisci informazioni elaborate in una stringa completa.
  
  } else {
     Notizia = "Error deserializing JSON";
      }
    } else {
      Notizia = "Error calling API. Code: " + String(httpResponseCode);
    }

    http.end();
  }
 
  return Notizia; // Ritorna la notizia o il messaggio di errore
}

//Funzione che ritorna una serie di informazioni relative al meteo corrente.
String Weather() {
  String Weather = "I found nothing to show!"; //Valore di default in caso di errore.

  //Chiamata all'API
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverNameWeather);

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String payload = http.getString();
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        //Ottiene il valore di "main" all'interno del primo oggetto ([0]) di "weather" e lo converte in una stringa.
        String main = doc["weather"][0]["main"].as<String>();
        
        //Ottiene il valore di "description" all'interno del primo oggetto ([0]) di "weather" e lo converte in una stringa.
        String description = doc["weather"][0]["description"].as<String>();
        
        //Ottiene il valore di "temp_min" all'interno di "main" e lo converte in un valore float (Valore decimale).
        float temp_min = doc["main"]["temp_min"].as<float>();
        
        //Ottiene il valore di "temp_max" all'interno di "main" e lo converte in un valore float (Valore decimale).
        float temp_max = doc["main"]["temp_max"].as<float>();
        
        //Ottiene il valore di "humidity" all'interno di "main" e lo converte in un valore intero.
        int humidity = doc["main"]["humidity"].as<int>();
        
        //Ottiene il valore di "feels_like" all'interno di "main" e lo converte in un valore float (Valore decimale).
        float feels_like = doc["main"]["feels_like"].as<float>();
        
        //Ottiene il valore di "name" e lo converte in una stringa, rappresentante il nome della città.
        String city = doc["name"].as<String>();

        //Unisci tutte le informazioni in un'unica stringa.
        Weather = ("Current weather in " + String(city) + ": " + String(main) + ", " + String(description) + ". Taking a look at the temperatures: Temp Max " + String(temp_max) + "  Temp Min " + String(temp_min) + "  Perceived Temp " + String(feels_like) + " The humidity rate is: " + String(humidity) + "%");

        //Associa una breve frase in base alle condizioni atmosferiche attuali.
        //Tutte le condizioni atmosferiche che possono capitare nella risposta JSON sono elencate al seguente link: https://openweathermap.org/weather-conditions#Weather-Condition-Codes-2
        if (main == "Clouds") {
          Weather = String(Weather) + " Let's hope the clouds go away!";
        } 
        if (main == "Rain") {
          Weather = String(Weather) + " Let's hope it stops raining soon!";
        }
        if (main == "Thunderstorm") {
          Weather = String(Weather) + " Thunder is so scary! Let's hope they stop soon!";
        }
        if (main == "Snow") {
          Weather = String(Weather) + " Lot of snow! Watch out for the road!";
        }
        if (main == "Clear") {
          Weather = String(Weather) + " The sky is clean, what a sight!";
        }
        if (main == "Fog") {
          Weather = String(Weather) + " In this fog you can't see anything, watch out for other cars!";
        }
        if (main == "Drizzle") {
          Weather = String(Weather) + " Unfortunately it's not raining hard, let's hope it stops!";
        }
        
        //Aggiungere qui altre eventuali condizioni.
        
      } else {
        Serial.println("deserializeJson() failed: " + String(error.c_str()));
      }
    } else {
      Serial.println("Error in HTTP request");
    }

    http.end();
  }

  return Weather; //Ritorna la stringa con il meteo o il messaggio di errore.
}

//Funzione che ritorna una serie di informazioni relative al meteo corrente.
String Oroscope() {
String Oroscopo = "I found nothing to show!"; //Valore di default in caso di errore.
String segnoMese = (segni[MeseOroscopo]); //Associa il numero del mese alla posizione del relativo segno zodiacale presente nell'array "segni".

//L'URL si trova nella funzione perchè fa riferimento ad una variabile inizializzata non a livello globale ma all'interno della medesima funzione.
String serverNameOroscope = "https://horoscope-app-api.vercel.app/api/v1/get-horoscope/daily?sign=" + String(segnoMese) + "&day=TODAY";  
 
  //Chiamata all'API
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverNameOroscope);

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String payload = http.getString();
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, payload);
      if (!error) {
    //Estrae il valore corrispondente a "horoscope_data" all'interno di "data" (nella risposta JSON) e lo converte in una stringa, assegnandolo alla variabile "OroscopoFrase"
    String OroscopoFrase = doc["data"]["horoscope_data"].as<String>();

    //Unisci tutte le informazioni in un'unica stringa.
    Oroscopo = "Today's horoscope for " + String(segnoMese) + ": " + String(OroscopoFrase);

      } else {
        Oroscopo = "Error deserializing JSON";
      }
    } else {
      Oroscopo = "Error calling API. Code: " + String(httpResponseCode);
    }

    http.end();
  }
    
  return Oroscopo; //Ritorna previsione o il messaggio di errore.
}

//Copyright © [2024] [Gabriele Parietti]
