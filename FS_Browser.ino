// Функция для формирования списка файлов в HTML
void listFiles(String path, String &html) {
  File root = LittleFS.open(path, "r");
  if (!root) {
    Serial.println(F("Failed to open directory"));
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(F("Specified path is not a directory"));
    root.close();
    return;
  }

  File file = root.openNextFile();
  while (file) {
    String name = String(file.name());
    String fullPath = name;
    if (!fullPath.startsWith("/")) {
      if (path.endsWith("/")) fullPath = path + name;
      else if (path == "/") fullPath = "/" + name;
      else fullPath = path + "/" + name;
    }

    if (file.isDirectory()) {
      html += "<li><strong>" + name + "/</strong></li>";
      listFiles(fullPath, html);
    } else {
      html += "<li>";
      html += "  <div class='file-info'>";
      html += "    <a href='/download?file=" + fullPath + "'>" + name + "</a>";
      html += "    <span style='color:#666; font-size:13px;'>(" + String(file.size()) + " bytes)</span>";
      html += "  </div>";
      html += "  <div class='actions'>";
      html += "    <a href='/download?file=" + fullPath + "' class='btn-download'>Скачать</a>";
      html += "    <a href='/edit?file=" + fullPath + "' class='btn-edit'>Редактировать</a>";
      html += "  </div>";
      html += "</li>";
    }
    file = root.openNextFile();
  }
  root.close();
}

