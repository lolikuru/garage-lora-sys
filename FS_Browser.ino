void listFiles(String path, String &html) {
  File root = LittleFS.open(path, "r");
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }

  if (!root.isDirectory()) {
    Serial.println("Specified path is not a directory");
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
      html += "<li><a href='/download?file=" + fullPath + "'>" + name + "</a> <span style='color:#666; font-size:13px;'>(" + String(file.size()) + " bytes)</span>";
      html += "<div class='actions'><a href='/download?file=" + fullPath + "' class='btn-download'>Скачать</a>";
      html += "<a href='/edit?file=" + fullPath + "'>Редактировать</a></div></li>";
    }
    file = root.openNextFile();
  }
  root.close();
}

void FS_Browser_init() {
  // Главная страница управления нагрузками
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

  // ЭНДПОИНТ 1: Отдача текущих состояний нагрузок в формате JSON
  server.on("/get_states", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (LittleFS.exists("/config.json")) {
      File confFile = LittleFS.open("/config.json", "r");
      String jsonContent = confFile.readString();
      confFile.close();
      request->send(200, "application/json", jsonContent);
    } else {
      // Если файла конфигурации еще нет, отдаем структуру по умолчанию
      request->send(200, "application/json", "{\"loads\":[0,0,0,0,0,0,0,0]}");
    }
  });

  // ЭНДПОИНТ 2: Изменение состояния одной конкретной нагрузки (0-7)
  server.on("/set_state", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasArg("id") && request->hasArg("state")) {
      int id = request->arg("id").toInt();
      int state = request->arg("state").toInt(); // 1 или 0

      JsonDocument doc;

      // Сначала читаем существующий файл конфигурации
      if (LittleFS.exists("/config.json")) {
        File confFile = LittleFS.open("/config.json", "r");
        deserializeJson(doc, confFile);
        confFile.close();
      }

      // Создаем массив loads, если его еще не существовало в config.json
      if (!doc.containsKey("loads")) {
        JsonArray array = doc.createNestedArray("loads");
        for (int i = 0; i < 8; i++) {
          array.add(0);
        }
      }

      // Обновляем значение нужной нагрузки по индексу
      if (id >= 0 && id < 8) {
        doc["loads"][id] = state;

        // ЗДЕСЬ ВЫ МОЖЕТЕ ДОБАВИТЬ ФИЗИЧЕСКОЕ ПЕРЕКЛЮЧЕНИЕ ПИНОВ РЕЛЕ, НАПРИМЕР:
        // int relayPins[8] = {5, 4, 0, 2, 14, 12, 13, 15}; // Пример пинов для ESP8266
        // digitalWrite(relayPins[id], state);

        Serial.printf("Load %d set to %d\n", id + 1, state);
      }

      // Перезаписываем обновленный JSON обратно в LittleFS
      File confFile = LittleFS.open("/config.json", "w");
      if (confFile) {
        serializeJson(doc, confFile);
        confFile.close();
        request->send(200, "text/plain", "OK");
      } else {
        request->send(500, "text/plain", "Failed to write config.json");
      }
    } else {
      request->send(400, "text/plain", "Missing arguments");
    }
  });

  // Просмотр списка файлов в LittleFS
  server.on("/files", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (LittleFS.exists("/files.html")) {
      File file = LittleFS.open("/files.html", "r");
      if (file) {
        String html = file.readString();
        file.close();
        listFiles("/", html);
        request->send(200, "text/html", html);
      } else {
        request->send(500, "text/plain", "Failed to open files.html");
      }
    } else {
      request->send(404, "text/plain", "files.html not found.");
    }
  });

  // Просмотр конфигурации Wi-Fi
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
        content.replace("{{timezone}}", String(doc["Client" + String(client_id)][1].as<int>()));
        content.replace("{{off_display_sec}}", String(doc["Client" + String(client_id)][3].as<int>()));
        content.replace("{{allways_on_disp}}", doc["Client" + String(client_id)][4].as<bool>() ? "checked" : "");
        content.replace("</body>", "</body><br><a href='/wifi' class='btn-back'>Back to Menu</a>");
        request->send(200, "text/html", content);
      } else {
        request->send(500, "text/plain", "Failed to open config.html");
      }
    } else {
      request->send(404, "text/plain", "Config file not found.");
    }
  });

  // Сохранение конфигурации Wi-Fi
  server.on("/save_wifi", HTTP_POST, [](AsyncWebServerRequest *request) {
    String ssdpName = request->arg("SSDPName");
    String ssidName = request->arg("ssidName");
    String ssidPassword = request->arg("ssidPassword");
    int timezone = request->arg("timezone").toInt();
    int off_display_sec = request->arg("off_display_sec").toInt();
    bool allways_on_disp = request->arg("allways_on_disp") == "on";

    JsonDocument doc;
    if (LittleFS.exists("/config.json")) {
      File confFile = LittleFS.open("/config.json", "r");
      deserializeJson(doc, confFile);
      confFile.close();
    }

    doc["SSDPName"] = ssdpName;
    doc["ssidName"] = ssidName;
    doc["ssidPassword"] = ssidPassword;

    // ВАЖНО: сохраняем старую логику работы с клиентами, чтобы не сломать её
    JsonArray clients = doc["Client"].as<JsonArray>();
    if(clients.isNull()) {
       clients = doc.createNestedArray("Client");
    }
    JsonArray clientData = clients.createNestedArray();
    clientData[0] = timezone;
    clientData[1] = off_display_sec;
    clientData[2] = allways_on_disp;

    File confFile = LittleFS.open("/config.json", "w");
    if (confFile) {
      serializeJson(doc, confFile);
      confFile.close();
      request->send(200, "text/plain", "Configuration saved");
    } else {
      request->send(500, "text/plain", "Failed to save configuration");
    }
  });

  // Страница load.html (при необходимости)
  server.on("/load", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (LittleFS.exists("/load.html")) {
      File file = LittleFS.open("/load.html", "r");
      if (file) {
        String content = file.readString();
        file.close();
        request->send(200, "text/html", content);
      } else {
        request->send(500, "text/plain", "Failed to open load.html");
      }
    } else {
      request->send(404, "text/plain", "load.html not found.");
    }
  });

  // Скачивание файлов
  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasArg("file")) {
      String filePath = request->arg("file");
      if (LittleFS.exists(filePath)) {
        request->send(LittleFS, filePath, "application/octet-stream", true);
      } else {
        request->send(404, "text/plain", "File not found");
      }
    } else {
      request->send(400, "text/plain", "File not specified");
    }
  });

  // Редактирование файлов через форму
    server.on("/edit", HTTP_GET, [](AsyncWebServerRequest *request) {
      if (request->hasArg("file")) {
        String filePath = request->arg("file");
        if (LittleFS.exists(filePath)) {
          File file = LittleFS.open(filePath, "r");
          if (file) {
            String content = file.readString();
            file.close();

            // Экранные сущности экранируются для корректного отображения внутри textarea
            content.replace("&", "&amp;");
            content.replace("<", "&lt;");
            content.replace(">", "&gt;");

            String html = "<!DOCTYPE html><html lang='ru'><head><meta charset='UTF-8'>";
            html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
            html += "<title>Редактор файлов</title><style>";
            html += ":root { --bg-color: #121214; --card-bg: #1a1a1e; --text-color: #e1e1e6; --accent: #2196f3; --border-color: #29292e; --primary: #4caf50; }";
            html += "body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; background-color: var(--bg-color); color: var(--text-color); margin: 0; padding: 20px; display: flex; flex-direction: column; align-items: center; }";
            html += ".container { max-width: 900px; width: 100%; }";
            html += "header { border-bottom: 1px solid var(--border-color); padding-bottom: 15px; margin-bottom: 25px; }";
            html += "h1 { margin: 0; font-size: 22px; font-weight: 600; color: #a8a8b3; }";
            html += "span { color: var(--accent); }";
            html += "form { display: flex; flex-direction: column; gap: 20px; }";
            html += "textarea { background: #09090a; border: 1px solid var(--border-color); border-radius: 8px; color: #00ff66; font-family: 'Fira Code', Consolas, monospace; font-size: 14px; padding: 15px; resize: vertical; outline: none; line-height: 1.5; }";
            html += "textarea:focus { border-color: var(--accent); }";
            html += ".btn-box { display: flex; gap: 15px; }";
            html += "input[type='submit'] { background-color: var(--primary); color: #fff; border: none; border-radius: 6px; padding: 12px 24px; font-size: 15px; font-weight: 600; cursor: pointer; transition: background 0.2s; }";
            html += "input[type='submit']:hover { background-color: #43a047; }";
            html += ".btn-cancel { display: inline-block; background-color: #2e2f34; color: var(--text-color); border: 1px solid var(--border-color); border-radius: 6px; padding: 12px 24px; font-size: 15px; text-decoration: none; text-align: center; transition: background 0.2s; }";
            html += ".btn-cancel:hover { background-color: #3e3e42; }";
            html += "</style></head><body><div class='container'>";
            html += "<header><h1>Редактирование файла: <span>" + filePath + "</span></h1></header>";
            html += "<form method='POST' action='/save'>";
            html += "<input type='hidden' name='file' value='" + filePath + "'>";
            html += "<textarea name='content' rows='22' spellcheck='false'>" + content + "</textarea>";
            html += "<div class='btn-box'><input type='submit' value='Сохранить изменения'>";
            html += "<a href='/files' class='btn-cancel'>Отмена</a></div>";
            html += "</form></div></body></html>";

            request->send(200, "text/html", html);
          } else {
            request->send(500, "text/plain", "Failed to open file");
          }
        } else {
          request->send(404, "text/plain", "File not found");
        }
      } else {
        request->send(400, "text/plain", "File not specified");
      }
    });

  // Сохранение изменений текстового файла
  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request) {
    String filePath = "";
    String content = "";
    if (request->hasArg("file")) filePath = request->arg("file");
    if (request->hasArg("content")) content = request->arg("content");

    if (filePath != "" && content != "") {
      File file = LittleFS.open(filePath, "w");
      if (file) {
        file.print(content);
        file.close();
        request->send(200, "text/plain", "File saved successfully");
      } else {
        request->send(500, "text/plain", "Failed to save file");
      }
    }else {
        request->send(400, "text/plain", "Missing 'file' or 'content' argument");
    }
  });
    server.begin();
    Serial.println("Server started");
}