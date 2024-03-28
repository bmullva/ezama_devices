void define_page() {

  page = R"(
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>EZAMA Circuit Toggle</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f2f2f2;
            text-align: center; /* Center align content horizontally */
            margin: 0;
        }

        /* Updated styling for labels "Ch1" and "Ch2" */
        .switch-label {
            margin-right: 10px;
            font-size: 24px; /* Bigger font size */
            font-weight: bold; /* Bolder font weight */
            font-family: 'Arial Black', sans-serif; /* Fancier font */
            color: #2196F3; /* Blue color */
            display: inline-block; /* Prevent labels from breaking to new lines */
        }

        .channel {
            margin-bottom: 20px; /* Add vertical spacing between channels */
        }

        .switch {
            position: relative;
            display: inline-block;
            width: 60px;
            height: 30px;
        }

        .switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }

        .slider {
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #ccc;
            border-radius: 30px;
            transition: 0.4s;
        }

        .slider:before {
            position: absolute;
            content: "";
            height: 26px;
            width: 26px;
            left: 2px;
            bottom: 2px;
            background-color: white;
            border-radius: 50%;
            transition: 0.4s;
        }

        input:checked + .slider {
            background-color: #2196F3;
        }

        input:checked + .slider:before {
            transform: translateX(26px);
        }

        /* Style for the sections */
        .section {
            margin-top: 20px; /* Add spacing between sections */
        }
    </style>
  </head>
  <body>
  <h1>Ezama Circuit Manager</h1>
  <br>

  <!--<div class="channel">
    <span class="switch-label">Ch1 :</span>)";
    page += R"(<meter id="meterAmps1" value=')";
    page += String(Amps1_TRMS);
    page += R"(' min='0' max='20'></meter>
    <label class="switch">
      <input type='checkbox' id='toggleGPIO25')";         
      page += gpio25State ? "checked" : "";
      page += R"(>
      <span class="slider"></span>
    </label>
  </div>-->

  <div class="channel">
    <span class="switch-label">Ch1 :</span>)";
    // page += String(Amps1_TRMS); // Display the numeric value directly
    page += R"(<meter id="meterAmps1" value=')";
    page += String(Amps1_TRMS);
    page += R"(' min='0' max='20'></meter>
    <label class="switch">
      <input type='checkbox' id='toggleGPIO25')";         
      page += gpio25State ? "checked" : "";
      page += R"(>
      <span class="slider"></span>
    </label>
</div>

  
  <div class="channel">
    <span class="switch-label">Ch2 :</span>)";
    page += R"(<meter id="meterAmps2" value=')";
    page += String(Amps2_TRMS);
    page += R"(' min='0' max='20'></meter>
    <label class="switch">
      <input type='checkbox' id='toggleGPIO22')";         
      page += gpio22State ? "checked" : "";
      page += R"(>
      <span class="slider"></span>
    </label>
  </div>
    
    
  <div class="channel">
    <span class="switch-label">Ch3 :</span>)";
    page += R"(<meter id="meterAmps3" value=')";
    page += String(Amps3_TRMS);
    page += R"(' min='0' max='20'></meter>
    <label class="switch">
      <input type='checkbox' id='toggleGPIO21')";         
      page += gpio21State ? "checked" : "";
      page += R"(>
      <span class="slider"></span>
    </label>
  </div>
      
  <div class="channel">
    <span class="switch-label">Ch4 :</span>)";
    page += R"(<meter id="meterAmps4" value=')";
    page += String(Amps4_TRMS);
    page += R"(' min='0' max='20'></meter>
    <label class="switch">
      <input type='checkbox' id='toggleGPIO19')";         
      page += gpio19State ? "checked" : "";
      page += R"(>
      <span class="slider"></span>
    </label>
  </div>
      
  <div class="channel">
    <span class="switch-label">Ch5 :</span>)";
    page += R"(<meter id="meterAmps5" value=')";
    page += String(Amps5_TRMS);
    page += R"(' min='0' max='20'></meter>
    <label class="switch">
      <input type='checkbox' id='toggleGPIO17')";         
      page += gpio17State ? "checked" : "";
      page += R"(>
      <span class="slider"></span>
    </label>
  </div>
      
  <div class="channel">
    <span class="switch-label">Ch6 :</span>)";
    page += R"(<meter id="meterAmps6" value=')";
    page += String(Amps6_TRMS);
    page += R"(' min='0' max='20'></meter>
    <label class="switch">
      <input type='checkbox' id='toggleGPIO16')";         
      page += gpio16State ? "checked" : "";
      page += R"(>
      <span class="slider"></span>
    </label>
  </div>

  </body>

    <script>
      // Function to update the <meter> element with the Amps1_TRMS
      function updateAmps1_TRMS() {
        fetch('/getAmps1_TRMS')
          .then(response => response.text())
          .then(value => {
            const meterElementAmps1 = document.getElementById('meterAmps1');
            meterElementAmps1.value = parseFloat(value);    
            setTimeout(updateAmps1_TRMS, 1000); // Fetch and update every second
          })
          .catch(error => {
            console.error('Error fetching Amps1_TRMS:', error);
            setTimeout(updateAmps1_TRMS, 1000); // Retry after 1 second on error
          });
      }    
      updateAmps1_TRMS();
    </script>