void FS_Browser_init() {
  // Главная страница (отдает index.html)
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (LittleFS.exists("/index.html")) {
      File file = LittleFS.open("/index.html", "r");
      if (file) {
        String html = file.readString();
        file.close();
        request->send(200, "text/html", html);
      } else {
        request->send(500, "text/plain", "Failed to open index.html");
      }
    } else {
      request->send(404, "text/plain", "index.html not found.");
    }
  });

  // ЭНДПОИНТ 1: Отдача полного config.json в браузер
  server.on("/get_states", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (LittleFS.exists("/config.json")) {
      File confFile = LittleFS.open("/config.json", "r");
      String jsonContent = confFile.readString();
      confFile.close();
      request->send(200, "application/json", jsonContent);
    } else {
      request->send(200, "application/json", "{}");
    }
  });

  // ЭНДПОИНТ 2: Изменение статуса (ВКЛ/ВЫКЛ) и отправка по LoRa
  server.on("/set_state", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasArg("id") && request->hasArg("state")) {
      int id = request->arg("id").toInt();
      int state = request->arg("state").toInt();

      JsonDocument doc;
      if (LittleFS.exists("/config.json")) {
        File confFile = LittleFS.open("/config.json", "r");
        deserializeJson(doc, confFile);
        confFile.close();
      }

      JsonArray loadsArray = doc["loads"].as<JsonArray>();
      if (!loadsArray.isNull() && id >= 0 && id < loadsArray.size()) {
        loadsArray[id]["pin"] = state;
      }

      Pinout[id] = state;
      sendLoraCommand(String("L") + String(id) + String(state));
      Serial.printf("LoRa Command Sent: Load ID %d -> State %d\n", id, state);

      File confFile = LittleFS.open("/config.json", "w");
      if (confFile) {
        serializeJson(doc, confFile);
        confFile.close();
        request->send(200, "text/plain", "OK");
      } else {
        request->send(500, "text/plain", "Write Error");
      }
    } else {
      request->send(400, "text/plain", "Bad Request");
    }
  });

  // ЭНДПОИНТ 3: Изменение ИМЕНИ нагрузки из браузера
  server.on("/rename_load", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasArg("id") && request->hasArg("name")) {
      int id = request->arg("id").toInt();
      String newName = request->arg("name");

      JsonDocument doc;
      if (LittleFS.exists("/config.json")) {
        File confFile = LittleFS.open("/config.json", "r");
        deserializeJson(doc, confFile);
        confFile.close();
      }

      JsonArray loadsArray = doc["loads"].as<JsonArray>();
      if (!loadsArray.isNull() && id >= 0 && id < loadsArray.size()) {
        loadsArray[id]["name"] = newName;
      }

      File confFile = LittleFS.open("/config.json", "w");
      if (confFile) {
        serializeJson(doc, confFile);
        confFile.close();
        request->send(200, "text/plain", "OK");
      } else {
        request->send(500, "text/plain", "Write Error");
      }
    } else {
      request->send(400, "text/plain", "Bad Request");
    }
  });

  // ЭНДПОИНТ 4: Отдача времени по запросу JS-часов
  server.on("/api/time", HTTP_GET, [](AsyncWebServerRequest *request){
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    char buffer[32];
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &timeinfo);
    request->send(200, "text/plain", buffer);
  });

  // Просмотр Wi-Fi конфигурации
  server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (LittleFS.exists("/config.html")) {
      File file = LittleFS.open("/config.html", "r");
      if (file) {
        String content = file.readString();
        file.close();

        JsonDocument doc;
        if (LittleFS.exists("/config.json")) {
          File confFile = LittleFS.open("/config.json", "r");
          deserializeJson(doc, confFile);
          confFile.close();
        }

        content.replace("{{SSDPName}}", doc["SSDPName"].as<String>());
        content.replace("{{ssidName}}", doc["ssidName"].as<String>());
        content.replace("{{ssidPassword}}", doc["ssidPassword"].as<String>());

        String clientKey = "Client" + String(client_id);
        JsonObject clientObj = doc[clientKey].as<JsonObject>();

        if (!clientObj.isNull()) {
          content.replace("{{timezone}}", String(clientObj["timezone"].as<int>()));
          content.replace("{{off_display_sec}}", String(clientObj["off_display_sec"].as<int>()));
          content.replace("{{allways_on_disp}}", clientObj["allways_on_disp"].as<bool>() ? "checked" : "");
        } else {
          content.replace("{{timezone}}", "0");
          content.replace("{{off_display_sec}}", "60");
          content.replace("{{allways_on_disp}}", "");
        }
        request->send(200, "text/html", content);
      } else { request->send(500, "text/plain", "Failed to open config.html"); }
    } else { request->send(404, "text/plain", "config.html not found."); }
  });

  // Сохранение Wi-Fi конфигурации
  server.on("/save_wifi", HTTP_POST, [](AsyncWebServerRequest *request) {
    JsonDocument doc;
    if (LittleFS.exists("/config.json")) {
      File confFile = LittleFS.open("/config.json", "r");
      deserializeJson(doc, confFile);
      confFile.close();
    }

    if (request->hasArg("SSDPName")) doc["SSDPName"] = request->arg("SSDPName");
    if (request->hasArg("ssidName")) doc["ssidName"] = request->arg("ssidName");
    if (request->hasArg("ssidPassword")) doc["ssidPassword"] = request->arg("ssidPassword");

    String clientKey = "Client" + String(client_id);
    JsonObject clientObj = doc[clientKey].as<JsonObject>();

    if (clientObj.isNull()) {
      clientObj = doc[clientKey].to<JsonObject>();
    }

    if (request->hasArg("timezone")) clientObj["timezone"] = request->arg("timezone").toInt();
    if (request->hasArg("off_display_sec")) clientObj["off_display_sec"] = request->arg("off_display_sec").toInt();
    clientObj["allways_on_disp"] = (request->arg("allways_on_disp") == "on");

    File confFile = LittleFS.open("/config.json", "w");
    if (confFile) {
      serializeJson(doc, confFile);
      confFile.close();

      // Синхронизируем внутреннее состояние переменных ESP
      timezone = clientObj["timezone"].as<int>();
      off_display_sec = clientObj["off_display_sec"].as<int>();
      allways_on_disp = clientObj["allways_on_disp"].as<bool>();

      request->send(200, "text/plain", "Configuration saved");
    } else { request->send(500, "text/plain", "Failed to save configuration"); }
  });

  // Просмотр списка файлов в LittleFS
  server.on("/files", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (LittleFS.exists("/files.html")) {
      File file = LittleFS.open("/files.html", "r");
      if (file) {
        String content = file.readString();
        file.close();
        String fileRows = "";
        listFiles("/", fileRows);
        content.replace("<!-- %FILE_LIST% -->", fileRows);
        request->send(200, "text/html", content);
      } else {
        request->send(500, "text/plain", "Failed to open files.html");
      }
    } else {
      request->send(404, "text/plain", "files.html not found.");
    }
  });

  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasArg("file")) {
      String filePath = request->arg("file");
      if (LittleFS.exists(filePath)) { request->send(LittleFS, filePath, "application/octet-stream", true); }
      else { request->send(404, "text/plain", "File not found"); }
    } else { request->send(400, "text/plain", "File not specified"); }
  });

  server.on("/edit", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasArg("file")) {
      String filePath = request->arg("file");
      if (LittleFS.exists(filePath)) {
        File file = LittleFS.open(filePath, "r");
        if (file) {
          String content = file.readString(); file.close();
          content.replace("&", "&amp;"); content.replace("<", "&lt;"); content.replace(">", "&gt;");
          String html = "<!DOCTYPE html><html lang='ru'><head><meta charset='UTF-8'><title>Редактор</title><style>:root { --bg-color: #121214; --card-bg: #1a1a1e; --text-color: #e1e1e6; --accent: #2196f3; --border-color: #29292e; --primary: #4caf50; } body { font-family: sans-serif; background-color: var(--bg-color); color: var(--text-color); padding: 20px; display: flex; flex-direction: column; align-items: center; } .container { max-width: 900px; width: 100%; } textarea { background: #09090a; border: 1px solid var(--border-color); border-radius: 8px; color: #00ff66; font-family: monospace; font-size: 14px; padding: 15px; width:100%; box-sizing:border-box; resize: vertical; outline: none; } .btn-box { display: flex; gap: 15px; margin-top:20px; } input[type='submit'] { background: var(--primary); color: #fff; border: none; border-radius: 6px; padding: 12px 24px; font-weight: 600; cursor: pointer; } .btn-cancel { display: inline-block; background: #2e2f34; color: var(--text-color); border: 1px solid var(--border-color); border-radius: 6px; padding: 12px 24px; text-decoration: none; }</style></head><body><div class='container'><h2>Редактирование: <span style='color:var(--accent)'>" + filePath + "</span></h2><form method='POST' action='/save'><input type='hidden' name='file' value='" + filePath + "'><textarea name='content' rows='22'>" + content + "</textarea><div class='btn-box'><input type='submit' value='Сохранить'><a href='/files' class='btn-cancel'>Отмена</a></div></form></div></body></html>";
          request->send(200, "text/html", html);
        } else { request->send(500, "text/plain", "Failed to open file"); }
      } else { request->send(404, "text/plain", "File not found"); }
    } else { request->send(400, "text/plain", "File not specified"); }
  });

  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request) {
    String filePath = ""; String content = "";
    if (request->hasArg("file")) filePath = request->arg("file");
    if (request->hasArg("content")) content = request->arg("content");
    if (filePath != "" && content != "") {
      File file = LittleFS.open(filePath, "w");
      if (file) { file.print(content); file.close(); request->send(200, "text/plain", "File saved successfully"); }
      else { request->send(500, "text/plain", "Failed to save file"); }
    } else { request->send(400, "text/plain", "Missing arguments"); }
  });

  server.begin();
  Serial.println("Server started");
}
