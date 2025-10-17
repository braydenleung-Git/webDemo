#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <esp_wifi.h>
#include <time.h>
#include <Arduino_JSON.h> //make sure to have the Ardunio_JSON library by Ardunio
#include "ultrasonics.h"
#include "imu.h"


bool toggleSerial = false;
extern bool toggleIMU, toggleUltraSonics;
extern float distanceCm, distanceInch;
extern float a_x, a_y, a_z, g_x, g_y, g_z, temp_c;
extern void triggerUltraSonics(bool);
const char *ssid = "webServer";
const char *password = "12345678";
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1); 
//instanciation of server
DNSServer dnsServer;
WebServer server(80);

//defines the web interface
//the webpage is a direct copy of the index.html
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<meta charset="UTF-8">
<!--
    TODO:
-->
<html>
    <head>
        <style>
            body {
                font-size: 18px;
            }
            #title{
                font-size: 28px;
            }
            
            @media (min-width: 768px) {
                body {
                font-size: 30px;
                }
                #title{
                    font-size: 40px;
                }
            }
        </style>
    </head>
    <body>
        <div id="control_Panel" style="display:block;">
            <p><label id='title'>Click button to toggle sensor </label></p>
            <p><label>Ultrasonic:</label><button class="sensor" id="ultrasonic" onclick="toggleSerial.call(this)"></button></p>
            <p><label>IMU:</label><button class="sensor" id="imu" onclick="toggleSerial.call(this)"></button></p>
            <p><label>All:</label><button id="all" onclick="toggleSerial.call(this)"></button></p>
        </div>
        
        <div id="serial_dataPanel">
            <p>Distance from sensor (cm) || <label class="sLbl"></label></p>
            <p>Distance from sensor (in) || <label class="sLbl"></label></p>
            <p>Acceleration(m/s^2) || x: <label class="sLbl"></label> y: <label class="sLbl"></label> z:<label class="sLbl"></label></p>
            <p>Gyro(rad/s) || x: <label class="sLbl"></label> y: <label class="sLbl"></label> z:<label class="sLbl"></label></p>
            Temperature || <label class="sLbl"></label>°C
        </div>
    </body>

    <script>
        async function toggleSerial(){
            //preventing another press
            this.disabled = true;
            sensor_id = this.getAttribute('id');
            try {
                const params = new URLSearchParams();
                params.append("sensor", sensor_id);
                const rspd = await fetch('/toggleSensor', {
                method: "POST",
                headers: { "Content-Type": "application/x-www-form-urlencoded" },
                body: params
                });
                const respond = await rspd.json();
            }catch (e){
                console.error('Failed to toggle sensor',e);
            }
            this.disabled = false;
            fetchSerialStatus();
        }
        async function fetchSerialStatus(){
            try {
                const rspd = await fetch('/sensorStatus');
                const data = await rspd.json();
                keys = Object.keys(data);
                //if all value is true, and all the sensor are also enabled, do absolutely nothing
                if(data[keys[0]]){
                    //essentially take all .sensor class objects, convert the result into an array
                    //loop through the array, if all the elements are equal to "Enabled", skip the rest of the method
                    if(Array.from(document.querySelectorAll(".sensor")).every(element=> element.innerHTML === "Enabled")){
                        return;
                    }
                }
                for (const key of keys) {
                    var toggled = data[key];
                    element = document.getElementById(key);
                    if (!element) continue;
                    if(!toggled){
                        element.style.backgroundColor= 'red';
                        element.innerHTML = 'Disabled';
                    }
                    else{
                        element.style.backgroundColor = 'green';
                        element.innerHTML = 'Enabled';
                    }
                }
            } catch (e){
                console.error('Failed to fetch Serial Status',e);
            }
        }
        var cm, inch, a_x, a_y, a_z, g_x, g_y, g_z, temp;

        async function fetchSensorData(){
            try{
                const rspd = await fetch('/serialData');
                const data = await rspd.json();
                cm = data.cm || 0;//shorthand, if there is no data for the first value, replace it with the other
                inch = data.inch || 0;
                a_x = data.a_x || 0;
                a_y = data.a_y || 0;
                a_z = data.a_z || 0;
                g_x = data.g_x || 0;
                g_y = data.g_y || 0;
                g_z = data.g_z || 0;
                temp = data.temp || 0;
            }catch (e){
                console.error('Failed to fetch sensor data',e);
            }
            updateData();
        }

        function updateData(){
            try{
                labels = document.querySelectorAll(".sLbl");
                labels[0].innerText = cm;
                labels[1].innerText = inch;
                labels[2].innerText = a_x;
                labels[3].innerText = a_y;
                labels[4].innerText = a_z;
                labels[5].innerText = g_x;
                labels[6].innerText = g_y;
                labels[7].innerText = g_z;
                labels[8].innerText = temp;
            }catch (e){
                console.error('Failed to update labels',e);
            }
        }
        //make it such that at each 1 second, it will call fetchSensorData
        //don't go lower than 1 second
        //these are just two ways of how you can run a method
        setInterval(fetchSerialStatus(),2000);
        //this is called a lambda function/body,google it 
        setInterval(()=>{
            fetchSensorData();
        },1000)
    </script>