<script>
  // Function to update the Ch1 value with the Amps1_TRMS
  function updateAmps1_TRMSX() {
    fetch('/getAmps1_TRMSX')
      .then(response => response.text())
      .then(value => {
        const channel1Element = document.querySelector('.channel span'); // Select the span element in the Ch1 div
        channel1Element.textContent = `Ch1 : ${parseFloat(value)}`;    
        setTimeout(updateAmps1_TRMSX, 1000); // Fetch and update every 1 second
      })
      .catch(error => {
        console.error('Error fetching Amps1_TRMSX:', error);
        setTimeout(updateAmps1_TRMSX, 1000); // Retry after 1 second on error
      });
  }    
  updateAmps1_TRMSX();
</script>


    
    <script>
      var gpio25Checkbox = document.getElementById('toggleGPIO25');
      gpio25Checkbox.addEventListener('change', function() {
        fetch('/toggleGPIO25?state=' + (gpio25Checkbox.checked ? 'on' : 'off'));
      });
    </script>

    <script>
      function updateAmps2_TRMS() {
        fetch('/getAmps2_TRMS')
          .then(response => response.text())
          .then(value => {
            const meterElementAmps2 = document.getElementById('meterAmps2');
            meterElementAmps2.value = parseFloat(value);    
            setTimeout(updateAmps2_TRMS, 1000);
          })
          .catch(error => {
            console.error('Error fetching Amps2_TRMS:', error);
            setTimeout(updateAmps2_TRMS, 1000); // Retry after 1 second on error
          });
      }    
      updateAmps2_TRMS();
    </script>
    <script>
      var gpio22Checkbox = document.getElementById('toggleGPIO22');
      gpio22Checkbox.addEventListener('change', function() {
        fetch('/toggleGPIO22?state=' + (gpio22Checkbox.checked ? 'on' : 'off'));
      });
    </script>

    <script>
      function updateAmps3_TRMS() {
        fetch('/getAmps3_TRMS')
          .then(response => response.text())
          .then(value => {
            const meterElementAmps3 = document.getElementById('meterAmps3');
            meterElementAmps3.value = parseFloat(value);    
            setTimeout(updateAmps3_TRMS, 1000);
          })
          .catch(error => {
            console.error('Error fetching Amps3_TRMS:', error);
            setTimeout(updateAmps3_TRMS, 1000); // Retry after 1 second on error
          });
      }    
      updateAmps3_TRMS();
    </script>
    <script>
      var gpio21Checkbox = document.getElementById('toggleGPIO21');
      gpio21Checkbox.addEventListener('change', function() {
        fetch('/toggleGPIO21?state=' + (gpio21Checkbox.checked ? 'on' : 'off'));
      });
    </script>
    
    <script>
      function updateAmps4_TRMS() {
        fetch('/getAmps4_TRMS')
          .then(response => response.text())
          .then(value => {
            const meterElementAmps4 = document.getElementById('meterAmps4');
            meterElementAmps4.value = parseFloat(value);    
            setTimeout(updateAmps4_TRMS, 1000);
          })
          .catch(error => {
            console.error('Error fetching Amps4_TRMS:', error);
            setTimeout(updateAmps4_TRMS, 1000); // Retry after 1 second on error
          });
      }    
      updateAmps4_TRMS();
    </script>
    <script>
      var gpio19Checkbox = document.getElementById('toggleGPIO19');
      gpio19Checkbox.addEventListener('change', function() {
        fetch('/toggleGPIO19?state=' + (gpio19Checkbox.checked ? 'on' : 'off'));
      });
    </script>

    <script>
      function updateAmps5_TRMS() {
        fetch('/getAmps5_TRMS')
          .then(response => response.text())
          .then(value => {
            const meterElementAmps5 = document.getElementById('meterAmps5');
            meterElementAmps5.value = parseFloat(value);    
            setTimeout(updateAmps5_TRMS, 1000);
          })
          .catch(error => {
            console.error('Error fetching Amps5_TRMS:', error);
            setTimeout(updateAmps5_TRMS, 1000); // Retry after 1 second on error
          });
      }    
      updateAmps5_TRMS();
    </script>
    <script>
      var gpio17Checkbox = document.getElementById('toggleGPIO17');
      gpio17Checkbox.addEventListener('change', function() {
        fetch('/toggleGPIO17?state=' + (gpio17Checkbox.checked ? 'on' : 'off'));
      });
    </script>

    <script>
      function updateAmps6_TRMS() {
        fetch('/getAmps6_TRMS')
          .then(response => response.text())
          .then(value => {
            const meterElementAmps6 = document.getElementById('meterAmps6');
            meterElementAmps6.value = parseFloat(value);    
            setTimeout(updateAmps6_TRMS, 1000);
          })
          .catch(error => {
            console.error('Error fetching Amps6_TRMS:', error);
            setTimeout(updateAmps6_TRMS, 1000); // Retry after 1 second on error
          });
      }    
      updateAmps6_TRMS();
    </script>
    <script>
      var gpio16Checkbox = document.getElementById('toggleGPIO16');
      gpio16Checkbox.addEventListener('change', function() {
        fetch('/toggleGPIO16?state=' + (gpio16Checkbox.checked ? 'on' : 'off'));
      });
    </script>
        
    </body>
    </html>
  )";




  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){    
    request->send(200, "text/html", page);
  });



  server.on("/getAmps1_TRMS", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(Amps1_TRMS));
  });
  server.on("/getAmps1_TRMSX", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(Amps1_TRMS,2));
  });
  server.on("/toggleGPIO25", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasArg("state")) {
      String state = request->arg("state");
      gpio25State = (state == "on");
      digitalWrite(25, gpio25State ? LOW : HIGH); // Set GPIO25 state
    }
    request->send(200, "text/plain", gpio25State ? "on" : "off");
  });
  server.on("/getGPIO25", HTTP_GET, [](AsyncWebServerRequest *request){
    int gpio25Value = digitalRead(25);
    request->send(200, "text/plain", String(gpio25Value));
  });

  server.on("/getAmps2_TRMS", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(Amps2_TRMS));
  });
  server.on("/toggleGPIO22", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasArg("state")) {
      String state = request->arg("state");
      gpio22State = (state == "on");
      digitalWrite(22, gpio22State ? LOW : HIGH); // Set GPIO22 state
    }
    request->send(200, "text/plain", gpio22State ? "on" : "off");
  });
  server.on("/getGPIO22", HTTP_GET, [](AsyncWebServerRequest *request){
    int gpio22Value = digitalRead(22);
    request->send(200, "text/plain", String(gpio22Value));
  });

  server.on("/getAmps3_TRMS", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(Amps3_TRMS));
  });
  server.on("/toggleGPIO21", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasArg("state")) {
      String state = request->arg("state");
      gpio21State = (state == "on");
      digitalWrite(21, gpio21State ? LOW : HIGH); // Set GPIO21 state
    }
    request->send(200, "text/plain", gpio21State ? "on" : "off");
  });
  server.on("/getGPIO21", HTTP_GET, [](AsyncWebServerRequest *request){
    int gpio21Value = digitalRead(21);
    request->send(200, "text/plain", String(gpio21Value));
  });

  server.on("/getAmps4_TRMS", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(Amps4_TRMS));
  });
  server.on("/toggleGPIO19", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasArg("state")) {
      String state = request->arg("state");
      gpio19State = (state == "on");
      digitalWrite(19, gpio19State ? LOW : HIGH); // Set GPIO19 state
    }
    request->send(200, "text/plain", gpio19State ? "on" : "off");
  });
  server.on("/getGPIO19", HTTP_GET, [](AsyncWebServerRequest *request){
    int gpio19Value = digitalRead(19);
    request->send(200, "text/plain", String(gpio19Value));
  });

  server.on("/getAmps5_TRMS", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(Amps5_TRMS));
  });
  server.on("/toggleGPIO17", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasArg("state")) {
      String state = request->arg("state");
      gpio17State = (state == "on");
      digitalWrite(17, gpio17State ? LOW : HIGH); // Set GPIO17 state
    }
    request->send(200, "text/plain", gpio17State ? "on" : "off");
  });
  server.on("/getGPIO17", HTTP_GET, [](AsyncWebServerRequest *request){
    int gpio17Value = digitalRead(17);
    request->send(200, "text/plain", String(gpio17Value));
  });

  server.on("/getAmps6_TRMS", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(Amps6_TRMS));
  });
  server.on("/toggleGPIO16", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasArg("state")) {
      String state = request->arg("state");
      gpio16State = (state == "on");
      digitalWrite(16, gpio16State ? LOW : HIGH); // Set GPIO16 state
    }
    request->send(200, "text/plain", gpio16State ? "on" : "off");
  });
  server.on("/getGPIO16", HTTP_GET, [](AsyncWebServerRequest *request){
    int gpio16Value = digitalRead(16);
    request->send(200, "text/plain", String(gpio16Value));
  });
  
  server.begin();
}
