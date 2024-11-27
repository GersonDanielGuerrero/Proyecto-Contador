#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

//GPIO 35
#define VOLTAJE_PIN 32
#define CORRIENTE_PIN 33

const char* ssidLocal = "MEDIDOR_0001";
const char* contrasenaLocal = "12345678";

int freqMedida = 1000;
int voltajeMax = 17600;
int corrienteMax = 5588;

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
    <!--Bootstrap cdn-->
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-QWTKZyjpPEjISv5WaRU9OFeRpok6YctnYmDr5pNlyT2bRjXh0JMhjY6hW+ALEwIH" crossorigin="anonymous">
</head>
<body>
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js" integrity="sha384-YvpcrYf0tY3lHB60NNkmXc5s9fDVZLESaAA55NDzOxhy9GkcIdslK1eN7N6jIeHz" crossorigin="anonymous"></script>
    <h1 class="text-center">
    Configuracion del medidor</h1>
    <!--Seccion de configuracion de conexion - debe ser un collapsable-->
    <div class="container">
        <div class="row card">
            <div class="col header">
                <h2 class ="card-header" >Configuracion de conexion</h2>
                    <form>
                        <div class="mb-3">
                            <label for="ssid" class="form-label">SSID</label>
                            <input type="text" class="form-control" id="ssid" aria-describedby="ssidHelp">
                            <label for="contraseña" class ="form-label">Contraseña</label>
                            <input type="password" class="form-control" id="contraseña" aria-describedby="contraseñaHelp">
                            <button id="btnGuardarRed" class="btn btn-primary">Guardar</button>
                        </div>
                    </form>
            </div>
        </div>
    </div>
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

AsyncWebServer server(80);

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

void iniciarServidor(){
  Serial.println("Iniciando servidor");
  WiFi.softAP(ssidLocal, contrasenaLocal);

  IPAddress IP = WiFi.softAPIP();
  
  Serial.print("IP Address: ");
  Serial.println(IP);
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
  while (true)
  {
    realizarMedida();
    delay(freqMedida);
  }
  
}


void setup() {
  Serial.begin(9600);
  pinMode(VOLTAJE_PIN, INPUT);
  pinMode(CORRIENTE_PIN, INPUT);

  iniciarServidor();
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