</html>
)=====";


void handleSensorToggle(){
    String sensor = server.arg("sensor");
    if(sensor.equals("all")){
        toggleSerial = !toggleSerial;
        Serial.println(toggleSerial);
        toggleUltraSonics = toggleSerial;
        toggleIMU = toggleSerial;
        Serial.println(sensor + " sensor state changed to :"+toggleSerial);
        server.send(200,"application/json","{\"status\":\"success\"}");
    } else {
    //cpp can do string based switch...case, but not java !
    Serial.print(sensor);
    switch (sensor.charAt(0)){
        case 'u' :
            toggleUltraSonics = !toggleUltraSonics;
            Serial.println(sensor + " sensor state changed to :"+toggleUltraSonics);
            server.send(200,"application/json","{\"status\":\"success\"}");
            toggleSerial = toggleSerial? !toggleSerial : toggleSerial;
            break;
        case 'i':
            toggleIMU = !toggleIMU;
            Serial.println(sensor + " sensor state changed to :"+toggleIMU);
            server.send(200,"application/json","{\"status\":\"success\"}");
            toggleSerial = toggleSerial? !toggleSerial : toggleSerial;
            break;
        default : 
            Serial.println("Error : Unexpected sensor data");
            server.send(404,"application/json","Sensor value not found");//unsure if this is correct
        }
    }
}

void handleData(){
    JSONVar doc;//creates a json document inside the memory of the esp32
    doc["cm"] = distanceCm;
    doc["inch"] = distanceInch;
    doc["a_x"] = a_x;
    doc["a_y"] = a_y;
    doc["a_z"] = a_z;
    doc["g_x"] = g_x;
    doc["g_y"] = g_y;
    doc["g_z"] = g_z;
    doc["temp"] = temp_c;
    server.send(200,"application/json",JSON.stringify(doc));
}

void handleSensorStatus(){
    //Serial.println("Sensor Status Update Requested");
    JSONVar doc;
    doc["all"] = toggleSerial;
    doc["ultrasonic"] = toggleUltraSonics;
    doc["imu"] = toggleIMU;
    server.send(200,"application/json",JSON.stringify(doc));
}
//This method is meant more than just the index request, but also any error, 
void handleAllRequests(){
    server.send(200, "text/html", MAIN_page);
}

void setupWebServer(void){
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    Serial.println("Webpage started hosting at 192.168.4.1");
    dnsServer.start(DNS_PORT, "*", apIP);
    delay(100);
    Serial.println(F(""));
    IPAddress myIP = WiFi.softAPIP();
    Serial.print(F("AP IP address: "));
    Serial.println(myIP);
    server.on("/",HTTP_GET,handleAllRequests);
    //handles serial toggle
    server.on("/toggleSensor",HTTP_POST, handleSensorToggle);
    server.on("/serialData",HTTP_GET, handleData);
    server.on("/sensorStatus",HTTP_GET,handleSensorStatus);
    server.onNotFound(handleAllRequests);//refresh the page upon error(not sure why)
    server.begin();
}