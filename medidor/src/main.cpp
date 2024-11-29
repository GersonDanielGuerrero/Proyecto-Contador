#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <HTTPClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define ANCHO_PANTALA 128
#define ALTURA_PANTALLA 64

#define SDA_PIN 26
#define SCL_PIN 27

Adafruit_SSD1306 display(ANCHO_PANTALA,ALTURA_PANTALLA,&Wire);

//GPIO 35
#define VOLTAJE_PIN 32
#define CORRIENTE_PIN 35

const char* ssidLocal = "MEDIDOR_0001";
const char* contrasenaLocal = "12345678";
const String nombreDispositivo = "Medidor_0001";

const String urlServer = "http://192.168.113.60:8000/medicion/";

int freqMedida = 1000;
int voltajeMax = 18000;
int corrienteMax = 8588;

int voltajeMedido = 0;
int corrienteMedida = 0;
int voltajemV = 0;
int corrientemA = 0;
unsigned long potenciauW = 0;
unsigned long long consumouWh = 0;

//Crear archivo html para el index
const char* index_html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Document</title>
</head>
<body>
    <div class="container">
        <h1 class="titulo">Configuracion del medidor</h1>
            <form class="card">
                    <h2 class = "sub-titulo">Configuración de la red</h2>
                    <label for="ssid" class="form-label">SSID</label>
                    <input type="text" class="form-control" id="ssid" aria-describedby="ssidHelp">
                    <label for="contraseña" class ="form-label">Contraseña</label>
                    <input type="password" class="form-control" id="contraseña" aria-describedby="contraseñaHelp">
                    <button id="btnGuardarRed" class="btn btn-primary">Guardar</button>
            </form>
    </div>
    <style>
        .titulo, sub-titulo {
            text-align: center;
        }
        .titulo {
            font-size: 2.75em;
        }
        .container {
            display: flex;
            justify-content: start;
            align-items: center;
            height: 100vh;
            flex-direction: column;
        }
        .card {
            width: 50%;
            background-color: #d2e5fc;
            border-radius: 15px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
            padding: 20px;
            display: flex;
            flex-direction: column;
            align-items: center;
        }
    </style>
    <script>
        document.addEventListener("DOMContentLoaded", function() {
            var ssid = document.getElementById("ssid");
            var contraseña = document.getElementById("contraseña");
            var boton = document.getElementById("btnGuardarRed");
            
            boton.addEventListener("click", async function(event) {
                // Evitar que se recargue la página
                event.preventDefault();
                
                try {
                    let respuesta = await fetch("http://192.168.4.1:80/configuracion/red", {
                        method: "POST",
                        headers: {
                            "Content-Type": "application/json"
                        },
                        body: JSON.stringify({
                            ssid: ssid.value,
                            contraseña: contraseña.value
                        })
                    });
    
                    if (respuesta.ok) {
                        alert("Configuración guardada");
                    } else {
                        alert("Error al guardar la configuración");
                    }
                } catch (error) {
                    alert("Error al comunicarse con el servidor");
                    console.error(error);
                }
            });
        });
    </script>    
</body>
</html>
)rawliteral";



void realizarMedida(){
  voltajeMedido = analogRead(VOLTAJE_PIN);
  corrienteMedida = analogRead(CORRIENTE_PIN);

  voltajemV = map(voltajeMedido, 0, 4095, 0, voltajeMax);
  corrientemA = map(corrienteMedida, 0, 4095, 0, corrienteMax);
  potenciauW = voltajemV*corrientemA;
  //calcular la cantidad de segundos entre mediciones y dividir por 3600 para obtener la cantidad de horas. freqMedida es en milisegundos
  //consumouWh += (potenciauW*freqMedida)/3600000;
  consumouWh += potenciauW/1000*freqMedida/1000/3600;

  
  Serial.print(voltajemV);
  Serial.print("mV | ");
  Serial.print(corrientemA);
  Serial.print("mA | ");
  Serial.print(potenciauW/1000);
  Serial.print("mW | ");
  Serial.print("Consumo: ");
  //Serial.print(consumouWh/1000);
  if (consumouWh<1000){
    Serial.print(consumouWh);
    Serial.println("uWh");
  }
  else if (consumouWh<1000000){
    Serial.print(consumouWh/1000);
    Serial.println("mWh");
  }
  else if (consumouWh<1000000000){
    Serial.print(consumouWh/1000000);
    Serial.println("Wh");
  }
  else{
    Serial.print(consumouWh/1000000000);
    Serial.println("kWh");
  }
}

void imprimirMedida(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Potencia: ");
  display.print(potenciauW/1000);
  display.print("mW");
  display.setCursor(0,30);
  display.print("Consumo: ");
  if (consumouWh<1000){
    display.print(consumouWh);
    display.print("uWh");
  }
  else if (consumouWh<1000000){
    display.print(consumouWh/1000);
    display.print("mWh");
  }
  else if (consumouWh<1000000000){
    display.print(consumouWh/1000000);
    display.print("Wh");
  }
  else{
    display.print(consumouWh/1000000000);
    display.print("kWh");
  }
  display.display();
}

void enviarMedida(){
  HTTPClient http;
  http.begin(urlServer);
  
  http.addHeader("Content-Type", "application/json");
  String json = "{\"potencia\":"+String(potenciauW)+",\"consumo\":"+String(consumouWh)+",\"dispositivo\":\""+nombreDispositivo+"\"}";
  int httpCode = http.POST(json);
  if (httpCode>0){
    String payload = http.getString();
  }
  else{
    Serial.println("Error en la peticion");
  }

  http.end();
}
void conectarAInternet(String ssid, String contra){
  WiFi.begin(ssid, contra);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Conectando a la red");
  }
  Serial.println("Conectado a la red");
  Serial.println(WiFi.localIP());

}

void iniciarServidor(){
  Serial.println("Iniciando servidor");
  WiFi.softAP(ssidLocal, contrasenaLocal);

  IPAddress IP = WiFi.softAPIP();
  
  Serial.print("IP Address: ");
  Serial.println(IP);

AsyncWebServer server(80);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Solicitud de la pagina principal");
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", index_html);
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");
    request->send(response);
  });
}
void medidaTask(void *pvParameters){


  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  while (true)
  {
    realizarMedida();
    imprimirMedida();
    if (potenciauW>0){
      enviarMedida();
    }
    delay(freqMedida);
  }
  
}


void setup() {
  Serial.begin(9600);
  pinMode(VOLTAJE_PIN, INPUT);
  pinMode(CORRIENTE_PIN, INPUT);
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC,0x3C)){
    Serial.println(F("Error con pantalla"));
    for(;;);
  }

  //iniciarServidor();
  
  conectarAInternet("Telefonodedani", "aguacate");
  xTaskCreatePinnedToCore(
    medidaTask,
    "Medida Task",
    10000,
    NULL,
    1,
    NULL,
    1
  );
}

void loop(){
}
